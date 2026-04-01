#include <chrono>
#include "response.h"
#include "http_utils.h"

HttpResponse HttpResponse::create(HttpStatus status, std::string body) {
    HttpResponse response;
    response.versionMajor = 1;
    response.versionMinor = 0;
    response.status = status;
    if (!body.empty()) {
        response.body = body;
        response.headers["Content-Length"] = std::to_string(response.body.size());
    } else {
        response.headers["Content-Length"] = "0";
    }
    response.headers["Content-Type"] = "text/plain"; // Default type
    response.headers["Server"] = "my-http-server/1.0";

    const auto now = std::chrono::system_clock::now();
    const std::time_t convertedNow = std::chrono::system_clock::to_time_t(now);
    response.headers["Date"] = HttpUtils::formatHttpDate(convertedNow);
    return response;
}
