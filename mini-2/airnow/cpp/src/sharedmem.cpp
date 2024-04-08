#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <vector>
#include <string>
#include "sharedmem.hpp"
#include <sstream>

// Function to serialize the vector of strings into a character buffer
std::string serializeVector(const std::vector<std::string> &vec)
{
    std::string serializedData;
    for (const std::string &str : vec)
    {
        serializedData += str + '\n'; // Separate strings with newline character
    }
    return serializedData;
}

// Function to deserialize the character buffer into a vector of strings
std::vector<std::string> deserializeVector(const std::string &serializedData)
{
    std::vector<std::string> vec;
    std::istringstream iss(serializedData);
    std::string line;
    while (std::getline(iss, line))
    {
        vec.push_back(line);
    }
    return vec;
}

int createSharedMemoryForFilePaths(const std::vector<std::string> &filepaths)
{
    std::string serialized = serializeVector(filepaths);
    int shmSize = serialized.size() + 1;

    // Create the segment. Note IPC_CREAT does not exist
    int shmid = shmget(APP_SHM_KEY, shmSize, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid < 0)
    {
        perror("shmget");
        exit(1);
    }

    // Attach the shared memory segment to the process's address space
    char *shmaddr = (char *)shmat(shmid, nullptr, 0);
    if (shmaddr == (char *)-1)
    {
        perror("shmat");
        return 1;
    }

    strcpy(shmaddr, serialized.c_str());

    // Detach the shared memory segment
    if (shmdt(shmaddr) == -1)
    {
        perror("shmdt");
        return 1;
    }

    return shmSize;
}

std::vector<std::string> getSharedMemoryFilePaths(char *shmaddr)
{

    // Access the shared string
    std::string receivedString(shmaddr);

    return deserializeVector(receivedString);
}