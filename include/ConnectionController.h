
#ifndef CONNECTION_CONTROLLER_H
#define CONNECTION_CONTROLLER_H

#include <string>
#include <unordered_map>
#include "nlohmann/json.hpp"
#include <curl/curl.h>
using json = nlohmann::json;

class ConnectionController {
private:
    std::string baseUrl;

public:
    ConnectionController(std::string& baseUrl);

    json sendRequest(
        const std::string& endpoint,
        const std::unordered_map<std::string, std::string>& params,
        const std::string& method,
        const std::string& token = "");

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif
