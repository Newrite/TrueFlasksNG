# TrueFlasksNG

**TrueFlasksNG** is a modern SKSE64 plugin for Skyrim Special Edition/Anniversary Edition/VR that overhauls the potion system. It introduces a flask mechanic inspired by modern games (like Cyberpunk 2077), where potions have limited charges (slots) that regenerate over time, rather than being consumable items that disappear from your inventory.

---

## For Users

### Core Features

*   **Flask System**: Potions are categorized into four types: **Health**, **Stamina**, **Magicka**, and **Other**.
*   **Charges & Cooldowns**: Instead of consuming a potion, you consume a "charge". Charges regenerate over time.
*   **No Item Removal**: Configured potions are not removed from your inventory when used.
*   **Anti-Spam**: Prevents rapid-fire consumption of potions to encourage tactical gameplay or prevent use flask slot when spamming hotkey.
*   **Visual Feedback**: Includes a customizable HUD widget (PrismaUI) that displays your current flask charges and cooldown progress.
*   **In-Game Configuration**: Fully configurable via an in-game menu (requires `SKSEMenuFramework`).

### Configuration (TrueFlasksNG.ini & In-Game Menu)

You can customize the behavior of each flask type independently.

#### Global Settings
*   **NoRemoveKeyword**: Potions with this keyword will not be removed from the inventory when used (if this potion from enabled potion types (Health \ Magick \ Stamina \ Other and config section enabled)).

#### Per-Flask Settings (Health, Stamina, Magicka, Other)
*   **Enable**: Toggle the flask system for this specific type.
*   **Player/NPC**: Enable or disable for Player or NPCs specifically.
*   **Notification**: Custom message when you try to drink an empty flask.
*   **Parallel Cooldown**:
    *   `true`: All used slots regenerate simultaneously.
    *   `false`: Slots regenerate one by one (queue system).
*   **Anti-Spam**: Adds a small delay between drinking potions.
*   **Regeneration Speed**:
    *   **Base**: Base multiplier for regeneration speed.
    *   **Keyword**: Active effects with this keyword increase regeneration speed.
*   **Capacity (Slots)**:
    *   **Base**: Starting number of charges.
    *   **Keyword**: Active effects with this keyword add extra slots.
*   **Cooldown Duration**:
    *   **Base**: Time in seconds to regenerate one charge.
    *   **Keyword**: Active effects with this keyword increase cooldown duration.

#### To accumulate values from active effects (with relative keyword) for modifying Regeneration Speed, Capacity (Slots), or Cooldown Duration, the effect must have the Recovery flag enabled. Whether the value is increased or decreased is determined by the Detrimental flag.

#### UI Settings (PrismaWidget)
*   **Enable**: Toggle the HUD widget.
*   **Auto Hide**: Automatically hides the widget when all flasks are full.
*   **Position & Scale**: Customize the position, size, and opacity of the entire widget or individual flask elements.
*   **Anchor All**: Lock individual elements to the global widget position or move them freely.

---

## For Mod Authors

TrueFlasksNG provides both a native C++ API and a Papyrus API for integration with other mods.

### Papyrus API

Script name: `TrueFlasksNG`

| Function | Description |
| :--- | :--- |
| `ModifyCooldown(Actor actor, int type, float amount, bool all_slots)` | Reduces (or increases) the current cooldown of a flask type. `amount` is in seconds. |
| `float GetNextCooldown(Actor actor, int type)` | Returns the remaining cooldown time (in seconds) for the next available slot. |
| `int GetMaxSlots(Actor actor, int type)` | Returns the maximum number of charges for a flask type. |
| `int GetCurrentSlots(Actor actor, int type)` | Returns the number of currently available charges. |
| `float GetRegenMult(Actor actor, int type)` | Returns the current regeneration speed multiplier. |
| `float GetCooldownPct(Actor actor, int type)` | Returns the cooldown progress as a percentage (0.0 to 1.0). |
| `int[] GetFlaskInfo(AlchemyItem potion)` | Returns an array: `[FlaskType, ConsumesSlot]`. `FlaskType`: 0=Health, 1=Stamina, 2=Magick, 3=Other. |
| `PlayFlaskGlow(Actor actor, int type)` | Triggers the visual "glow" effect on the HUD for a specific flask type. |

**Flask Types (int):**
*   `0`: Health
*   `1`: Stamina
*   `2`: Magick
*   `3`: Other

### Native C++ API

To use the native API, include `TrueFlasksAPI.h` and link against `TrueFlasksNG.dll` at runtime.

#### Interface: `IVTrueFlasks1`

```cpp
enum class FlaskType : uint8_t {
    Health = 0,
    Stamina = 1,
    Magick = 2,
    Other = 3
};

struct FlaskSettings {
    bool enable;
    bool npc;
    bool player;
    // ... (see header for full struct)
};

class IVTrueFlasks1 {
public:
    virtual void ModifyCooldown(RE::Actor* actor, FlaskType type, float amount, bool all_slots) noexcept = 0;
    virtual float GetNextCooldown(RE::Actor* actor, FlaskType type) noexcept = 0;
    virtual int GetMaxSlots(RE::Actor* actor, FlaskType type) noexcept = 0;
    virtual int GetCurrentSlots(RE::Actor* actor, FlaskType type) noexcept = 0;
    virtual float GetRegenMult(RE::Actor* actor, FlaskType type) noexcept = 0;
    virtual float GetCooldownPct(RE::Actor* actor, FlaskType type) noexcept = 0;
    virtual std::pair<int, bool> GetFlaskInfo(RE::AlchemyItem* potion) noexcept = 0;
    virtual void PlayFlaskGlow(RE::Actor* actor, FlaskType type) noexcept = 0;
    virtual std::optional<FlaskSettings> GetFlaskSettings(FlaskType type) noexcept = 0;
};
```

#### How to Request API

```cpp
#include "TrueFlasksAPI.h"

// ... inside your plugin load function
auto trueFlasks = reinterpret_cast<TrueFlasksAPI::IVTrueFlasks1*>(
    TrueFlasksAPI::RequestPluginAPI(TrueFlasksAPI::InterfaceVersion::V1)
);

if (trueFlasks) {
    // API is available
    trueFlasks->ModifyCooldown(player, TrueFlasksAPI::FlaskType::Health, 10.0f, false);
}
```
