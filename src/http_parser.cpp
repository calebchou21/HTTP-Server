#include <sstream>

#include "http_parser.h"
#include "logger.h"

bool HttpParser::feed(std::string &data) {
    m_buffer.append(data); 

    if (m_state == ParseState::RequestLine) {
        if (m_buffer.find("\r\n", 0) != std::string::npos) {
            parseRequestLine();
            m_state = ParseState::Complete;
            return true;
        }
    }
    
    logger::logError("Failed to successfully parse HTTP request");
    return false;
}

bool HttpParser::parseRequestLine() {
    std::vector<std::string> elements = splitBySpace(m_buffer); 
    
    m_request.method = strToRequestMethod(elements[0]);
    
    logger::logMessage(elements[1]);
    std::filesystem::path path(elements[1]);
    bool pathIsValid = isValidPath(path);
    if (!pathIsValid) {
        logger::logError("Unsafe path requested");
        return false;
    }
    m_request.path = path;
    
    try {
        float version = std::stof(elements[2]);
        if (version > 1.1) {
            logger::logError("Unsupported HTTP Version");
            return false;
        }
        m_request.version = version; 
    } catch (const std::invalid_argument &e) {
        logger::logError("Invalid version (couldn't parse to float)");
        return false;
    }

    return true;
}

bool HttpParser::isComplete() {
    return m_state == ParseState::Complete;
}

/**
 * Determines if a path is "valid" such that it does not contain any relative
 * components. It does NOT determine if a resource actually exists at the requested path.
 */
bool HttpParser::isValidPath(const std::filesystem::path &path) {
    for (const auto& component : path) {
        if (component == "." || component == "..") {
            return false;
        }
    }
    return true;
}

HttpRequestMethod HttpParser::strToRequestMethod(const std::string &str) {
    if (str == "GET") {
        return HttpRequestMethod::GET;
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
