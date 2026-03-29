#ifndef RESPONSE_SERIALIZER
#define RESPONSE_SERIALIZER

#include <string>
#include "response.h"

class ResponseSerializer {
    public:
        static std::string serializeResponse(const HttpResponse &response);

    private:
        static std::string_view statusMethodToPhrase(HttpStatus status);
        static std::string serializeHeaders(const HttpResponse &response);
};
#endif
