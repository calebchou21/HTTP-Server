#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <unordered_map>

enum class HttpStatus {
    // Successful 2xx
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NO_CONTENT = 204,
    
    // Redirection 3xx
    MULTIPLE_CHOICES = 300,
    MOVED_PERMANENTLY = 301,
    MOVED_TEMPORARILY = 302,
    NOT_MODIFIED = 304,
    
    // Client errors 4xx
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,

    // Server errors 5xx
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    BAD_GATEWAY = 502,
    SERVICE_UNAVAILABLE = 503 
};

struct HttpResponse {
    int versionMajor;
    int versionMinor;
    HttpStatus status;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    static HttpResponse create(HttpStatus status, std::string body = "");
};
#endif
