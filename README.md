# Cerebras API Tools

**Intelligence, unleashed.**
Build fast. Create smart.

Qwen3-32B on Cerebras Inference powers instant reasoning at 2,400 tokens per second. With hybrid reasoning, agentic support, and advanced tool calling, it outshines GPT-4.1 and Claude Sonnet 3.7—open-weight and ready for production.

## What You Get

- **Cerebras CLI**: A sleek command-line tool for instant chat completions. ✅
- **Cerebras Web UI**: An intuitive interface for seamless model interaction.
- **Business Solutions**: We help advance businesses and provide guidance to protect companies through consistent AI-powered solutions.

## Our Mission

We help advance short business operations and carry forward strategic studies to protect and guide companies. Our commitment is consistent, leveraging the power of Qwen3 32B on Cerebras infrastructure and integrating with platforms like [OpenRouter.ai](https://openrouter.ai/) for enhanced AI routing capabilities.

## Start in Minutes

Clone, configure, and build with ease.

```bash
git clone https://github.com/uploader/Cerebras_API_Tools.git
cd Cerebras_API_Tools

# Set your API key
echo "CEREBRAS_API_KEY=your_api_key_here" > .env

# Build
mkdir build
cd build
cmake ..
make
```

### Run CLI
```bash
./cerebras_cli --model qwen-3-32b --prompt "You are a helpful assistant."
```

### Run Web UI
```bash
./cerebras_server
# Visit http://localhost:8080
```

## Requirements

- CMake 3.10+
- C++17-compatible compiler
- libcurl, nlohmann/json libraries

### Install Dependencies

**macOS**
```bash
brew install cmake curl nlohmann-json
```

**Ubuntu/Debian**
```bash
sudo apt update
sudo apt install cmake libcurl4-openssl-dev nlohmann-json3-dev
```

## Build

```bash
mkdir build
cd build
cmake ..
make
```

Update after changes:
```bash
cd build
cmake ..
make
```

## Use

### CLI
```bash
./cerebras_cli --model qwen-3-32b --prompt "You are a helpful assistant."
```

**Options**:
- `--model`: Select model (default: qwen-3-32b)
- `--prompt`: Set system prompt
- `--help`: View all options

**Recommended Prompt**:
```
You are a CLI assistant powered by Qwen3 on Cerebras. Deliver concise, machine-readable output, optimize for speed, and follow best practices.
```

### Web UI
```bash
./cerebras_server --port 8080
```

Visit `http://localhost:8080` to:
- Choose models
- Set prompts
- Chat interactively
- View history

**Options**:
- `--port`: Set port (default: 8080)
- `--help`: View all options

## Setup API Key

Add your Cerebras API key:
- `.env` file: `CEREBRAS_API_KEY=your_api_key_here`
- Environment: `export CEREBRAS_API_KEY=your_api_key_here`

## Examples

**CLI Chat**
```bash
./cerebras_cli --model qwen-3-32b --prompt "You are a helpful assistant."
```

**Web UI**
```bash
./cerebras_server
# Open http://localhost:8080
```

## Troubleshoot

**Build Issues**
- Missing headers? Install dependencies.
- nlohmann/json missing? Run `brew install nlohmann-json` or `sudo apt install nlohmann-json3-dev`.
- libcurl errors? Verify `curl` installation.

**Runtime Issues**
- API key errors? Check `.env` or environment variable.
- Connection issues? Confirm internet and Cerebras API status.
- Web UI not updating? Rebuild after modifying HTML/CSS/JS.

## Project Structure

- `src/`: Source code
  - `cerebras_cli.cpp`: CLI for chat requests
  - `cerebras_server.cpp`: Web server for UI and API
  - `cli.cpp`: Additional CLI implementation
- `web/`: Web UI components
  - `index.html`
  - `styles.css`
  - `script.js`
- `docs/`: Documentation and legal files
- `scripts/`: Git hooks and utilities
- `CMakeLists.txt`: Build configuration
- `.env`: API key storage

## Why Cerebras?

- **Speed**: 2,400 tokens/second
- **Power**: Surpasses GPT-4.1 and Claude Sonnet 3.7
- **Flexibility**: Hybrid reasoning, agentic workflows
- **Openness**: Fully customizable open-weight model
- **Ready**: Production-grade performance

**Build the future.** Harness Cerebras to create, innovate, and inspire.

# Cerebras AI CLI ✅

A simple command-line interface for interacting with the Cerebras AI API.

## Prerequisites

- C++ compiler with C++11 support
- libcurl
- nlohmann-json library

## Installation

1. Install the required dependencies:

```bash
# macOS
brew install curl nlohmann-json
```

## Configuration

1. Create a `.env` file in the project root directory:
```
CEREBRAS_API_KEY=your-api-key-here
```

Replace `your-api-key-here` with your actual Cerebras API key.

## Compilation

Compile the code with C++11 support:

```bash
g++ -std=c++11 -o cli cli.cpp -lcurl -I/opt/homebrew/include
```

## Usage

Run the compiled program:

```bash
./cli
```

The program will start an interactive session where you can:
- Type your questions to the AI
- Get responses from the AI
- Type 'exit' to quit

## Features

- Simple command-line interface
- Secure API key management using .env file
- JSON request/response handling
- Error handling for API requests

## Conventional Commits

This project follows conventional commit standards to maintain a clean and consistent git history.

### Setup

To enable the commit message hook:

```bash
cp scripts/commit-msg .git/hooks/commit-msg
chmod +x .git/hooks/commit-msg
```

### Commit Message Format

Commit messages must:
- Start with a type: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`, `perf`, `ci`, `build`, `revert`
- Be entirely lowercase
- Have a first line ≤60 characters

Example: `feat: add new cli option`

### Rewriting History

To clean up existing commit messages:

```bash
./scripts/rewrite_msg.sh
```

This script lowercases and truncates commit messages to conform to standards.
