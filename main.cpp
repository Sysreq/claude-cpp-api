#include "ClaudeAPI.h"

int main() {
    ClaudeAPI claude("YOUR_API_KEY");
    std::string response = claude.sendMessage("Hello, Claude!");
    std::cout << response << std::endl;
    return 0;
}