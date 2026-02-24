#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

class HttpServer 
{
    public:
        HttpServer(int port);
        ~HttpServer();


    private:
        int m_socketfd;
        int m_port;
        
        void setupSocket();
        void startListening();
        
        const char* socktypeToString(int socktype);
        const char* addrfamilyToString(int addrfamily);
        const char* protocolToString(int protocol);
};
#endif
