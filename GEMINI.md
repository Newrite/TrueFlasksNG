# Project Context: TrueFlasksNG

## Project Overview
TrueFlasksNG is a Skyrim Special Edition (SKSE64) plugin. It implements a new potion/flask system using a modern C++ architecture and web-based UI technologies.

## Technology Stack
- **Language:** C++ (C++20 Standard).
- **Module System:** C++ Modules are used primarily.
- **Game Engine:** Skyrim Special Edition (SSE).
- **Dependencies:** - **CommonLibSSE-NG:** A reverse engineered library for Skyrim SE and VR.
    - SKSE64.
- **UI Frameworks:**
    - **In-Game HUD:** PrismaUI (HTML/CSS/JS).
    - **Configuration Menu:** ImGui via `SKSEMenuFramework`.

---

## C++ Coding Standards & Conventions

### 1. Modules vs. Headers
- **Default:** Use C++ Modules (`export module`, `import`) for all new core logic.
- **Exceptions (Legacy/Interop):** The following files/folders must remain standard C++ headers/source files:
    - `pch.h`
    - `src/API/TrueFlasksAPI.h` & `src/API/TrueFlasksAPI.cpp`
    - `src/library/*` (Third-party wrappers).

### 2. Precompiled Headers (PCH)
- **Global Includes:** The `pch.h` file already includes:
    - All standard STL libraries (`std::vector`, `std::string`, etc.).
    - The entire Reverse Engineering namespace (`RE::`).
    - `CommonLibSSE-NG` headers.
- **Rule:** **DO NOT** manually `#include` standard libraries or `RE::` headers in generated code. Assume they are globally available.

### 3. Safety & pointers (RE:: Namespace)
- **Strict Null-Checking:** All pointers obtained from the `RE::` namespace (reverse engineered game data) must be treated as volatile.
- **Rule:** Always check for `nullptr` before accessing members of any `RE::` class (e.g., `RE::PlayerCharacter`, `RE::Actor`). The game state is unpredictable.

### 4. Syntax & Style
- **Variables:** Prefer `auto` and `const auto` for type deduction.
- **Const Correctness:** Apply `const` aggressively to variables and methods.
- **INI Files:** Use `mINI` library (`src/library/ini.h`) for parsing configuration.
- **External APIs:** To obtain an API instance of another SKSE plugin, refer to `src/Core/ModsAPIRepository.cpp`.

---

## UI Development (PrismaUI)

### 1. General Info
- **Location:** `src/UI/views/TrueFlasksNG` (HTML/CSS/JS resources).
- **Engine:** WebKit (CPU rendering only, no WebGL).
- **Refresh Rate:** Capped at 60 FPS.

### 2. Communication Methods (C++ to JS)

There are two distinct methods for calling JavaScript from C++. Choose based on performance requirements.

#### A. InteropCall (High Performance)
Use this for **frequent updates** (e.g., game loops, real-time health/mana updates).

* **Characteristics:** Very fast, NO return value, accepts only **one string argument**.
* **Requirements:** The JS function must be globally accessible (attached to `window`).
* **Syntax:**
    ```cpp
    PrismaUI->InteropCall(view, "functionName", "argumentString");
    ```
* **Example:**
    ```cpp
    // C++
    PrismaUI->InteropCall(view, "updateHealth", "100");
    
    // JS
    window.updateHealth = (val) => { 
        // val is received as a string!
        console.log(Number(val)); 
    };
    ```

#### B. Invoke (Flexibility & Complex Data)
Use this for **one-time calls**, initialization, complex JSON data transfer, or when you need a **return value**.

* **Characteristics:** Slower than InteropCall, executes arbitrary JS code, supports callbacks.
* **Syntax:**
    ```cpp
    PrismaUI->Invoke(view, "jsCode", callbackOrNull);
    ```
* **JSON Data:** Use `nlohmann/json` to serialize complex data structures before sending.
    ```cpp
    #include <nlohmann/json.hpp>
    using JSON = nlohmann::json;

    void SendData() {
        JSON data = { {"health", 100}, {"magicka", 50} };
        std::string script = "initStats(" + data.dump() + ")"; 
        PrismaUI->Invoke(view, script.c_str());
    }
    ```
* **String Escaping:** If not using the JSON library, you **must** manually escape strings (quotes, backslashes) before passing them to `Invoke` to prevent syntax errors.

#### Summary: When to use what?
| Feature | InteropCall | Invoke |
| :--- | :--- | :--- |
| **Use Case** | Real-time data (HP, Stamina) | Initialization, Settings, Complex Logic |
| **Performance** | Highest | Standard |
| **Arguments** | 1 String only | Arbitrary Script / JSON |
| **Return Value** | No | Yes (via Callback) |

### 3. PrismaUI Limitations & Workarounds
- **Audio/Video:** Not supported. Use GIFs for animation. Play sounds via SKSE C++.
- **JS Version:** ES2022 and below.
- **Context Menu:** The `contextmenu` event is broken.
- **Right-Click Fix:** Use the following JS pattern for right-clicks:
    ```javascript
    window.addEventListener('mousedown', function (event) {
      if (event.button === 2) {
        const contextMenuEvent = new MouseEvent('contextmenu', {
          ...event,
          view: window,
          bubbles: true,
          cancelable: true,
          screenX: event.pageX,
          screenY: event.pageY,
          clientX: event.pageX,
          clientY: event.pageY,
        });
        event.target?.dispatchEvent(contextMenuEvent);
      }
    });
    ```
