#include <unistd.h>
#include <string.h>
#include <fstream>
#include <sys/socket.h>

#include "connection.h"
#include "response.h"
#include "response_serializer.h"
#include "file_service.h"
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
            HttpResponse response = processRequest(m_parser.getRequest());
            std::string serializedResponse = ResponseSerializer::serializeResponse(response);
            return writeToSocket(serializedResponse);
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
    std::filesystem::path path = std::filesystem::path(SERVE_FROM) /
                                    request.path.substr(1);

    if (std::filesystem::is_directory(path)) {
        path /= "index.html";
    }

    switch (request.method) {
        case HttpRequestMethod::GET:
        case HttpRequestMethod::HEAD:
            return FileService::serveFile(request, path);
        case HttpRequestMethod::POST:
            return handlePost(request);
        default:
            HttpResponse response = HttpResponse::create(HttpStatus::NOT_IMPLEMENTED);
            response.headers["Allow"] = "GET, HEAD, POST";
            return response;
    }

    if (request.method == HttpRequestMethod::GET) {
        return FileService::serveFile(request, path);
    }

    return HttpResponse::create(HttpStatus::BAD_REQUEST);
}

HttpResponse Connection::handlePost(const HttpRequest &request) {
    if (request.path != "/post_to_this.html") {
        return HttpResponse::create(HttpStatus::NOT_IMPLEMENTED); // 405 not defined in HTTP/1.0 spec
    }
    
    auto it = request.headers.find("Content-Length");
    if (it == request.headers.end()) {
        return HttpResponse::create(HttpStatus::BAD_REQUEST); 
    }
    
    int bodyLen;
    try {
        bodyLen = std::stoi(it->second);
    } catch (const std::exception &e) {
        // If length is not parsable to int for some reason, return 400
        return HttpResponse::create(HttpStatus::BAD_REQUEST);
    }

    if (request.body.size() != static_cast<size_t>(bodyLen)) {
        return HttpResponse::create(HttpStatus::BAD_REQUEST);
    }
    
    std::filesystem::path postToPath = std::filesystem::path(SERVE_FROM) / "post_to_this.html";
    std::ofstream file(postToPath); // overwrite mode
    if (!file) {
        return HttpResponse::create(HttpStatus::INTERNAL_SERVER_ERROR);
    }
    file << request.body;
    file.close();
    
    std::string body = R"(
        <html>
          <body>
            <h1>201 Created</h1>
            <a href="./post_to_this.html" target=_blank>Click here!</a>
          </body>
        </html>
        )";
    HttpResponse response = HttpResponse::create(HttpStatus::CREATED, body);
    response.headers["Content-Type"] = "text/html";
    return response;
}

bool Connection::writeToSocket(const std::string &serializedResponse) {
    size_t totalSent = 0;
    const char *ptr = serializedResponse.c_str();
    size_t remaining = serializedResponse.size();

    while (totalSent < serializedResponse.size()) {
        ssize_t sent = send(m_fd, ptr + totalSent, remaining, 0);

        if (sent == -1) {
            logger::logError("Failed to send message to client");
            return false;
        }

        totalSent += sent;
        remaining -= sent;
    }

    return true;
}

