#include <iostream>
#include <string>
#include <fstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ANSI color codes for a simple, readable UI
#define COLOR_RESET   "\033[0m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_RED     "\033[31m"
#define COLOR_MAGENTA "\033[35m"

// Simple callback for CURL
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    s->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Read API key from .env
std::string readApiKeyFromEnv() {
    std::ifstream envFile(".env");
    std::string line;
    std::string apiKey;

    if (!envFile.is_open()) {
        throw std::runtime_error("Could not open .env file");
    }

    while (std::getline(envFile, line)) {
        if (line.find("CEREBRAS_API_KEY=") == 0) {
            apiKey = line.substr(17); // Length of "CEREBRAS_API_KEY="
            break;
        }
    }

    if (apiKey.empty()) {
        throw std::runtime_error("CEREBRAS_API_KEY not found in .env file");
    }

    return apiKey;
}

// Renamed to Clie
class Clie {
private:
    std::string apiKey;
    CURL* curl;

public:
    Clie(const std::string& key) : apiKey(key) {
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
    }

    ~Clie() {
        if (curl) curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

    std::string ask(const std::string& question) {
        if (!curl) return "Error: CURL not initialized";

        json payload = {
            {"messages", json::array({ {{"role", "user"}, {"content", question}} })},
            {"model", "qwen-3-32b"},
            {"stream", false}
        };

        std::string payloadStr = payload.dump();
        std::string response;

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string authHeader = "Authorization: Bearer " + apiKey;
        headers = curl_slist_append(headers, authHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.cerebras.ai/v1/chat/completions");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            return std::string("Error: ") + curl_easy_strerror(res);
        }

        try {
            json responseJson = json::parse(response);
            return responseJson["choices"][0]["message"]["content"];
        } catch (const std::exception& e) {
            return "Error parsing response: " + std::string(e.what());
        }
    }
};

int main() {
    try {
        std::string apiKey = readApiKeyFromEnv();
        Clie ai(apiKey);
        std::string input;

        std::cout << COLOR_MAGENTA << "\n╔════════════════════════════════════════╗\n";
        std::cout << "║         Welcome to Clie Chat!         ║\n";
        std::cout << "╚════════════════════════════════════════╝\n" << COLOR_RESET;
        std::cout << COLOR_YELLOW << "Type 'exit' to quit\n" << COLOR_RESET;

        while (true) {
            std::cout << COLOR_CYAN << "\nYou: " << COLOR_RESET;
            std::getline(std::cin, input);

            if (input == "exit") break;

            std::string reply = ai.ask(input);
            std::cout << COLOR_GREEN << "\nClie: " << COLOR_RESET << reply << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << COLOR_RED << "Error: " << e.what() << COLOR_RESET << std::endl;
        return 1;
    }

    return 0;
}
