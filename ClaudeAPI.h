#ifndef CLAUDE_API_H
#define CLAUDE_API_H

#include <string>

class ClaudeAPI {
private:
    std::string apiKey;
    std::string apiUrl;

public:
    ClaudeAPI(const std::string& key);
    std::string processDocument(const std::string& pdfPath);
    std::string sendMessage(const std::string& message);

private:
    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data);
};

#endif