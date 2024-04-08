#ifndef LOADER_HPP
#define LOADER_HPP

#include "record.hpp"
#include <vector>
#include <string>

class Loader
{
public:
    std::vector<Record> loadRecordsFromCSV(const std::string &filePath);

private:
    Record parseCSVLineToRecord(const std::string &line);
    std::vector<std::string> parseCSVLine(const std::string &line);
};

#endif // LOADER_HPP
