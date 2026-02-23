//
// Created by rekitrelt 2025-03-06
//

#pragma once

#include "Authorization.hpp"
#include <build/build_info.h>
#include <curl/curl.h>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <Features/Events/BaseTickEvent.hpp>

bool hasCheckedPrivateUser = false;
bool isaPrivateUser = false;
std::string computerName = "PCName";

std::string getComputerNameX() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return std::string(hostname);
    }
    else {
        return "Unknown";
    }
}

std::string getUserNameX() {
    char username[256];
    DWORD size = sizeof(username);

    if (GetUserNameA(username, &size)) {
        return std::string(username);
    }
    else {
        return "Unknown User";
    }
}

// Callback function for CURL to write data into a string
size_t WriteCallbackX(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch the CSV data from the Google Sheet
std::string fetchGoogleSheetCSVX(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Allow libcurl to follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Set up the function to handle the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackX);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            spdlog::error("Failed to fetch Google Sheet: {}", curl_easy_strerror(res));
            return "";
        }
    }
    return readBuffer;
}

std::vector<std::string> parseCSVX(const std::string& csvData) {
    std::vector<std::string> whiteList;
    std::istringstream stream(csvData);
    std::string line;

    while (std::getline(stream, line)) {
        // Skip empty lines
        if (line.empty()) continue;

        // Split the line into columns (names in this case)
        std::istringstream lineStream(line);
        std::string name;

        while (std::getline(lineStream, name, ',')) {
            // Trim leading/trailing whitespace
            name.erase(0, name.find_first_not_of(" \t\n\r\f\v"));
            name.erase(name.find_last_not_of(" \t\n\r\f\v") + 1);

            // Add the name to the list if it's not empty
            if (!name.empty()) {
                whiteList.push_back(name);
            }
        }
    }

    return whiteList;
}
bool Auth::isPrivateUser() {
    static const std::vector<std::string> devUsers = {
        "desktop-cfggece/rekitrelt", // rekitrelt
        "desktop-cg2fr4g/pc", // Proflly
        "desktop-l69i8bu/provi" // Player 5
        "pc-beaub/chand" // EDest
    };
    std::string csvUrl = "https://docs.google.com/spreadsheets/d/1_9JLrTk8nouFl9NEeKkxyByGIZHDDTQhpdnanQL238E/export?format=csv";
    std::string csvData = fetchGoogleSheetCSVX(csvUrl);
    if (csvData.empty()) {
        spdlog::error("Failed to fetch or parse the Google Sheet.");
    }
    std::vector<std::string> privUsers = parseCSVX(csvData);
    if (!hasCheckedPrivateUser) {
        computerName = getComputerNameX();
        std::string pcusername = getUserNameX();
        std::string whois = computerName + "/" + pcusername;
        std::string whoisLower = StringUtils::toLower(whois);
        isaPrivateUser = std::find(devUsers.begin(), devUsers.end(), whoisLower) != devUsers.end();
        if (!isaPrivateUser)
            isaPrivateUser = std::find(privUsers.begin(), privUsers.end(), whoisLower) != privUsers.end();
        return isaPrivateUser;
    }
    return isaPrivateUser;
}