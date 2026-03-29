#include <sstream>
#include "response_serializer.h"

std::string ResponseSerializer::serializeResponse(const HttpResponse &response) {
    std::ostringstream buffer;
    buffer << "HTTP/1.1 " << static_cast<int>(response.status)
        << " " << statusMethodToPhrase(response.status) << "\r\n"
        << serializeHeaders(response) << "\r\n" << response.body;
    return buffer.str();
}

std::string ResponseSerializer::serializeHeaders(const HttpResponse &response) {
    std::ostringstream buffer;
    for (const auto& pair : response.headers) {
        buffer << pair.first << ": " << pair.second << "\r\n"; 
    }

    return buffer.str();
}

std::string_view ResponseSerializer::statusMethodToPhrase(HttpStatus status) {
    switch (status) {
        case HttpStatus::OK: return "OK";
        case HttpStatus::CREATED: return "CREATED";
        case HttpStatus::ACCEPTED: return "ACCEPTED";
        case HttpStatus::NO_CONTENT: return "NO CONTENT";
        case HttpStatus::MULTIPLE_CHOICES: return "MULTIPLE CHOICES";
        case HttpStatus::MOVED_PERMANENTLY: return "MOVED PERMANENTLY";
        case HttpStatus::MOVED_TEMPORARILY: return "MOVED TEMPORARILY";
        case HttpStatus::NOT_MODIFIED: return "NOT MODIFIED";
        case HttpStatus::BAD_REQUEST: return "BAD REQUEST";
        case HttpStatus::UNAUTHORIZED: return "UNAUTHORIZED";
        case HttpStatus::FORBIDDEN: return "FORBIDDEN";
        case HttpStatus::NOT_FOUND: return "NOT FOUND";
        case HttpStatus::INTERNAL_SERVER_ERROR: return "INTERNAL SERVER ERROR";
        case HttpStatus::NOT_IMPLEMENTED: return "NOT IMPLEMENTED";
        case HttpStatus::BAD_GATEWAY: return "BAD GATEWAY";
        case HttpStatus::SERVICE_UNAVAILABLE: return "SERVICE UNAVAILABLE";
    }
}
