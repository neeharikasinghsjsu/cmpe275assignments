#include "loader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>

Record Loader::parseCSVLineToRecord(const std::string &line)
{
    std::vector<std::string> fields = parseCSVLine(line);
    if (fields.size() < 13)
    {
        throw std::runtime_error("Invalid record format");
    }

    // Convert fields to appropriate types
    double latitude = std::stod(fields[0]);
    double longitude = std::stod(fields[1]);

    // Parse datetime
    std::tm tm = {};
    std::istringstream ss(fields[2]);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M");
    auto record_time = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    std::string pollutant = fields[3];
    double concentration = std::stod(fields[4]);
    std::string unit = fields[5];
    double rawConcentration = std::stod(fields[6]);
    int aqi = std::stoi(fields[7]);
    int category = std::stoi(fields[8]);
    std::string siteName = fields[9];
    std::string agency = fields[10];
    std::string aqsId = fields[11];
    std::string ccAqsId = fields[12];

    return Record(latitude, longitude, record_time, pollutant, concentration, unit,
                  rawConcentration, aqi, category, siteName, agency, aqsId, ccAqsId);
}

std::vector<Record> Loader::loadRecordsFromCSV(const std::string &filePath)
{
    std::vector<Record> records;
    std::ifstream file(filePath);

    if (!file.is_open())
    {
        std::string errorMessage = "Could not open file: " + filePath;
        std::cerr << errorMessage << std::endl; // Print error to standard error stream
        throw std::runtime_error(errorMessage); // Throw exception with error message
    }

    std::string line;
    // Skip header if present
    // std::getline(file, line);

    while (std::getline(file, line))
    {
        try
        {
            Record record = parseCSVLineToRecord(line);
            records.push_back(record);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to parse line: " << line << "; Error: " << e.what() << std::endl;
            // Handle or ignore error
        }
    }
    file.close();
    return records;
}

std::vector<std::string> Loader::parseCSVLine(const std::string &line)
{
    std::vector<std::string> fields;
    std::string field;
    bool insideQuotes = false;

    for (size_t i = 0; i < line.length(); ++i)

    {
        char currentChar = line[i];

        // Start or end of a quoted field
        if (currentChar == '"')
        {
            // If it's a double quote inside a quoted field (lookahead to check if next character is also a quote)
            if (insideQuotes && i + 1 < line.length() && line[i + 1] == '"')
            {
                field += currentChar; // Add one quote to field and skip the next
                ++i;                  // Skip next quote
            }
            else
            {
                insideQuotes = !insideQuotes; // Toggle the state
            }
        }
        else if (currentChar == ',' && !insideQuotes)
        {
            // End of a field
            fields.push_back(field);
            field.clear();
        }
        else
        {
            // Part of a field
            field += currentChar;
        }
    }

    // Add the last field (if any) as it won't be followed by a comma
    if (!field.empty() || line.back() == ',')
    {
        fields.push_back(field);
    }
    return fields;
}
