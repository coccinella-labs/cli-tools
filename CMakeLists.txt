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
add_executable(clie src/cli.cpp)

# Link libraries for CLI
target_link_libraries(cerebras_cli PRIVATE CURL::libcurl)
target_link_libraries(clie PRIVATE CURL::libcurl)
if(nlohmann_json_FOUND)
    target_link_libraries(cerebras_cli PRIVATE nlohmann_json::nlohmann_json)
    target_link_libraries(clie PRIVATE nlohmann_json::nlohmann_json)
else()
    target_link_libraries(cerebras_cli PRIVATE nlohmann_json::nlohmann_json)
    target_link_libraries(clie PRIVATE nlohmann_json::nlohmann_json)
endif()

# Include directories
target_include_directories(cerebras_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(clie PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Install
install(TARGETS cerebras_cli clie DESTINATION bin)


