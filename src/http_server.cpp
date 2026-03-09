#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <system_error>
#include <errno.h>
#include <string.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <filesystem>
#include <unistd.h>

#include "logger.h"
#include "connection.h"
#include "http_server.h"

HttpServer::HttpServer(int port) 
    : m_port(port)
{
    setupSocket();     
    startListening();
}

HttpServer::~HttpServer() 
{
    // TODO: Make sure this is called when we SIGINT
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
            logger::logError("Failed to create socket: ", strerror(errno));
            continue;
        }
        
        // Reuse the port if in use
        int yes = 1;
        setsockopt(m_socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

        if (bind(m_socketfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(m_socketfd);
            logger::logError("Failed to bind socket: ", strerror(errno));
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
        
        int connectionfd;
        connectionfd = accept(m_socketfd, (struct sockaddr *)&their_addr, &addr_size);
        if (connectionfd == -1) {
            logger::logError("Failed to accept incoming connection: ", strerror(errno));
            continue;
        }
        
        Connection connection(connectionfd);
        connection.handle();
    }
}

// FIXME: Delete this eventually
void HttpServer::handleConnection(int connectionfd)
{
    logger::logMessage("Client Connected!");
    
    std::string request;
    char buffer[4096];
    int headerSearchPos = 0;

    do {
        int bytes_read = recv(connectionfd, buffer, sizeof buffer, 0);
        logger::logMessage("recv'd ", bytes_read, " bytes!");
        
        if (bytes_read == 0) {
            logger::logMessage("Connection closed by peer");
            break;
        }

        if (bytes_read <= 0) {
            logger::logError("recv failed:", strerror(errno));
            break;
        }

        request.append(buffer, bytes_read);
        
        size_t header_end = request.find("\r\n\r\n", headerSearchPos);
        if (header_end != std::string::npos) {
            // We have headers! This is the entire request in HTTP/0.9
            logger::logMessage(request);
            std::string path = request.substr(4);
            size_t path_end = path.find(" ");
            if (path_end == std::string::npos) {
                logger::logError("Failed to identify path to resource");
                break;
            }
            
            path = path.substr(0, path_end);

            if (path == "/") {
                path = "/index.html";
            }
            

            std::string pathToResource = "website" + path;
            
            // TODO: Open file, read data into buffer, transmit chunks of data to client, close connection

            std::uintmax_t fileSize = std::filesystem::file_size(pathToResource);

            int filefd = open(pathToResource.c_str(), O_RDONLY);
            if (filefd == -1) {
                logger::logError("Failed to open file");
                return;
            }

            ssize_t totalSent = 0;
            off_t offset = 0;
            
            while (totalSent < fileSize) {
                int sent = sendfile(connectionfd, filefd, &offset, fileSize - totalSent);

                if (sent == -1) {
                    logger::logError("Send failed");
                    return;
                }

                totalSent += sent;
            }

            logger::logMessage(path);
            return;
        }

        headerSearchPos += bytes_read;
    } while (request.size() < 16384); // 16KB limit

    close(connectionfd);
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
