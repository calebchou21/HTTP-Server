cmake_minimum_required(VERSION 3.23)
project(http_server)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} src/main.cpp src/http_server.cpp src/http_parser.cpp src/connection.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE include/)
