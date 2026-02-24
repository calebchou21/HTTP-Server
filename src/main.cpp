#include <iostream>
#include "http_server.h"

int main(int argc, char *argv[])
{
    try {
        HttpServer server {8080};
    } catch (std::exception &e) {
        std::cout << "\x1b[31m[FATAL ERROR] " <<
            e.what() << "\x1b[0m" << std::endl;
    }
    return 0;
}
