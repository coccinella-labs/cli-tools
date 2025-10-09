# {Clie} — CLI

<div align="center">
  <img width="730" alt="Screenshot 2025-05-24 at 12 54 09" src="https://github.com/user-attachments/assets/500ea5be-20a7-42bc-b5ab-6b45f95cc199" />
</div>

**intelligence, unleashed.**  
build fast. create smart.

qwen3-32b inference delivers 2,400 tokens/sec with hybrid reasoning and tool support.

## what it is

- **CLI** — terminal chat completions  
- **Web UI** — browser interface  
- **Integration** — compatible with [openrouter.ai](https://openrouter.ai/)

## quick start

```bash
git clone https://github.com/bniladridas/cli.git
cd cli
echo "API_KEY=your_api_key_here" > .env
mkdir build && cd build
cmake ..
make
````

### run cli

```bash
./build/cli --model qwen-3-32b --prompt "you are a helpful assistant."
```

### run web ui

```bash
./build/cerebras_server
# visit http://localhost:8080
```

## requirements

* cmake 3.10+
* c++17 compiler
* libcurl, nlohmann/json

**macos**

```bash
brew install cmake curl nlohmann-json
```

**ubuntu/debian**

```bash
sudo apt install cmake libcurl4-openssl-dev nlohmann-json3-dev
```

## structure

* `src/` — cli and server