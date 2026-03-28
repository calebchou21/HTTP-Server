#ifndef CONNECTION_H
#define CONNECTION_H

#include <filesystem>
#include <string>

#include "response.h"
#include "http_parser.h"

class Connection {
public:
    explicit Connection(int fd);
    ~Connection();

    void handle();

private:
    int m_fd;
    int m_requestSize = 0; // bytes
    std::string m_writeBuffer;
    bool m_shouldClose = false;
    HttpParser m_parser;
    static constexpr std::string_view SERVE_FROM = "website";
   
    bool isModifiedSince(const std::filesystem::path &path, const std::string &headerDate);
    bool readFromSocket();
    HttpResponse processRequest(const HttpRequest &request);
    std::string formatResponse(const HttpResponse &response);
    bool writeToSocket();
};
#endif
