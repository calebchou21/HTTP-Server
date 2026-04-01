#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <string>
#include <vector>

#include "request.h"

class HttpParser {
    public:
        HttpParser() = default;
        ~HttpParser() = default;

        bool feed(std::string &data);
        bool isComplete();
        HttpRequest getRequest();

    private:
        enum class ParseState {
            RequestLine,
            Headers,
            Body,
            Complete
        };

        ParseState m_state = ParseState::RequestLine;
        std::string m_buffer;
        HttpRequest m_request;
        int m_startParseIndex = 0;

        bool parseRequestLine();
        bool parseHeader(const std::string &header);
        HttpRequestMethod strToRequestMethod(const std::string &str);
        std::vector<std::string> splitBySpace(std::string &str);
        
        std::string methodToString(HttpRequestMethod method); 
        void logRequestLine();
        void logHeaders();
};
#endif
