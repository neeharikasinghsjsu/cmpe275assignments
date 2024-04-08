#include <iostream>
#include "record.hpp"
#include <vector>
#include <mpi.h>
#include "loader.hpp"
#include <chrono>
#include "filemanager.hpp"
#include <map>
#include <fstream>
#include <omp.h>
#include <random>
#include <sstream>
#include "sharedmem.hpp"
#include <sys/ipc.h>
#include <sys/shm.h>

#define ALL_DONE "ALL_DONE"

typedef struct
{
    int worldSize;
    std::string inputDirectory;
    std::string finalOutputDirectory;
    std::string tempOutputDirectory;
} WorldCommonInfo;

int getCurrentSeconds()
{
    // Get the current time as a time_point
    auto now = std::chrono::system_clock::now();

    // Convert the time_point to a duration representing the number of seconds
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());

    // Convert the duration to an integer representing the number of seconds
    return static_cast<long>(duration.count());
}

void ensureDirectory(std::string dir)
{
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }
}

std::string createAndGetOutputDirectory(std::string parentDir)
{
    ensureDirectory(parentDir);
    std::string outputDir = parentDir + "/" + std::to_string(getCurrentSeconds());
    ensureDirectory(outputDir);
    return outputDir;
}

int sitenameToWorldRank(const std::string &sitename, const WorldCommonInfo &worldCommonInfo)
{
    int sum = 0;
    for (char c : sitename)
    {
        sum += static_cast<int>(c); // Convert char to its ASCII value and add it to sum
    }
    return sum % worldCommonInfo.worldSize;
}

std::string getTempRankDir(int worldRank, const WorldCommonInfo &worldCommonInfo)
{
    return worldCommonInfo.tempOutputDirectory + "/" + std::to_string(worldRank);
}

std::string getTempFilenameForSite(int processingWorldRank, const std::string &sitename, const WorldCommonInfo &worldCommonInfo)
{
    int sitenameWorldRank = sitenameToWorldRank(sitename, worldCommonInfo);
    std::string rankDir = getTempRankDir(sitenameWorldRank, worldCommonInfo);
    std::string sitenameDir = rankDir + "/" + FileManager::sanitizeFilename(sitename);
    ensureDirectory(sitenameDir);
    return sitenameDir + "/" + std::to_string(processingWorldRank) + ".csv";
}

bool flushToFile(const std::string &filename, const std::vector<std::string> &siteRecords)
{
    std::ofstream outputFile(filename, std::ios::app);
    if (!outputFile.is_open())
    {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false; // Skip, todo: add some error metrics
    }
    for (const std::string &record : siteRecords)
    {
        // Write record to the file
        outputFile << record << std::endl;
    }
    outputFile.close();
    return true;
}

void flushAll(int worldRank, const WorldCommonInfo &worldCommonInfo, const std::map<std::string, std::vector<std::string>> &recordsBySite)
{
    std::vector<std::string> sitenames;
    for (const auto &pair : recordsBySite)
    {
        sitenames.push_back(pair.first);
    }
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < sitenames.size(); ++i)
    {
        const auto &sitename = sitenames[i];
        const auto &records = recordsBySite.at(sitename);
        if (records.size() > 0)
        {
            std::string filename = getTempFilenameForSite(worldRank, sitename, worldCommonInfo);
            flushToFile(filename, records);
        }
    }
}

// to stagger file writes to avoid bottlenecking on disk
bool randomlyDecideToFlushToFile(const WorldCommonInfo &worldCommonInfo)
{
    // Random number generator
    static std::random_device rd;                                               // Obtain a random number from hardware
    static std::mt19937 gen(rd());                                              // Seed the generator
    static std::uniform_int_distribution<> distr(0, worldCommonInfo.worldSize); // Define the range

    // Generate a random integer
    int randomInteger = distr(gen);
    return randomInteger == 1;
}

void processFile(Loader &loader, const std::string &filepath, std::map<std::string, std::vector<std::string>> &recordsBySite)
{
    try
    {
        std::map<std::string, std::vector<Record>> typedRecordsBySite;

        // Load records from the current file
        std::vector<Record> records = loader.loadRecordsFromCSV(filepath);
        // std::cout << "Rank " << worldRank << " loaded " << records.size() << " records from " << filePath << std::endl;

        for (const Record &record : records)
        {
            typedRecordsBySite[record.getSiteName()].push_back(record);
        }

        for (const auto &pair : typedRecordsBySite)
        {
            auto sitename = pair.first;
            auto records = pair.second;
            if (records.size() > 0)
            {
                // find MAX AQI
                // Find the maximum element using a lambda comparator
                auto record = std::max_element(records.begin(), records.end(), [](Record a, Record b)
                                               { return a.getAQI() < b.getAQI(); });

                std::stringstream ss;
                ss << record->getRecordTime().time_since_epoch().count() << ","
                   << record->getPollutant() << "," << record->getConcentration() << ","
                   << record->getPollutantUnit() << "," << record->getRawConcentration() << ","
                   << record->getAQI() << "," << record->getCategory();

                recordsBySite[sitename].push_back(ss.str());
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << " failed to load records from " << filepath << ": " << e.what() << std::endl;
    }
}

void fileProcessingWork(int worldRank, const WorldCommonInfo &worldCommonInfo, std::vector<std::string> filePaths)
{
    Loader loader;

    // Group records by siteName using a map
    std::map<std::string, std::vector<std::string>> recordsBySite;
    for (int i = worldRank; i < filePaths.size(); i += worldCommonInfo.worldSize)
    {
        std::string filepath = filePaths[i];
        processFile(loader, filepath, recordsBySite);

        // to stagger disk write
        if (randomlyDecideToFlushToFile(worldCommonInfo))
        {
            flushAll(worldRank, worldCommonInfo, recordsBySite);
            recordsBySite.clear();
        }
    }
    flushAll(worldRank, worldCommonInfo, recordsBySite);
}

void consolidateSiteFiles(const std::string &sitename, std::string readDir, const WorldCommonInfo &worldCommonInfo)
{
    std::string outputFilename = worldCommonInfo.finalOutputDirectory + "/" + sitename + ".csv";
    // Open output file for writing
    std::ofstream outputFile(outputFilename, std::ios::out | std::ios::binary);
    if (!outputFile.is_open())
    {
        std::cerr << "Failed to open output file: " << outputFilename << std::endl;
        return;
    }
    for (const auto &entry : std::filesystem::recursive_directory_iterator(readDir))
    {
        if (entry.is_regular_file())
        {
            // Open input file for reading
            std::ifstream inputFile(entry, std::ios::in | std::ios::binary);
            if (!inputFile.is_open())
            {
                std::cerr << "Failed to open input file: " << entry << std::endl;
                continue;
            }

            // Read input file contents and write to output file
            outputFile << inputFile.rdbuf();

            // Close input file
            inputFile.close();
        }
    }
    outputFile.close();
}

void sitenameFileConsolidationWork(int worldRank, const WorldCommonInfo &worldCommonInfo)
{

    std::vector<std::string> filePaths;
    std::string rankDir = getTempRankDir(worldRank, worldCommonInfo);
    std::vector<std::string> sitenames;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(rankDir))
    {
        if (entry.is_directory())
        {
            auto sitename = entry.path().filename();
            sitenames.push_back(sitename);
        }
    }
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < sitenames.size(); ++i)
    {
        std::string path = rankDir + "/" + sitenames[i];
        consolidateSiteFiles(sitenames[i], path, worldCommonInfo);
    }
}

void preCreateDirs(int worldRank, const WorldCommonInfo &worldCommonInfo)
{
    std::string rankDir = getTempRankDir(worldRank, worldCommonInfo);
    ensureDirectory(rankDir);
}

std::vector<std::string> getAllFilePaths(const WorldCommonInfo &worldCommonInfo)
{
    std::vector<std::string> filePaths;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(worldCommonInfo.inputDirectory))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".csv")
        {
            filePaths.push_back(entry.path().string());
        }
    }
    return filePaths;
}

int millisFromNow(std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
    auto end = std::chrono::high_resolution_clock::now();
    // Calculate and print the elapsed time for this process
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return static_cast<int>(duration.count());
}

int main(int argc, char **argv)
{
    auto start = std::chrono::high_resolution_clock::now();
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <input_directory> <output_directory> <num_threads>" << std::endl;
        return 1;
    }

    std::string inputDirectory = argv[1];
    std::string outputDirectory = argv[2];
    int numThreads = std::stoi(argv[3]);

    // Set the number of OpenMP threads
    omp_set_num_threads(numThreads);

    MPI_Init(&argc, &argv);

    int worldSize;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    int worldRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

    char finalOutputDirectoryBuf[4096];
    char tempOutputDirectoryBuf[4096];

    if (worldRank == 0)
    {
        strcpy(finalOutputDirectoryBuf, createAndGetOutputDirectory(outputDirectory).c_str());
        strcpy(tempOutputDirectoryBuf, createAndGetOutputDirectory("/tmp/airnow").c_str());
    }

    // Broadcast directory paths from root process to all other processes
    MPI_Bcast(&finalOutputDirectoryBuf, 4096, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&tempOutputDirectoryBuf, 4096, MPI_CHAR, 0, MPI_COMM_WORLD);

    std::string finalOutputDirectory(finalOutputDirectoryBuf);
    std::string tempOutputDirectory(tempOutputDirectoryBuf);

    WorldCommonInfo worldCommonInfo = {
        worldSize,
        inputDirectory,
        finalOutputDirectory,
        tempOutputDirectory};

    preCreateDirs(worldRank, worldCommonInfo);

    // Wait for all processes to intialize
    MPI_Barrier(MPI_COMM_WORLD);

    int shmsize = 0;
    if (worldRank == 0)
    {
        auto filepaths = getAllFilePaths(worldCommonInfo);
        shmsize = createSharedMemoryForFilePaths(filepaths);
    }

    // Broadcast shmsize from root process to all other processes
    MPI_Bcast(&shmsize, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int shmid = shmget(APP_SHM_KEY, shmsize, 0666);
    if (shmid < 0)
    {
        perror("shmget failed to get application's shared memory segment.");
        exit(1);
    }

    // Attach the shared memory segment
    char *shmaddr = (char *)shmat(shmid, nullptr, 0);
    if (shmaddr == (char *)-1)
    {
        perror("shmat");
        MPI_Finalize();
        return 1;
    }
    auto filepaths = getSharedMemoryFilePaths(shmaddr);
    fileProcessingWork(worldRank, worldCommonInfo, filepaths);

    // Detach the shared memory segment
    if (shmdt(shmaddr) == -1)
    {
        perror("shmdt");
    }

    if (worldRank == 0)
    {
        // Destroy the shared memory segment
        if (shmctl(shmid, IPC_RMID, nullptr) == -1)
        {
            perror("shmctl");
        }
    }

    // Wait for all processes to finish file processing
    MPI_Barrier(MPI_COMM_WORLD);

    sitenameFileConsolidationWork(worldRank, worldCommonInfo);

    std::cout << "Rank " << worldRank << " Time taken: " << millisFromNow(start) << " milliseconds" << std::endl;
    MPI_Finalize();
    return 0;
}
