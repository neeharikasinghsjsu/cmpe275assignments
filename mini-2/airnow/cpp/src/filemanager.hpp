#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <stdlib.h>
#include <vector>

class FileManager
{
public:
    static std::vector<std::string> readCSVFilePaths(const std::string &directoryPath);
    // Function to sanitize a filename by replacing disallowed characters with underscores
    static std::string sanitizeFilename(const std::string &filename);
};

#endif