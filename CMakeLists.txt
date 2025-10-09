cmake_minimum_required(VERSION 3.10)
project(cerebras_cli)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json 3.2.0 QUIET)
find_package(Threads REQUIRED)

# If nlohmann_json is not found, fetch it
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
endif()

# Add executables
add_executable(cerebras_cli src/cerebras_cli.cpp)
add_executable(cerebras_server src/cerebras_server.cpp)
add_executable(cli src/cli.cpp)

# Link libraries for CLI
target_link_libraries(cerebras_cli PRIVATE CURL::libcurl)
target_link_libraries(cli PRIVATE CURL::libcurl)
if(nlohmann_json_FOUND)
    target_link_libraries(cerebras_cli PRIVATE nlohmann_json::nlohmann_json)
    target_link_libraries(cli PRIVATE nlohmann_json::nlohmann_json)
else()
    target_link_libraries(cerebras_cli PRIVATE nlohmann_json::nlohmann_json)
    target_link_libraries(cli PRIVATE nlohmann_json::nlohmann_json)
endif()

# Link libraries for Server
target_link_libraries(cerebras_server PRIVATE CURL::libcurl)
target_link_libraries(cerebras_server PRIVATE Threads::Threads)
if(nlohmann_json_FOUND)
    target_link_libraries(cerebras_server PRIVATE nlohmann_json::nlohmann_json)
else()
    target_link_libraries(cerebras_server PRIVATE nlohmann_json::nlohmann_json)
endif()

# Include directories
target_include_directories(cerebras_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(cerebras_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Install
install(TARGETS cerebras_cli cerebras_server cli DESTINATION bin)

# Copy web files to build directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/web/index.html ${CMAKE_CURRENT_BINARY_DIR}/index.html COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/web/styles.css ${CMAKE_CURRENT_BINARY_DIR}/styles.css COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/web/script.js ${CMAKE_CURRENT_BINARY_DIR}/script.js COPYONLY)
