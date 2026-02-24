#include <iostream>

namespace logger
{
    template<typename... Args>
    void logMessage(Args&&... args) {
        std::cout << "[LOG] ";
        (std::cout << ... << args) << "\n";
    }

    template<typename... Args>
    void logError(Args&&... args) {
        std::cerr << "\x1b[31m[ERROR] ";
        (std::cerr << ... << args) << "\x1b[0m\n";
    }
}
