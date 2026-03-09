#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <string>
#include <filesystem>
#include <vector>

#include "request.h"

class HttpParser {
    public:
        HttpParser() = default;
        ~HttpParser() = default;

        bool feed(std::string &data);
        bool isComplete();
        bool getRequest();

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

        bool parseRequestLine();
        bool isValidPath(const std::filesystem::path &path);

        HttpRequestMethod strToRequestMethod(const std::string &str);
        std::vector<std::string> splitBySpace(std::string &str);
};
#endif
