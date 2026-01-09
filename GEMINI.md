# Project Context: TrueFlasksNG

## Project Overview
TrueFlasksNG is a Skyrim Special Edition (SKSE64) plugin. It implements a new potion/flask system using a modern C++ architecture and web-based UI technologies.

## Technology Stack
- **Language:** C++ (C++20 Standard).
- **Module System:** C++ Modules are used primarily.
- **Game Engine:** Skyrim Special Edition (SSE).
- **Dependencies:** SKSE64, CommonLibSSE-NG.
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
    - CommonLibSSE headers.
- **Rule:** **DO NOT** manually `#include` standard libraries or `RE::` headers in generated code. Assume they are globally available.

### 3. Syntax & Style
- **Variables:** Prefer `auto` and `const auto` for type deduction.
- **Const Correctness:** Apply `const` aggressively to variables and methods wherever modification is not required.
- **INI Files:** Use `mINI` library (located in `src/library/ini.h`) for parsing configuration.
- **External APIs:** To obtain an API instance of another SKSE plugin, refer to the implementation patterns in `src/Core/ModsAPIRepository.cpp`.

---

## UI Development Guidelines

### 1. Configuration UI (ImGui)
- Located in `src/UI` (C++ side) wrapping `SKSEMenuFramework`.
- Used strictly for the plugin configuration menu.

### 2. In-Game HUD (PrismaUI)
- **Location:** `src/UI/views/TrueFlasksNG` (HTML/CSS/JS resources).
- **Engine:** WebKit based.
- **Rendering:** CPU only (No WebGL).
- **Refresh Rate:** Capped at 60 FPS.

#### PrismaUI Limitations
- **JavaScript Version:** ES2022 and below.
- **Media:** NO Video (use GIF), NO Audio (trigger sounds via SKSE C++).
- **Event Handling:** The `contextmenu` event is broken.

#### PrismaUI Workarounds
**Right-Click Implementation:**
Use the following logic to handle right-clicks instead of the standard contextmenu event:
```javascript
window.addEventListener('mousedown', function (event) {
  if (event.button === 2) {
    // right-click logic
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
