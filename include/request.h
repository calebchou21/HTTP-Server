#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <unordered_map>

enum class HttpRequestMethod {
    GET,
    POST,
    UNKNOWN,
};

struct HttpRequest {
    HttpRequestMethod method;
    std::string path;
    std::string version;
    
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};
#endif
