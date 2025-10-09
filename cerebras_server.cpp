#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <stdexcept>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

// For socket programming
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using json = nlohmann::json;

// Thread-safe queue for handling requests
template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable cond;

public:
    void push(T item) {
        std::unique_lock<std::mutex> lock(mutex);
        queue.push(std::move(item));
        lock.unlock();
        cond.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [this] { return !queue.empty(); });
        T item = std::move(queue.front());
        queue.pop();
        return item;
    }

    bool empty() {
        std::unique_lock<std::mutex> lock(mutex);
        return queue.empty();
    }
};

// HTTP request structure
struct HttpRequest {
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;
};

// HTTP response structure
struct HttpResponse {
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;
};

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

    std::string chatCompletions(const std::string& model, const std::string& systemPrompt,
                              const std::string& userPrompt, double temperature = 0.7,
                              double top_p = 0.95, int max_tokens = 16382) {
        if (!curl) {
            throw std::runtime_error("CURL not initialized");
        }

        // Prepare JSON payload
        json payload = {
            {"messages", json::array({
                {{"role", "system"}, {"content", systemPrompt}},
                {{"role", "user"}, {"content", userPrompt}}
            })},
            {"model", model},
            {"stream", false},
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

        // Process the response
        std::string buffer;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        CURLcode res = curl_easy_perform(curl);

        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            throw std::runtime_error(std::string("curl_easy_perform() failed: ") + curl_easy_strerror(res));
        }

        return buffer;
    }
};

// HTTP server class
class HttpServer {
private:
    int server_fd;
    int port;
    bool running;
    std::thread server_thread;
    ThreadSafeQueue<std::function<void()>> task_queue;
    std::vector<std::thread> worker_threads;
    std::string apiKey;

    // Function to parse HTTP request
    HttpRequest parseRequest(const std::string& request_str) {
        HttpRequest req;
        std::istringstream stream(request_str);
        std::string line;

        // Parse request line
        std::getline(stream, line);
        std::istringstream request_line(line);
        request_line >> req.method >> req.path;

        // Parse headers
        while (std::getline(stream, line) && line != "\r") {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                // Trim leading/trailing whitespace
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of("\r\n") + 1);
                req.headers[key] = value;
            }
        }

        // Parse body if Content-Length is present
        if (req.headers.find("Content-Length") != req.headers.end()) {
            int content_length = std::stoi(req.headers["Content-Length"]);
            char* body_buffer = new char[content_length + 1];
            stream.read(body_buffer, content_length);
            body_buffer[content_length] = '\0';
            req.body = body_buffer;
            delete[] body_buffer;
        }

        return req;
    }

    // Function to serialize HTTP response
    std::string serializeResponse(const HttpResponse& res) {
        std::ostringstream stream;
        stream << "HTTP/1.1 " << res.status_code << " ";

        // Status text
        switch (res.status_code) {
            case 200: stream << "OK"; break;
            case 404: stream << "Not Found"; break;
            case 500: stream << "Internal Server Error"; break;
            default: stream << "Unknown"; break;
        }
        stream << "\r\n";

        // Headers
        for (const auto& header : res.headers) {
            stream << header.first << ": " << header.second << "\r\n";
        }

        // Content-Length header
        stream << "Content-Length: " << res.body.length() << "\r\n";

        // End of headers
        stream << "\r\n";

        // Body
        stream << res.body;

        return stream.str();
    }

    // Function to handle client connection
    void handleClient(int client_fd) {
        char buffer[4096] = {0};
        ssize_t bytes_read = read(client_fd, buffer, 4096);

        if (bytes_read > 0) {
            HttpRequest req = parseRequest(buffer);
            HttpResponse res = routeRequest(req);
            std::string response_str = serializeResponse(res);
            write(client_fd, response_str.c_str(), response_str.length());
        }

        close(client_fd);
    }

    // Function to route request to appropriate handler
    HttpResponse routeRequest(const HttpRequest& req) {
        // Serve static files
        if (req.method == "GET") {
            if (req.path == "/" || req.path == "/index.html") {
                return serveFile("index.html", "text/html");
            } else if (req.path == "/styles.css") {
                return serveFile("styles.css", "text/css");
            } else if (req.path == "/script.js") {
                return serveFile("script.js", "text/javascript");
            } else if (req.path == "/legal.html") {
                return serveFile("legal.html", "text/html");
            }
        }

        // Handle API requests
        if (req.method == "POST" && req.path == "/api/chat") {
            return handleChatRequest(req);
        }

        // 404 Not Found
        HttpResponse res;
        res.status_code = 404;
        res.headers["Content-Type"] = "text/plain";
        res.body = "404 Not Found";
        return res;
    }

    // Function to serve static file
    HttpResponse serveFile(const std::string& filename, const std::string& content_type) {
        HttpResponse res;
        std::ifstream file(filename);

        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            res.status_code = 200;
            res.headers["Content-Type"] = content_type;
            res.body = buffer.str();
        } else {
            res.status_code = 404;
            res.headers["Content-Type"] = "text/plain";
            res.body = "404 Not Found";
        }

        return res;
    }

    // Function to handle chat API request
    HttpResponse handleChatRequest(const HttpRequest& req) {
        HttpResponse res;

        try {
            json request_data = json::parse(req.body);
            std::string model = request_data["model"];
            std::string system_prompt = request_data["system_prompt"];
            std::string user_prompt = request_data["user_prompt"];

            CerebrasClient client(apiKey);
            std::string response = client.chatCompletions(model, system_prompt, user_prompt);

            // Parse the response and extract only the final content
            json response_json = json::parse(response);
            if (response_json.contains("choices") &&
                response_json["choices"].size() > 0 &&
                response_json["choices"][0].contains("message") &&
                response_json["choices"][0]["message"].contains("content")) {

                std::string content = response_json["choices"][0]["message"]["content"];
                // Extract only the final response after the last "Let me"
                size_t last_let_me = content.rfind("Let me");
                if (last_let_me != std::string::npos) {
                    size_t last_newline = content.find('\n', last_let_me);
                    if (last_newline != std::string::npos) {
                        content = content.substr(last_newline + 1);
                    }
                }
                response_json["choices"][0]["message"]["content"] = content;
                response = response_json.dump();
            }

            res.status_code = 200;
            res.headers["Content-Type"] = "application/json";
            res.body = response;
        } catch (const std::exception& e) {
            res.status_code = 500;
            res.headers["Content-Type"] = "application/json";
            json error = {{"error", e.what()}};
            res.body = error.dump();
        }

        return res;
    }

    // Server main loop
    void serverLoop() {
        struct sockaddr_in address;
        int addrlen = sizeof(address);

        // Create socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            std::cerr << "Socket failed" << std::endl;
            return;
        }

        // Set socket options
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            std::cerr << "Setsockopt failed" << std::endl;
            return;
        }

        // Bind socket to port
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            std::cerr << "Bind failed" << std::endl;
            return;
        }

        // Listen for connections
        if (listen(server_fd, 10) < 0) {
            std::cerr << "Listen failed" << std::endl;
            return;
        }

        std::cout << "Server listening on port " << port << std::endl;

        // Accept connections
        while (running) {
            int new_socket;
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                if (!running) break;
                std::cerr << "Accept failed" << std::endl;
                continue;
            }

            // Add client handling task to queue
            task_queue.push([this, new_socket]() {
                handleClient(new_socket);
            });
        }
    }

    // Worker thread function
    void workerLoop() {
        while (running) {
            auto task = task_queue.pop();
            task();
        }
    }

public:
    HttpServer(int port = 8080) : port(port), running(false) {
        // Load API key from environment
        const char* apiKeyEnv = std::getenv("CEREBRAS_API_KEY");
        if (!apiKeyEnv) {
            std::cerr << "Warning: CEREBRAS_API_KEY environment variable not set" << std::endl;
            apiKey = "";
        } else {
            apiKey = apiKeyEnv;
        }
    }

    ~HttpServer() {
        stop();
    }

    void start() {
        if (running) return;

        running = true;

        // Start worker threads
        for (int i = 0; i < 4; i++) {
            worker_threads.emplace_back(&HttpServer::workerLoop, this);
        }

        // Start server thread
        server_thread = std::thread(&HttpServer::serverLoop, this);

        std::cout << "Server started" << std::endl;
    }

    void stop() {
        if (!running) return;

        running = false;

        // Close server socket to unblock accept
        close(server_fd);

        // Wait for server thread to finish
        if (server_thread.joinable()) {
            server_thread.join();
        }

        // Add empty tasks to unblock worker threads
        for (size_t i = 0; i < worker_threads.size(); i++) {
            task_queue.push([](){});
        }

        // Wait for worker threads to finish
        for (auto& thread : worker_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        worker_threads.clear();

        std::cout << "Server stopped" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    // Load environment variables from .env file
    loadEnvFromFile(".env");

    // Parse command line arguments
    int port = 8080;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --port PORT             Specify the port to listen on (default: 8080)" << std::endl;
            std::cout << "  --help                  Show this help message" << std::endl;
            return 0;
        }
    }

    try {
        HttpServer server(port);
        server.start();

        std::cout << "Press Enter to stop the server..." << std::endl;
        std::cin.get();

        server.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
