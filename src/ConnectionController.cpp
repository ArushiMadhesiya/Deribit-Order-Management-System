#include "ConnectionController.h"
#include <iostream>


using json = nlohmann::json; // Alias for convenience

ConnectionController::ConnectionController(std::string& baseUrl) : baseUrl(baseUrl) {}

size_t ConnectionController::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

json ConnectionController::sendRequest(
    const std::string& endpoint,
    const std::unordered_map<std::string, std::string>& params,
    const std::string& method,
    const std::string& token) {

    CURL* curl;
    CURLcode res;

    std::string url = baseUrl + endpoint;

    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "curl_easy_init() failed!" << std::endl;
        return json(); // Return empty json object on error
    }

    json dataJson; 

    if (method == "POST") {
        for (const auto& param : params) {
            dataJson[param.first] = param.second; // Add parameters to json object
        }
    } else if (method == "GET") {
        if (!params.empty()) {
            url += "?";
            for (const auto& param : params) {
                url += param.first + "=" + param.second + "&";
            }
            url.pop_back();
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        }
    } else {
        // Handle other methods (e.g., PUT, DELETE) if needed
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
        for (const auto& param : params) {
          dataJson[param.first] = param.second;
        }
    }


    std::string dataString;
    if (!dataJson.empty() && method != "GET") {
      dataString = dataJson.dump();
      curl_easy_setopt(curl, CURLOPT_POST, 1L);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataString.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, dataString.length());
    }


    std::string response_string;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    struct curl_slist* headers = nullptr;
    if (!token.empty()) {
        std::string auth_header = "Authorization: Bearer " + token;
        headers = curl_slist_append(headers, auth_header.c_str());
    }

    // headers
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return json();
    }

    json doc;
    try {
        doc = json::parse(response_string); // Parse using nlohmann::json
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse error: " << e.what() << std::endl;
        std::cerr << "Response String: " << response_string << std::endl;
    }


    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return doc;
}