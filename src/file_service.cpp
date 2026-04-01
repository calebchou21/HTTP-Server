#include <fstream>

#include "response.h"
#include "http_utils.h"
#include "file_service.h"

HttpResponse FileService::serveFile(const HttpRequest &request, const std::filesystem::path &path) {
    bool pathIsValid = isValidPath(path); 
    
    if (!pathIsValid) {
        return HttpResponse::create(HttpStatus::FORBIDDEN, "Forbidden");
    }

    if (!std::filesystem::exists(path)) {
        return HttpResponse::create(HttpStatus::NOT_FOUND, "Not Found");
    }

    std::time_t lastModified = getLastWriteTime(path);
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string lastModifiedStr = HttpUtils::formatHttpDate(lastModified);
    
    HttpResponse response;
    auto it = request.headers.find("If-Modified-Since");
    if (it != request.headers.end() && request.method != HttpRequestMethod::HEAD) {
        if (!isNotModifiedSince(path, it->second)) {
            response = HttpResponse::create(HttpStatus::NOT_MODIFIED);
        }
    } else {
        std::string body = getFileContent(path);
        HttpResponse response = HttpResponse::create(HttpStatus::OK, body);
    }
    
    if (request.method == HttpRequestMethod::HEAD) {
        response.body = "";
    }

    response.headers["Content-Type"] = getMimeType(path);
    response.headers["Last-Modified"] = lastModifiedStr;
    return response;
}

std::string FileService::getFileContent(const std::filesystem::path &path) {
    auto size = std::filesystem::file_size(path);
    std::string content(size, '\0');
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return ""; // File open / read failed
    }
    in.read(&content[0], size);
    return content;
}

std::string FileService::getMimeType(const std::filesystem::path &path) {
    static const std::unordered_map<std::string, std::string> mimeTypes = {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".txt",  "text/plain"},
        {".png",  "image/png"},
        {".jpg",  "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif",  "image/gif"},
        {".svg",  "image/svg+xml"},
        {".ico",  "image/x-icon"}
    };

    std::string ext = path.extension().string();

    auto it = mimeTypes.find(ext);
    if (it != mimeTypes.end()) {
        return it->second;
    }

    return "application/octet-stream";
}

bool FileService::isNotModifiedSince(const std::filesystem::path &path, const std::string &headerDate) {
    std::time_t fileTime = getLastWriteTime(path);
    std::time_t headerTime = HttpUtils::parseHttpDate(headerDate);
    return fileTime > headerTime;
}

std::time_t FileService::getLastWriteTime(const std::filesystem::path &path) {
    auto lastWrite = std::filesystem::last_write_time(path);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        lastWrite - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    std::time_t fileTime = std::chrono::system_clock::to_time_t(sctp);
    return fileTime;
}

/**
 * Determines if a path is "valid" such that it does not contain any relative
 * components. It does NOT determine if a resource actually exists at the requested path.
 */
bool FileService::isValidPath(const std::filesystem::path &path) {
    for (const auto& component : path) {
        if (component == "." || component == "..") {
            return false;
        }
    }
    return true;
}
