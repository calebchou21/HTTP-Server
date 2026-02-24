#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <system_error>
#include <errno.h>
#include <string.h>

#include "logger.h"
#include "http_server.h"

HttpServer::HttpServer(int port) 
    : m_port(port)
{
    setupSocket();     
    startListening();
}

HttpServer::~HttpServer() 
{
    close(m_socketfd);
}

void HttpServer::setupSocket()
{
    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    int rv;
    if ((rv = getaddrinfo(NULL, std::to_string(m_port).c_str(), &hints, &res)) != 0) {
        throw std::runtime_error(gai_strerror(rv));
    }

    for (p = res; p != NULL; p = p->ai_next) {
        if ((m_socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            logger::logError("Failed to create socket - ", strerror(errno));
            continue;
        }
        
        // Reuse the port if in use
        int yes = 1;
        setsockopt(m_socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

        if (bind(m_socketfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(m_socketfd);
            logger::logError("Failed to bind socket - ", strerror(errno));
            continue;
        }

        break; // If we get here, we have connected successfully!
    }

    if (p == NULL) {
        // Looped off end of linked list without successful bind :(
        throw std::runtime_error("Failed to successfully bind socket");
    }

    char ipstr[INET6_ADDRSTRLEN];
    
    void *addr;
    if (p->ai_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in*)p->ai_addr;
        addr = &(ipv4->sin_addr);
    } else {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*)p->ai_addr;
        addr = &(ipv6->sin6_addr);
    }
    inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);

    logger::logMessage("Successfully bound socket!");
    logger::logMessage("IP Adress: ", ipstr);
    logger::logMessage("Socket type: ", socktypeToString(p->ai_socktype));
    logger::logMessage("Address family: ", addrfamilyToString(p->ai_family));
    logger::logMessage("Protocol: ", protocolToString(p->ai_protocol));

    freeaddrinfo(res);
}

void HttpServer::startListening()
{
    int rv;
    if ((rv = listen(m_socketfd, 20)) == -1) {
        throw std::system_error(errno, std::generic_category(), "Listen failed");
    }

    logger::logMessage("Listening on port ", m_port);

    while (true) {
        struct sockaddr_storage their_addr;
        socklen_t addr_size = sizeof their_addr;
        
        int new_fd;
        new_fd = accept(m_socketfd, (struct sockaddr *)&their_addr, &addr_size);

        if (new_fd == -1) {
            logger::logError("Failed to accept incoming connection - ", strerror(errno));
        }

        char buffer[4096];
        int bytes_read = recv(new_fd, buffer, sizeof buffer, 0);
        if (bytes_read == -1) {
            logger::logError("Failed to receive byte - ", strerror(errno));
        }
        logger::logMessage("Client Connected!");
        logger::logMessage("Received ", bytes_read, " bytes");
        close(new_fd);
    }
}

const char* HttpServer::socktypeToString(int socktype)
{
    switch (socktype) {
        case SOCK_STREAM:
            return "SOCK_STREAM";
        case SOCK_DGRAM:
            return "SOCK_DGRAM";
        default:
            return "OTHER";
    }
}

const char* HttpServer::addrfamilyToString(int addrfamily)
{
    switch (addrfamily) {
        case AF_INET:
            return "INET";
        case AF_INET6:
            return "INET6";
        case AF_UNIX:
            return "UNIX";
        default:
            return "OTHER";
    }
}

const char* HttpServer::protocolToString(int protocol)
{
    switch (protocol) {
        case IPPROTO_TCP:
            return "TCP";
        case IPPROTO_UDP:
            return "UDP";
        case 0:
            return "0 (defualt)";
        default:
            return "OTHER";
    }
}
