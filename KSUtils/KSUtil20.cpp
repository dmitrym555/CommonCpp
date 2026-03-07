//
// CommonCPP utils
// Author: Dmitry Melnik
//

#include "KSUtil20.h"

#include <filesystem>

#include <chrono>
#include <string>

#include <iostream>
#include <fstream>

double KSStr2Double(const std::string& str ) {
    double res;
    size_t comapos = str.find( "," );
    if ( comapos != std::string::npos ) {
        std::string strcopy = str;
        strcopy[comapos] = '.';
        std::from_chars( strcopy.data(), strcopy.data() + strcopy.length(), res );
    }
    else {
        std::from_chars( str.data(), str.data() + str.length(), res );
    }
    return res;
}


//
// AI generated
//
int removeOldFiles(const std::string& directoryPath, int daysOld) {
    namespace fs = std::filesystem;
    // Calculate the cutoff time point
    const auto cutoffTime = fs::file_time_type::clock::now() - std::chrono::days(daysOld);

    int removedCount = 0;
    try {
        // Iterate over all entries in the directory
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            // Check if the entry is a regular file and its write time is older than the cutoff
            if (entry.is_regular_file()) {
                auto fileWriteTime = entry.last_write_time();

                if (fileWriteTime < cutoffTime) {
                    fs::remove(entry.path()); // Delete the file
                    ++removedCount;
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        removedCount = -1;
    }
    return removedCount;
}

std::string KSLoadFromFileToString(const std::string& filename) {
    namespace fs = std::filesystem;
    std::ifstream file(filename, std::ios::binary); // Open in binary mode for exact size
    if (!file) {
        return "";
    }

    // Get the file size (C++17 feature)
    auto size = fs::file_size(filename);
    std::string res(size, '\0'); // Resize the string

    // Read the data directly into the string's buffer
    file.read(&res[0], size);

    return res;
}

