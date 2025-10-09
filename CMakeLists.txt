cmake_minimum_required(VERSION 3.10)
project(cerebras_cli)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(CURL QUIET)
find_package(nlohmann_json 3.2.0 QUIET)

# If CURL is not found, fetch it
if(NOT CURL_FOUND)
    include(FetchContent)
    FetchContent_Declare(curl
        DOWNLOAD_EXTRACT_TIMESTAMP true
        URL https://github.com/curl/curl/releases/download/curl-8_10_1/curl-8.10.1.tar.xz
    )
    FetchContent_MakeAvailable(curl)
    set(CURL_LIBRARIES curl)
    set(CURL_INCLUDE_DIRS ${curl_SOURCE_DIR}/include)
endif()

# If nlohmann_json is not found, fetch it
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    FetchContent_Declare(json
        DOWNLOAD_EXTRACT_TIMESTAMP true
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
endif()

# Add executables
add_executable(cerebras_cli src/cerebras_clie.cpp)
add_executable(clie src/clie.cpp)

# Link libraries for CLI
if(CURL_FOUND)
    target_link_libraries(cerebras_cli PRIVATE CURL::libcurl)
    target_link_libraries(clie PRIVATE CURL::libcurl)
else()
    target_link_libraries(cerebras_cli PRIVATE ${CURL_LIBRARIES})
    target_link_libraries(clie PRIVATE ${CURL_LIBRARIES})
    target_include_directories(cerebras_cli PRIVATE ${CURL_INCLUDE_DIRS})
    target_include_directories(clie PRIVATE ${CURL_INCLUDE_DIRS})
endif()
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


