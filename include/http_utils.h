#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <string>
#include <ctime>

namespace HttpUtils {
    std::string formatHttpDate(std::time_t time); 
    std::time_t parseHttpDate(const std::string &dateStr);
}

#endif
