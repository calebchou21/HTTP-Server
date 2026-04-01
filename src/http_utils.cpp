#include <sstream>
#include <iomanip>

#include "http_utils.h"

std::string HttpUtils::formatHttpDate(std::time_t time) {
    std::tm gmtm = *std::gmtime(&time);
    std::stringstream ss;
    ss << std::put_time(&gmtm, "%a, %d %b %Y %H:%M:%S GMT");
    return ss.str();
}

time_t HttpUtils::parseHttpDate(const std::string& dateStr) {
    std::tm tm = {};
    std::istringstream ss(dateStr);
    // Parse format: "Sun, 06 Nov 1994 08:49:37 GMT"
    ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    return timegm(&tm);
}
