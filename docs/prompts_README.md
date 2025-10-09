# Cerebras Prompt Library

> **Powered by Qwen3 32B on Cerebras**: Up to 2,400 t/s, with hybrid reasoning modes, agentic support, and advanced tool calling. Outperforms GPT-4.1 and Claude Sonnet 3.7 — but runs faster, open-weight, and ready to deploy.

This repository contains a collection of reusable prompts designed for hardware, machine learning, and diagnostic applications. These prompts are optimized for use with Qwen3 32B on Cerebras infrastructure but can be adapted for other LLMs.

## Prompt Collections

This library includes three main prompt collections:

1. **Hardware System Prompts** (`hardware_system_prompts.md`)
   - Specialized prompts for hardware interaction, optimization, and control
   - Includes prompts for CLI assistants, hardware optimization, monitoring, deployment, and security

2. **ML Optimization Prompts** (`ml_optimization_prompts.md`)
   - Prompts focused on machine learning, model optimization, and AI infrastructure
   - Covers training optimization, architecture design, hyperparameter tuning, infrastructure, deployment, and debugging

3. **Diagnostic Troubleshooting Prompts** (`diagnostic_troubleshooting_prompts.md`)
   - Prompts for hardware diagnostics, system troubleshooting, and technical problem-solving
   - Includes diagnostics, performance analysis, log interpretation, network troubleshooting, root cause analysis, and recovery

## How to Use These Prompts

### Basic Usage

1. **Select the appropriate prompt** from one of the collections based on your task
2. **Copy the prompt text** from the markdown file
3. **Paste it as the system prompt** when using the Cerebras API or CLI
4. **Interact with the model** to accomplish your specific task

### Example with Cerebras CLI

```bash
./cerebras_cli --model qwen-3-32b --system-prompt "You are a Hardware Diagnostician powered by Qwen3 on Cerebras infrastructure. Your purpose is to help users diagnose hardware issues through systematic analysis of symptoms, tests, and system information. [...]"
```

### Example with Cerebras API

```python
import os
from cerebras.cloud.sdk import Cerebras

client = Cerebras(
    api_key=os.environ.get("CEREBRAS_API_KEY")
)

# Load the prompt from file
with open('hardware_system_prompts.md', 'r') as file:
    content = file.read()
    # Extract the Hardware CLI Assistant prompt (between the first set of triple backticks)
    import re
    hardware_cli_prompt = re.search(r'## Hardware CLI Assistant\n\n```\n(.*?)\n```', content, re.DOTALL).group(1)

response = client.chat.completions.create(
    messages=[
        {
            "role": "system",
            "content": hardware_cli_prompt
        },
        {
            "role": "user",
            "content": "status"
        }
    ],
    model="qwen-3-32b",
    max_completion_tokens=16382,
    temperature=0.7,
    top_p=0.95
)

print(response.choices[0].message.content)
```

### Example with Web UI

1. Start the Cerebras Web UI server:
   ```bash
   ./cerebras_server
   ```

2. Open your browser and navigate to `http://localhost:8080`

3. Click the "Use CLI Prompt" button or paste your selected prompt into the System Prompt field

4. Start interacting with the model

## Customizing Prompts

These prompts are designed to be templates that you can customize for your specific needs:

1. **Add domain-specific knowledge** relevant to your hardware or systems
2. **Adjust the level of technical detail** based on your users' expertise
3. **Incorporate specific tools or commands** used in your environment
4. **Modify the interaction style** to match your preferred communication approach
5. **Combine elements from different prompts** to create specialized assistants

## Best Practices

1. **Be specific**: The more specific your prompt, the more focused the model's responses will be
2. **Include examples**: When appropriate, include examples of expected inputs and outputs
3. **Set boundaries**: Clearly define what the model should and shouldn't do
4. **Test and iterate**: Refine prompts based on the quality of responses
5. **Consider context limits**: Be mindful of the model's context window when creating very detailed prompts

## Contributing

Feel free to add new prompts or improve existing ones. When contributing:

1. Follow the established format for consistency
2. Include clear descriptions of the prompt's purpose
3. Organize prompts into the appropriate collection
4. Test your prompts with the Cerebras API before submitting

## License

These prompts are provided for use with Cerebras systems and can be freely adapted for your specific needs.
