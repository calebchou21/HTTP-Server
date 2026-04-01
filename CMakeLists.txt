cmake_minimum_required(VERSION 3.23)
project(http_server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(http_core
    src/http_server.cpp
    src/http_parser.cpp
    src/connection.cpp
    src/response_serializer.cpp
    src/file_service.cpp
    src/http_utils.cpp
    src/response.cpp
)

target_include_directories(http_core PUBLIC include/)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE http_core)

add_executable(tests
    tests/main.cpp
    tests/test_file_service.cpp
)
target_link_libraries(tests PRIVATE http_core)
enable_testing()
add_test(NAME tests COMMAND tests)
