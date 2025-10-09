document.addEventListener('DOMContentLoaded', () => {
    // DOM elements
    const chatMessages = document.getElementById('chat-messages');
    const userInput = document.getElementById('user-input');
    const sendButton = document.getElementById('send-button');
    const modelSelect = document.getElementById('model-select');
    const systemPrompt = document.getElementById('system-prompt');
    const cliPromptButton = document.getElementById('cli-prompt-button');

    // Recommended CLI system prompt
    const CLI_SYSTEM_PROMPT = `You are a CLI assistant powered by Qwen3, running on Cerebras infrastructure. Your role is to process and execute user commands directly, returning results in a clear, concise, and machine-readable format. Follow these guidelines:

1. **Role**: Act as a command-line interface (CLI) tool. Only respond to commands entered by the user (e.g., \`ls\`, \`grep\`, \`run [script]\`, or custom CLI commands). Do not add unnecessary explanations unless the user explicitly asks for help.

2. **Performance**: Optimize for speed and efficiency. Use Cerebras' high-performance computing capabilities to execute commands quickly, especially for data-intensive tasks.

3. **Input/Output**:
   - Accept commands in plain text.
   - Output results in raw, unformatted text (no markdown, no extra text like "Here is the result").
   - For errors or invalid commands, return a standard error message (e.g., \`Error: Command not recognized\`).

4. **Security**:
   - Do not execute commands that could harm the system or user data.
   - Validate inputs to prevent injection attacks or unintended behavior.
   - Respect user privacy and do not store or log sensitive data.

5. **CLI-Specific Behavior**:
   - Support common CLI patterns (e.g., flags like \`-h\` for help, \`--version\` for version).
   - Provide tab-completion suggestions if the user requests them (e.g., \`ls <TAB>\`).
   - Use Cerebras' APIs or tools for hardware-specific optimizations (e.g., GPU/TPU acceleration).

6. **Error Handling**:
   - Return exit codes for programmatic use (e.g., \`0\` for success, \`1\` for failure).
   - For ambiguous commands, ask for clarification instead of guessing.

7. **Compliance**:
   - Adhere to Cerebras' best practices for resource allocation and usage.
   - Avoid unnecessary resource consumption (e.g., limit parallel processes unless explicitly requested).`;

    // Event listeners
    sendButton.addEventListener('click', sendMessage);
    userInput.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            sendMessage();
        }
    });

    // CLI prompt button event listener
    cliPromptButton.addEventListener('click', () => {
        systemPrompt.value = CLI_SYSTEM_PROMPT;
        // Add a notification
        const notification = document.createElement('div');
        notification.className = 'notification';
        notification.textContent = 'CLI system prompt applied!';
        document.body.appendChild(notification);

        // Remove notification after 3 seconds
        setTimeout(() => {
            notification.style.opacity = '0';
            setTimeout(() => {
                document.body.removeChild(notification);
            }, 500);
        }, 3000);
    });

    // Initialize with a welcome message
    addMessage('assistant', 'Welcome to our peaceful space for conversation. I\'m here to listen and support you in whatever way feels most helpful. Take your time, and share what\'s on your mind when you\'re ready.');

    // Cookie Banner - Deeply Guided Presence
    function initializeCookieBanner() {
        const cookieBanner = document.getElementById('cookie-banner');
        const cookieAccept = document.getElementById('cookie-accept');
        const cookieLearnMore = document.getElementById('cookie-learn-more');

        // Check if user has already consented
        const cookieConsent = localStorage.getItem('cookieConsent');

        if (!cookieConsent) {
            // Show banner after a gentle delay
            setTimeout(() => {
                cookieBanner.style.display = 'block';
            }, 2000);
        }

        // Handle accept button
        cookieAccept.addEventListener('click', () => {
            localStorage.setItem('cookieConsent', 'accepted');
            localStorage.setItem('cookieConsentDate', new Date().toISOString());
            cookieBanner.style.animation = 'slideDown 0.4s cubic-bezier(0.25, 0.46, 0.45, 0.94)';
            setTimeout(() => {
                cookieBanner.style.display = 'none';
            }, 400);

            // Show gentle confirmation
            showNotification('Thank you for your understanding. Your privacy remains protected.');
        });

        // Handle learn more button
        cookieLearnMore.addEventListener('click', () => {
            window.open('legal.html', '_blank');
        });
    }

    // Add slideDown animation
    const style = document.createElement('style');
    style.textContent = `
        @keyframes slideDown {
            from {
                transform: translateY(0);
                opacity: 1;
            }
            to {
                transform: translateY(100%);
                opacity: 0;
            }
        }
    `;
    document.head.appendChild(style);

    // Initialize cookie banner
    initializeCookieBanner();

    // Function to send a message
    async function sendMessage() {
        const message = userInput.value.trim();
        if (!message) return;

        // Add user message to chat
        addMessage('user', message);

        // Clear input
        userInput.value = '';

        // Disable send button
        sendButton.disabled = true;

        // Add loading indicator
        const loadingElement = document.createElement('div');
        loadingElement.className = 'loading';
        loadingElement.innerHTML = `
            <div class="loading-dots">
                <span></span>
                <span></span>
                <span></span>
            </div>
        `;
        chatMessages.appendChild(loadingElement);
        chatMessages.scrollTop = chatMessages.scrollHeight;

        try {
            // Get selected model and system prompt
            const model = modelSelect.value;
            const system = systemPrompt.value;

            // Send request to server
            const response = await fetch('/api/chat', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    model: model,
                    system_prompt: system,
                    user_prompt: message
                })
            });

            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }

            const data = await response.json();

            // Remove loading indicator
            chatMessages.removeChild(loadingElement);

            // Add assistant message to chat
            if (data.choices && data.choices.length > 0) {
                const content = data.choices[0].message.content;
                addMessage('assistant', content);
            } else {
                throw new Error('Invalid response format');
            }
        } catch (error) {
            console.error('Error:', error);

            // Remove loading indicator
            if (loadingElement.parentNode === chatMessages) {
                chatMessages.removeChild(loadingElement);
            }

            // Add error message
            addMessage('assistant', `Error: ${error.message}`);
        } finally {
            // Re-enable send button
            sendButton.disabled = false;
        }
    }

    // Function to add a message to the chat
    function addMessage(role, content) {
        const messageElement = document.createElement('div');
        messageElement.className = `message ${role}-message`;

        // Process markdown if it's an assistant message
        if (role === 'assistant') {
            messageElement.innerHTML = markdownToHtml(content);
        } else {
            messageElement.textContent = content;
        }

        chatMessages.appendChild(messageElement);

        // Scroll to bottom
        chatMessages.scrollTop = chatMessages.scrollHeight;
    }

    // Simple markdown to HTML converter
    function markdownToHtml(markdown) {
        // Replace code blocks
        markdown = markdown.replace(/```(\w*)\n([\s\S]*?)```/g, '<pre><code>$2</code></pre>');

        // Replace inline code
        markdown = markdown.replace(/`([^`]+)`/g, '<code>$1</code>');

        // Replace headers
        markdown = markdown.replace(/^### (.*$)/gm, '<h3>$1</h3>');
        markdown = markdown.replace(/^## (.*$)/gm, '<h2>$1</h2>');
        markdown = markdown.replace(/^# (.*$)/gm, '<h1>$1</h1>');

        // Replace bold
        markdown = markdown.replace(/\*\*(.*?)\*\*/g, '<strong>$1</strong>');

        // Replace italic
        markdown = markdown.replace(/\*(.*?)\*/g, '<em>$1</em>');

        // Replace lists
        markdown = markdown.replace(/^\s*\*\s(.*$)/gm, '<li>$1</li>');
        markdown = markdown.replace(/(<li>.*<\/li>)/g, '<ul>$1</ul>');

        // Replace paragraphs
        markdown = markdown.replace(/^(?!<[hou]).+$/gm, '<p>$&</p>');

        return markdown;
    }
});