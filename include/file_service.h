#ifndef FILE_SERVICE_H
#define FILE_SERVICE_H

#include <filesystem>
#include <string>

#include "response.h"
#include "request.h"

class FileService {
    public:
        static HttpResponse serveFile(const HttpRequest &request, const std::filesystem::path &path);

    private:
        static bool isNotModifiedSince(const std::filesystem::path &path, const std::string &headerDate);
        static std::string getMimeType(const std::filesystem::path &path);
        static std::string getFileContent(const std::filesystem::path &path);
        static bool isValidPath(const std::filesystem::path &path);
        static std::time_t getLastWriteTime(const std::filesystem::path &path);
};
#endif
