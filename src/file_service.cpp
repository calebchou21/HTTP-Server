#include "file_service.h"
#include "http_parser.h"

HttpResponse FileService::serveFile(const HttpRequest &request, const std::filesystem::path &path) {
    bool pathIsValid = HttpParser::isValidPath(path); 
    
    if (!pathIsValid) {
        return buildResponse(HttpStatus::FORBIDDEN, "Forbidden");
    } 

    if (!std::filesystem::exists(path)) {
        return buildResponse(HttpStatus::NOT_FOUND, "Not Found");
    }

    auto it = request.headers.find("If-Modified-Since");
    if (it != request.headers.end()) {
        if (!isModifiedSince(path, it->second)) {
            return buildResponse(HttpStatus::NOT_MODIFIED);
        }
    }

    //TODO: Read in file
}

bool FileService::isModifiedSince(const std::filesystem::path &path, const std::string &headerDate) {
    auto lastWrite = std::filesystem::last_write_time(path);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        lastWrite - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    std::time_t fileTime = std::chrono::system_clock::to_time_t(sctp);

    std::time_t headerTime = HttpParser::parseHttpDate(headerDate);
    return fileTime > headerTime;
}

HttpResponse FileService::buildResponse(HttpStatus status, std::string body) {
    HttpResponse response;
    response.status = status;
    if (!body.empty()) {
        response.body = body;
        response.headers["Content-Length"] = std::to_string(response.body.size());
    } else {
        response.headers["Content-Length"] = "0";
    }
    return response;
}
