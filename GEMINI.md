# Project Context: TrueFlasksNG

## 1. Project Overview
TrueFlasksNG is a Skyrim Special Edition (SKSE64) plugin. It implements a new potion/flask system using a modern C++ architecture and web-based UI technologies.

## 2. Technology Stack
- **Language:** C++ (C++20 Standard).
- **Build System:** xmake.
- **Module System:** C++ Modules are used primarily.
- **Game Engine:** Skyrim Special Edition (SSE).
- **Core Dependencies:**
    - **CommonLibSSE-NG:** A reverse engineered library for Skyrim SE and VR.
    - **SKSE64.**
    - **Glaze:** High-performance C++20 JSON library (`glaze/glaze.hpp`).
    - **mINI:** INI file parsing (`src/library/ini.h`).
- **UI Frameworks:**
    - **In-Game HUD:** PrismaUI (HTML/CSS/JS).
    - **Configuration Menu:** ImGui via `SKSEMenuFramework`.

---

## 3. C++ Coding Standards & Conventions

### Modules vs. Headers
- **Default:** Use C++ Modules (`export module`, `import`) for all new core logic.
- **Exceptions (Legacy/Interop):** The following files/folders must remain standard C++ headers/source files:
    - `pch.h`
    - `src/API/TrueFlasksAPI.h` & `src/API/TrueFlasksAPI.cpp`
    - `src/library/*` (Third-party wrappers).

### Precompiled Headers (PCH)
- **Global Includes:** The `pch.h` file already includes:
    - All standard STL libraries (`std::vector`, `std::string`, `std::format`, etc.).
    - The entire Reverse Engineering namespace (`RE::`).
    - `CommonLibSSE-NG` headers.
    - `glaze/glaze.hpp`.
- **Rule:** **DO NOT** manually `#include` standard libraries, `RE::` headers, or `glaze` in generated code. Assume they are globally available via PCH.

### Safety & Pointers (RE:: Namespace)
- **Strict Null-Checking:** All pointers obtained from the `RE::` namespace (game data) must be treated as volatile.
- **Rule:** Always check for `nullptr` before accessing members of any `RE::` class (e.g., `RE::PlayerCharacter`, `RE::Actor`). The game state is unpredictable.

### Syntax & Style
- **Variables:** Prefer `auto` and `const auto` for type deduction.
- **Const Correctness:** Apply `const` aggressively to variables and methods.
- **External APIs:** To obtain an API instance of another SKSE plugin, refer to patterns in `src/Core/ModsAPIRepository.cpp`.

---

## 4. UI Development (PrismaUI)

### General Info
- **Location:** `src/UI/views/TrueFlasksNG` (HTML/CSS/JS resources).
- **Engine:** WebKit (CPU rendering only, no WebGL).
- **Refresh Rate:** Capped at 60 FPS.
- **JS Version:** ES2022 and below.

### Communication Methods (C++ to JS)

Choose the method based on performance requirements.

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
    ```

#### B. Invoke (Flexibility & Complex Data)
Use this for **one-time calls**, initialization, complex JSON data transfer, or when you need a **return value**.

* **Characteristics:** Slower than InteropCall, executes arbitrary JS code, supports callbacks.
* **Syntax:**
    ```cpp
    PrismaUI->Invoke(view, "jsCode", callbackOrNull);
    ```
* **JSON Data (Glaze Library):**
  We use **Glaze** for high-performance C++20 JSON serialization. Define a struct and use `glz::write_json`.

    ```cpp
    // 1. Define a struct (must have public members for reflection)
    struct StatData {
        int health;
        int magicka;
    };

    void SendData(PrismaView view) {
        // 2. Create and fill data
        StatData data { 100, 50 };
        
        // 3. Serialize using Glaze (Fast & Type-safe)
        std::string jsonArgs = glz::write_json(data); // Returns: {"health":100,"magicka":50}
        
        // 4. Construct JS call
        // Resulting script: initStats({"health":100,"magicka":50})
        std::string script = std::format("initStats({})", jsonArgs); 
        
        PrismaUI->Invoke(view, script.c_str());
    }
    ```

#### Summary: When to use what?
| Feature | InteropCall | Invoke |
| :--- | :--- | :--- |
| **Use Case** | Real-time data (HP, Stamina) | Initialization, Settings, Complex Logic |
| **Performance** | Highest | Standard |
| **Arguments** | 1 String only | Arbitrary Script / JSON |
| **Return Value** | No | Yes (via Callback) |

### PrismaUI Limitations & Workarounds
- **Audio/Video:** Not supported. Use GIFs for animation. Play sounds via SKSE C++.
- **Context Menu:** The native `contextmenu` event is broken.
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
