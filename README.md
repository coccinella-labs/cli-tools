# Cerebras CLI

**intelligence, unleashed.**  
build fast. create smart.

qwen3-32b on cerebras inference powers instant reasoning at 2,400 tokens per second — hybrid reasoning, agentic support, and tool calling ready for production.

---

## what it is

- **cerebras cli** — instant chat completions from the terminal.  
- **cerebras web ui** — simple browser interface for model interaction.  
- **open integration** — built for qwen3 32b and compatible with [openrouter.ai](https://openrouter.ai/).

---

## quick start

```bash
git clone https://github.com/bniladridas/cli.git
cd cli

# set your api key
echo "CEREBRAS_API_KEY=your_api_key_here" > .env

# build
mkdir build && cd build
cmake ..
make
````

### run cli

```bash
./cerebras_cli --model qwen-3-32b --prompt "you are a helpful assistant."
```

### run web ui

```bash
./cerebras_server
# visit http://localhost:8080
```

---

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

---

## structure

* `src/` — cli and server source
* `web/` — html, css, js ui
* `scripts/` — utilities and build helpers
* `CMakeLists.txt` — build config

---

## why {clie}?

* 2,400 tokens/sec inference speed
* hybrid reasoning and agentic workflows
* open-weight, production-ready foundation

**build the future with cerebras.**
