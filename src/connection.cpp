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

//TODO: This should be called something like "format response"
std::string Connection::formatResponse(const HttpResponse &response) {
    std::string builtResponse;
}


bool writeToSocket() {}
