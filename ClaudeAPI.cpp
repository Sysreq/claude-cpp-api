#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

class ClaudeAPI {
private:
    std::string apiKey;
    std::string apiUrl;

public:
    ClaudeAPI(const std::string& key) : apiKey(key), apiUrl("https://api.anthropic.com") {}

    std::string sendMessage(const std::string& message) {
        // Prepare the request payload
        nlohmann::json payload = {
            {"prompt", message},
            {"model", "claude-v1"},
            {"max_tokens_to_sample", 100}
        };

        // Set up the cURL request
        CURL* curl = curl_easy_init();
        if (curl) {
            // Set the request URL
            curl_easy_setopt(curl, CURLOPT_URL, (apiUrl + "/v1/complete").c_str());

            // Set the request method to POST
            curl_easy_setopt(curl, CURLOPT_POST, 1L);

            // Set the request headers
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            // Set the request body
            std::string requestBody = payload.dump();
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());

            // Set the response callback function
            std::string response;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            // Send the request
            CURLcode res = curl_easy_perform(curl);

            // Clean up
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);

            if (res == CURLE_OK) {
                // Parse the JSON response
                nlohmann::json jsonResponse = nlohmann::json::parse(response);
                return jsonResponse["completion"].get<std::string>();
            }
        }

        return "";
    }

private:
    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append(ptr, size * nmemb);
        return size * nmemb;
    }
};