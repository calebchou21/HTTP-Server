#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#include "connection.h"
#include "logger.h"

Connection::Connection(int fd): m_fd(fd) {}

Connection::~Connection() {
    if (m_fd != -1) {
        close(m_fd);
    }
}

void Connection::handle() {
    logger::logMessage("Client connected!");

    if (!readFromSocket()) {
        logger::logError("Failed to read from socket");
    }
}

bool Connection::readFromSocket() {
    char buffer[4096];
    
    while (true) {
        if (m_parser.isComplete()) {
            processRequest(m_parser.getRequest());
            return true;
        }

        int bytesRead = recv(m_fd, buffer, sizeof buffer, 0);
        logger::logMessage("Received ", bytesRead, " bytes!");

        if (bytesRead == 0) {
            logger::logMessage("Connection closed by peer");
            return false;
        }

        if (bytesRead <= 0) {
            logger::logError("recv failed: ", strerror(errno));
            return false;
        }
        
        std::string chunk(buffer, bytesRead);
        if (!m_parser.feed(chunk)) {
            logger::logError("An error occured while parsing request");
            return false;
        }

        m_requestSize += bytesRead;
        
        if (m_requestSize > 16384) {
            logger::logError("Request exceeds 16KB limit");
            return false;
        }
    }
}

HttpResponse Connection::processRequest(const HttpRequest &request) {
    HttpResponse response;
    std::filesystem::path path = std::filesystem::path(SERVE_FROM) / request.path;
    if (request.method == HttpRequestMethod::GET) {
        bool pathIsValid = isValidPath(path);

        if (!pathIsValid) {
            response.status = HttpStatus::FORBIDDEN; 
            response.body = "Forbidden";
            response.headers["Content-Length"] = std::to_string(response.body.size());
            return response;
        }

        if (!std::filesystem::exists(path)) {
            response.status = HttpStatus::NOT_FOUND;
            response.body = "Not Found";
            response.headers["Content-Length"] = std::to_string(response.body.size());
            return response;
        }
        
        auto it = request.headers.find("If-Modified-Since");
        if (it != request.headers.end()) {
            if (!isModifiedSince(path, it->second)) {
                response.status = HttpStatus::NOT_MODIFIED;
                response.headers["Content-Length"] = "0";
                return response;
            }
        } else {
            response.status = HttpStatus::OK;
        }
    }
}

bool Connection::isModifiedSince(const std::filesystem::path &path, const std::string &headerDate) {
    auto lastWrite = std::filesystem::last_write_time(path);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        lastWrite - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    std::time_t fileTime = std::chrono::system_clock::to_time_t(sctp);

    std::time_t headerTime = HttpParser::parseHttpDate(headerDate);
    return fileTime > headerTime;
}

//TODO: This should be called something like "format response"
std::string Connection::formatResponse(const HttpResponse &response) {
    std::string builtResponse;
}

/**
 * Determines if a path is "valid" such that it does not contain any relative
 * components. It does NOT determine if a resource actually exists at the requested path.
 */
bool Connection::isValidPath(const std::filesystem::path &path) {
    for (const auto& component : path) {
        if (component == "." || component == "..") {
            return false;
        }
    }
    return true;
}

bool writeToSocket() {}
