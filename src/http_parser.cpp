#include <sstream>
#include <filesystem>
#include <string>

#include "http_parser.h"
#include "logger.h"

bool HttpParser::feed(std::string &data) {
    m_buffer.append(data); 
    while (true) {
        size_t lineEnd = m_buffer.find("\r\n");

        switch (m_state) {
            case ParseState::RequestLine: 
                if (lineEnd == std::string::npos) {
                    break; 
                }

                if (!parseRequestLine())
                    return false;
                
                m_buffer.erase(0, lineEnd + 2); // Account for CRLF
                logRequestLine();
                m_state = ParseState::Headers;
                break;
            case ParseState::Headers: {
                if (lineEnd == std::string::npos) {
                    break; 
                }

                const std::string header = m_buffer.substr(0, lineEnd);
                if (header.empty()) {
                    m_buffer.erase(0, 2);
                    logHeaders();

                    auto it = m_request.headers.find("Content-Length");
                    if (it != m_request.headers.end()) {
                        m_state = ParseState::Body;
                    } else {
                        m_state = ParseState::Complete;
                    }
                    break;
                }

                parseHeader(header);
                m_buffer.erase(0, lineEnd + 2); // Account for CRLF
                break;
            }
            case ParseState::Body: {
                int length = std::stoi(m_request.headers["Content-Length"]);
                if (m_buffer.size() < length) {
                    continue;
                }
                
                m_request.body = m_buffer.substr(0, length);
                logger::logMessage("Body: ", m_request.body);
                m_state = ParseState::Complete;
                break;
            }
            case ParseState::Complete:
                return true;
            default:
                logger::logError("Parse in invalid state.");
                return false;
        }
    }
    return true;    
}

HttpRequest HttpParser::getRequest() {
    return m_request;
}

bool HttpParser::parseHeader(const std::string &header) {
    size_t colonPos = header.find(":");
    if (colonPos == std::string::npos) {
        logger::logError("Headers are not formatted properly");
        return false;
    }
    
    std::string key = header.substr(m_startParseIndex, colonPos);
    std::string val = header.substr(colonPos + 2, header.length()); // + 2 accounts for leading space

    m_request.headers.insert({key, val});

    return true;
}

bool HttpParser::parseRequestLine() {
    std::vector<std::string> elements = splitBySpace(m_buffer); 
    
    m_request.method = strToRequestMethod(elements[0]);
    if (m_request.method == HttpRequestMethod::UNKNOWN) {
        logger::logError("Unknown request method");
        return false;
    }
    
    std::filesystem::path path(elements[1]);
    m_request.path = path;
    
    try {
        size_t startVersion = elements[2].find("/");
        if (startVersion == std::string::npos) {
            logger::logError("HTTP version formatted incorrectly (couldn't find '/'");
            return false;
        }
        
        // HTTP/x.x
        int major = elements[2].at(5) - '0';
        int minor = elements[2].at(7) - '0';
        if (major > 1 || minor > 1) {
            logger::logError("Unsupported HTTP Version");
            return false;
        }

        m_request.version = std::to_string(major) + "." + std::to_string(minor); 
    } catch (const std::invalid_argument &e) {
        logger::logError("Invalid version (couldn't parse major and/or minor HTTP version to decimal");
        return false;
    }

    return true;
}

bool HttpParser::isComplete() {
    return m_state == ParseState::Complete;
}

HttpRequestMethod HttpParser::strToRequestMethod(const std::string &str) {
    if (str == "GET") {
        return HttpRequestMethod::GET;
    }

    if (str == "POST") {
        return HttpRequestMethod::POST;
    }
    
    return HttpRequestMethod::UNKNOWN;
}

std::vector<std::string> HttpParser::splitBySpace(std::string &str) {
    std::vector<std::string> words;
    std::stringstream ss(str);
    std::string word;

    while (ss >> word) {
        words.push_back(word);
    }

    return words;
}

void HttpParser::logRequestLine() {
    std::string method = methodToString(m_request.method); 

    logger::logMessage("Request Line: ", method, " ", m_request.path, " ",
            m_request.version);
}

void HttpParser::logHeaders() {
    std::string headers;
    for (const auto& pair : m_request.headers) {
        headers += "\t" + pair.first + ": " + pair.second + "\n";
    }
    
    if (!headers.empty()) {
        headers.pop_back();
    }

    logger::logMessage("Headers: \n", headers);
}

std::string HttpParser::methodToString(HttpRequestMethod method) {
    switch (method) {
        case HttpRequestMethod::GET:
            return "GET";
        case HttpRequestMethod::POST:
            return "POST";
        case HttpRequestMethod::UNKNOWN:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}

