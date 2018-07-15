#pragma once
#ifndef RENDERER_CONTEXT_WINDOW_INPUT_HPP
#define RENDERER_CONTEXT_WINDOW_INPUT_HPP

struct GLFWwindow;
struct WindowInputImpl;

class WindowInput {
    WindowInput(const WindowInput&) = delete;
    WindowInput& operator=(const WindowInput&) = delete;
public:

    WindowInput(GLFWwindow* window);
    ~WindowInput();

    void Update();

    void AddMonitoredScancode(int code);
    void RemoveMonitoredScancode(int code);

    void GetScancodeActions(int code, int* actions, int* modifiers) const;
    void ConsumeScancodeActions(int code, int* actions, int* modifiers);
    // Clears all scancode actions
    void FlushScancodeActions();

    // First call sets size of destination string. Second calls sets destination string
    // and flushes stored buffer.
    void GetInputCharacters(size_t* num_chars, unsigned int* dest_str);

    void GetMouseActions(int button, int* actions, int* modifiers) const;
    void ConsumeMouseAction(int button, int* actions, int* modifiers);

    const char* GetClipboardText();

private:
    WindowInputImpl* impl = nullptr;
};

#endif //!RENDERER_CONTEXT_WINDOW_INPUT_HPP
