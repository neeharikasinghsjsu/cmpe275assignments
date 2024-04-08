#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include "filemanager.hpp"

std::vector<std::string> FileManager::readCSVFilePaths(const std::string &directoryPath)
{
    std::vector<std::string> filePaths;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(directoryPath))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".csv")
        {
            filePaths.push_back(entry.path().string());
        }
    }
    return filePaths;
}

// Function to sanitize a filename by replacing disallowed characters with underscores
std::string FileManager::sanitizeFilename(const std::string &filename)
{
    std::string sanitized_filename;
    for (char c : filename)
    {
        // Check if the character is disallowed
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
        {
            // Replace disallowed characters with underscores
            sanitized_filename += '_';
        }
        else
        {
            // Keep allowed characters unchanged
            sanitized_filename += c;
        }
    }
    return sanitized_filename;
}