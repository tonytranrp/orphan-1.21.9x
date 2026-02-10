#pragma once
//
// Created by vastrakai on 6/29/2024.
//

#include <string>
#include <vector>

class Resource;

class FileUtils {
public:
    static std::string getRoamingStatePath();
    static std::string getOrphanDir();
    static bool fileExists(const std::string& path);
    static void createDirectory(const std::string& path);
    static void validateDirectories();
    static bool deleteFile(const std::string& path);
    static void writeResourceToFile(Resource* resource, const std::string& path);
    static void writeResourceToFile(const std::string& path, const unsigned char* data, size_t size);
    static std::vector<std::string> listFiles(const std::string& path);
    static void createFile(const std::string& path);
    // getFileSize
    static size_t getFileSize(const std::string& path);
    //readFile
    static std::vector<unsigned char> readFile(const std::string& path);

    // Download a file from a URL and save it to the given path
    static bool downloadFile(const std::string& url, const std::string& savePath);

    // Save an embedded resource (from Resources.hpp) to a file
    static bool saveResourceToFile(const std::string& resourceName, const std::string& savePath);
};
