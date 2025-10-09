#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <vector>
#include <stdexcept>

using json = nlohmann::json;

// Callback function for writing received data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
        return newLength;
    } catch(std::bad_alloc& e) {
        // Handle memory problem
        return 0;
    }
}

// Function to load environment variables from .env file
void loadEnvFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open .env file at " << filePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Find the equals sign
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Remove quotes if present
            if (!value.empty() && (value.front() == '"' || value.front() == '\'')) {
                value.erase(0, 1);
            }
            if (!value.empty() && (value.back() == '"' || value.back() == '\'')) {
                value.pop_back();
            }

            // Set environment variable
            #ifdef _WIN32
            _putenv_s(key.c_str(), value.c_str());
            #else
            setenv(key.c_str(), value.c_str(), 1);
            #endif
        }
    }
}

// Cerebras API client class
class CerebrasClient {
private:
    std::string apiKey;
    CURL* curl;

public:
    CerebrasClient(const std::string& apiKey) : apiKey(apiKey) {
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }
    }

    ~CerebrasClient() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }

    void streamChatCompletions(const std::string& model, const std::string& systemPrompt,
                              double temperature = 0.7, double top_p = 0.95, int max_tokens = 16382) {
        if (!curl) {
            std::cerr << "CURL not initialized" << std::endl;
            return;
        }

        // Prepare JSON payload
        json payload = {
            {"messages", json::array({
                {{"role", "system"}, {"content", systemPrompt}}
            })},
            {"model", model},
            {"stream", true},
            {"max_completion_tokens", max_tokens},
            {"temperature", temperature},
            {"top_p", top_p}
        };

        std::string payloadStr = payload.dump();

        // Set up headers
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string authHeader = "Authorization: Bearer " + apiKey;
        headers = curl_slist_append(headers, authHeader.c_str());

        // Set up CURL options
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.cerebras.ai/v1/chat/completions");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        // Process the stream
        std::string buffer;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            // Process the streamed response
            // In a real implementation, you would parse the SSE format
            // Here we're just printing the raw response
            processStreamResponse(buffer);
        }

        curl_slist_free_all(headers);
    }

private:
    void processStreamResponse(const std::string& response) {
        // Split the response by lines (SSE format)
        std::istringstream stream(response);
        std::string line;

        while (std::getline(stream, line)) {
            // Skip empty lines
            if (line.empty() || line == "\r") {
                continue;
            }

            // Check if it's a data line
            if (line.substr(0, 5) == "data:") {
                std::string data = line.substr(5);

                // Skip "[DONE]" message
                if (data == " [DONE]") {
                    continue;
                }

                try {
                    // Parse JSON data
                    json jsonData = json::parse(data);

                    // Extract content from the delta
                    if (jsonData.contains("choices") &&
                        jsonData["choices"].size() > 0 &&
                        jsonData["choices"][0].contains("delta") &&
                        jsonData["choices"][0]["delta"].contains("content")) {

                        std::string content = jsonData["choices"][0]["delta"]["content"];
                        std::cout << content << std::flush;
                    }
                } catch (json::parse_error& e) {
                    // Handle JSON parsing errors
                    std::cerr << "JSON parse error: " << e.what() << std::endl;
                }
            }
        }
    }
};

int main(int argc, char* argv[]) {
    // Load environment variables from .env file
    loadEnvFromFile(".env");

    // Get API key from environment variable
    const char* apiKeyEnv = std::getenv("CEREBRAS_API_KEY");
    if (!apiKeyEnv) {
        std::cerr << "Error: CEREBRAS_API_KEY environment variable not set" << std::endl;
        return 1;
    }

    std::string apiKey = apiKeyEnv;
    std::string model = "qwen-3-32b";
    std::string systemPrompt = "";

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--model" && i + 1 < argc) {
            model = argv[++i];
        } else if (arg == "--system-prompt" && i + 1 < argc) {
            systemPrompt = argv[++i];
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --model MODEL           Specify the model to use (default: qwen-3-32b)" << std::endl;
            std::cout << "  --system-prompt PROMPT  Specify the system prompt" << std::endl;
            std::cout << "  --help                  Show this help message" << std::endl;
            return 0;
        }
    }

    try {
        CerebrasClient client(apiKey);
        client.streamChatCompletions(model, systemPrompt);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
