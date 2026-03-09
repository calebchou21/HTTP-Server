#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>

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

    bool readFromSocket();
    //HttpResponse processRequest(const HttpRequest &request);
    bool writeToSocket();
};
#endif
