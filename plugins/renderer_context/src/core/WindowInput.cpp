#include "core/WindowInput.hpp"
#include "GLFW/glfw3.h"
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <mutex>
#include <vector>
#include "concurrentqueue.hpp"

struct KeyActions {
    KeyActions(int s, int a, int m) : ScanCode(std::move(s)), Actions(std::move(a)), Modifiers(std::move(m)) {}
    KeyActions() : ScanCode(-1), Actions(-1), Modifiers(-1) {}
    KeyActions(KeyActions&& other) noexcept : ScanCode(std::move(other.ScanCode)),
        Actions(std::move(other.Actions)), Modifiers(std::move(other.Modifiers)) {}
    KeyActions& operator=(KeyActions&& other) noexcept {
        ScanCode = std::move(other.ScanCode);
        Actions = std::move(other.Actions);
        Modifiers = std::move(other.Modifiers);
        return *this;
    }
    KeyActions(const KeyActions& other) noexcept : ScanCode(other.ScanCode),
        Actions(other.Actions), Modifiers(other.Modifiers) {}
    KeyActions& operator=(const KeyActions& other) noexcept {
        ScanCode = other.ScanCode;
        Actions = other.Actions;
        Modifiers = other.Modifiers;
        return *this;
    }
    int ScanCode;
    int Actions;
    int Modifiers;
};

struct WindowInputImpl {
    WindowInputImpl(GLFWwindow* window);
    GLFWwindow* window;
    std::unordered_set<int> monitoredScancodes;
    std::unordered_map<int, KeyActions> scancodeActions;
    std::unordered_map<int, KeyActions> mouseActions;
    std::mutex guardMutex;
};

static moodycamel::ConcurrentQueue<KeyActions> actionQueue;
static moodycamel::ConcurrentQueue<KeyActions> mouseButtonQueue;
static std::mutex charMutex;
static std::vector<unsigned int> inputChars;

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    actionQueue.enqueue(std::move(KeyActions{ std::move(scancode), std::move(action), std::move(mods) }));
}

static void CharCallback(GLFWwindow* window, unsigned int c) {
    std::lock_guard<std::mutex> charGuard(charMutex);
    inputChars.emplace_back(c);
}

static void MouseButtonCallback(GLFWwindow* window, int button, int actions, int mods) {
    mouseButtonQueue.enqueue(std::move(KeyActions{ std::move(button), std::move(actions), std::move(mods) }));
}

WindowInput::WindowInput(GLFWwindow* window) : impl(new WindowInputImpl(window)) {
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCharCallback(window, CharCallback);
}

WindowInput::~WindowInput() {
    if (impl) {
        delete impl;
    }
}

void WindowInput::Update() {
    glfwPollEvents();
    KeyActions action;
    while (actionQueue.try_dequeue(action)) {
        if (impl->monitoredScancodes.count(action.ScanCode) != 0) {
            impl->scancodeActions[action.ScanCode] = action;
        }
    }
    while (mouseButtonQueue.try_dequeue(action)) {
        impl->mouseActions[action.ScanCode] = action;
    }
}

void WindowInput::AddMonitoredScancode(int code) {
    std::lock_guard<std::mutex> lock(impl->guardMutex);
    impl->monitoredScancodes.emplace(code);
}

void WindowInput::RemoveMonitoredScancode(int code) {
    std::lock_guard<std::mutex> lock(impl->guardMutex);
    impl->monitoredScancodes.erase(code);
}

void WindowInput::GetScancodeActions(int code, int* actions, int* modifiers) const {
    auto iter = std::find_if(std::cbegin(impl->scancodeActions), std::cend(impl->scancodeActions),
        [code](const decltype(impl->scancodeActions)::value_type& item){ return item.second.ScanCode == code; });
    
    if (iter != std::end(impl->scancodeActions)) {
        if (actions) {
            *actions = iter->second.Actions; 
        }
        if (modifiers) {
            *modifiers = iter->second.Modifiers;
        }
    }
    else {
        if (actions) {
            *actions = -1;
        }
        if (modifiers) {
            *modifiers = -1;
        }
    }
}

void WindowInput::ConsumeScancodeActions(int code, int* actions, int* modifiers) {
    auto iter = std::find_if(std::cbegin(impl->scancodeActions), std::cend(impl->scancodeActions),
        [code](const decltype(impl->scancodeActions)::value_type& item){ return item.second.ScanCode == code; });
    
    if (iter != std::end(impl->scancodeActions)) {
        if (actions) {
            *actions = iter->second.Actions; 
        }
        if (modifiers) {
            *modifiers = iter->second.Modifiers;
        }
        impl->scancodeActions.erase(iter);
    }
    else {
        if (actions) {
            *actions = -1;
        }
        if (modifiers) {
            *modifiers = -1;
        }
    }
}


void WindowInput::GetInputCharacters(size_t* num_chars, unsigned int* dest_str) {
    *num_chars = inputChars.size();
    if (dest_str != nullptr) {
#ifdef _MSC_VER
        memcpy_s(dest_str, *num_chars * sizeof(unsigned int), inputChars.data(), sizeof(unsigned int) * *num_chars);
#else
        memcpy(dest_str, inputChars.data(), sizeof(unsigned int) * (*num_chars));
#endif
        std::lock_guard<std::mutex> inputCharsGuard(charMutex);
        inputChars.erase(std::begin(inputChars), std::begin(inputChars) + *num_chars);
        inputChars.shrink_to_fit();
    }
}

void WindowInput::GetMouseActions(int button, int * actions, int * modifiers) const {
    auto iter = std::find_if(std::cbegin(impl->mouseActions), std::cend(impl->mouseActions),
        [button](const decltype(impl->mouseActions)::value_type& value) { return value.second.ScanCode == button; });

    if (iter != std::cend(impl->mouseActions)) {
        if (actions) {
            *actions = iter->second.Actions;
        }
        if (modifiers) {
            *modifiers = iter->second.Modifiers;
        }
    }
    else {
        if (actions) {
            *actions = -1;
        }
        if (modifiers) {
            *modifiers = -1;
        }
    }
}

void WindowInput::ConsumeMouseAction(int button, int * actions, int * modifiers) {
    auto iter = std::find_if(std::cbegin(impl->mouseActions), std::cend(impl->mouseActions),
        [button](const decltype(impl->mouseActions)::value_type& value) { return value.second.ScanCode == button; });

    if (iter != std::cend(impl->mouseActions)) {
        if (actions) {
            *actions = iter->second.Actions;
        }
        if (modifiers) {
            *modifiers = iter->second.Modifiers;
        }
        impl->mouseActions.erase(iter);
    }
    else {
        if (actions) {
            *actions = -1;
        }
        if (modifiers) {
            *modifiers = -1;
        }
    }
}

const char * WindowInput::GetClipboardText() {
    return nullptr;
}

void WindowInput::FlushScancodeActions() {
    impl->scancodeActions.clear();
}

WindowInputImpl::WindowInputImpl(GLFWwindow * _window) : window(_window) {}
