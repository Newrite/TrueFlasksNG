# TrueFlasksNG

**TrueFlasksNG** is a modern SKSE64 plugin for Skyrim Special Edition/Anniversary Edition/VR that overhauls the potion system. It introduces a flask mechanic inspired by modern action RPGs (like *Cyberpunk 2077* or *Souls-likes*), where healing items use regenerating charges instead of being consumable items that disappear from your inventory.

[Current download link (Google Drive)](https://drive.google.com/drive/u/0/folders/1xIUqI47D59vgLwzfco1dpwEENh3YKJM5) *(Nexus release coming soon)*

### Requirements
* [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
* [Prisma UI](https://www.prismaui.dev/getting-started/installation/) *(Optional: Required only if you want the visual widget)*
* [SKSE Menu Framework](https://www.nexusmods.com/skyrimspecialedition/mods/120352) *(Optional: Required only for in-game configuration)*

> *Note: This plugin was created with the assistance of Gemini 3.*

---

## Motivation & Concept

**The Problem:**
In vanilla Skyrim, resource management is often chaotic and cluttered. You likely have an inventory filled with multiple tiers of the same potion (Minor, Light, Plentiful, etc.) plus various modded potions. Since these are physically distinct objects, hotkey management becomes tedious—once you run out of "Minor Potions", your hotkey stops working until you rebind it to the next tier.

**The Solution:**
TrueFlasksNG solves this by unifying resource restoration into a **Flask System**:
* **Infinite Resource:** You are given special Flasks (Health, Stamina, Magicka) that are never consumed upon use.
* **Charge System:** Instead of losing the item, using a flask consumes a "Charge".
* **Auto-Regeneration:** Charges regenerate automatically over time (with configurable cooldowns).
* **Strategic Gameplay:** This shifts the focus from hoarding hundreds of bottles to managing your cooldowns and charges tactically during combat.

---

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

#### UI Settings for PrismaWidget
*   **Enable**: Toggle the HUD widget.
*   **Auto Hide**: Automatically hides the widget when all flasks are full.
*   **Position & Scale**: Customize the position, size, and opacity of the entire widget or individual flask elements.
*   **Anchor All**: Lock individual elements to the global widget position or move them freely.

#### Prisma Widget required [Prisma Framework](https://www.prismaui.dev/getting-started/installation/) (prisma widget is optional)

---

## Key Features

### 1. Cyberpunk-style Flasks
Dedicated Flasks for **Health**, **Stamina**, and **Magicka**. When used, they consume a charge but the item remains in your inventory. Charges regenerate based on your stats or configuration.

### 2. "Other" Potions Management
The mod isn't limited to just the main flasks. It introduces a shared slot mechanic for all other potions (Resistances, Buffs, Utility):
* **Shared Slots:** By default, these potions require a free **"Other Slot"** to be used. If all slots are on cooldown, you cannot drink potions.
* **Consumption:** Unlike flasks, these potions **are still consumed** upon use (vanilla behavior).
* **Infinite Keyword:** You can assign a specific keyword to **any** potion to make it "Infinite" (non-consumable) like the main flasks.
* **Exclusive & Revert Logic:** You can fine-tune which potions use slots via the `ExclusiveKeyword` in the config:
    * *Standard Mode:* If a potion has the **Exclusive Keyword**, it **does not** use a slot (bypasses the system).
    * *Revert Mode:* If `FlasksRevertExclusive` is enabled, the logic flips: **only** potions with the Exclusive Keyword will use a slot, while all others bypass the system.

### 3. Anti-Spam Protection
Designed for players who do not use potion drinking animation mods. This feature prevents accidental double-clicks or macro-spamming, ensuring you don't waste charges instantly by mistake.

### 4. Highly Modular & Customizable
Everything is adjustable via the INI file or In-Game Menu:
* **Keyword-based Definition:** The mod identifies which items are Flasks (Health/Stamina/Magicka) based on Keywords. You define these Keyword IDs in the configuration, making it compatible with any item from any mod.
* **Toggle Modules:** You can disable the entire flask system for the Player while keeping it for NPCs, or vice versa.
* **Selective Logic:** You can disable the inventory management part and use the mod purely for its cooldown/slot mechanics.
* **Balance:** Adjust Max Charges (Slots), Cooldown Duration, and Regeneration Speed.

### 5. Framework Approach
At its core, TrueFlasksNG is a DLL framework. While it comes with a "Ready-to-use" ESP and pre-configured flasks, it does not hardcode game logic. Mod authors can use the provided API to define their own logic, create custom flasks, or override default behaviors without relying on the included ESP.

#### Gameplay Transformations
The modular nature of TrueFlasksNG allows you to radically change the game's pacing by combining `.ini` tweaks with simple scripts:

* **The "Souls-like" Experience:**
    * *Config:* Set `FlasksRegenerationMultBase = 0.0` to disable auto-regeneration.
    * *Logic:* Create interaction points (Bonfires/Shrines) that call `ModifyCooldown` to fully restore charges when triggered. You can also distribute rare items throughout the world that add Active Effects with the `FlasksCapKeyword`, permanently increasing your flask charges as you progress.

* **The "Blood-Fueled" (Aggressive Combat):**
    * *Config:* Set regeneration to very slow or zero.
    * *Logic:* Encourage aggression by using a Papyrus script to monitor enemy kills. On a kill, call `ModifyCooldown(Player, Type, -10.0, false)` to instantly reduce the cooldown, rewarding the player with health/resources for staying in the fight.

* **The "Alchemist" (Preparation-based):**
    * *Config:* Set regeneration to 0.
    * *Logic:* Instead of finding potions, the player must "brew" refills at a crafting station. A crafting result could trigger a script that restores charges, simulating preparing your supplies before a journey without cluttering the inventory with hundreds of individual bottles.

---

## For Developers (API)

TrueFlasksNG exports an interface that allows other SKSE plugins to interact with the flask system directly.

### How to Request the Interface

You can request the `TrueFlasksIVTrueFlasks1` interface via the SKSE messaging system.

```cpp
// In your SKSE plugin load or message handler:
void OnMessage(SKSE::MessagingInterface::Message* message) {
  if (message->type == SKSE::MessagingInterface::kDataLoaded) {
    
    auto true_flasks = static_cast<TrueFlasksAPI::IVTrueFlasks1*>(TrueFlasksAPI::RequestPluginAPI(TrueFlasksAPI::InterfaceVersion::V1));
    true_flasks ? logger::info("Success request TrueFlasks") : logger::info("Failed request TrueFlasks");
    
  }
}
```

### Interface Definition: IVTrueFlasks1

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

### Create Your Own UI
While the mod comes with a ready-to-use Prisma UI widget, you are not bound to it.
* **Full Data Access:** Both the C++ and Papyrus APIs provide real-time access to current charges, max slots, and cooldown progress.
* **Freedom:** You can build a completely custom widget (SWF, ImGui) that fits your specific modlist's aesthetic, or integrate the flask status into existing HUD mods seamlessly.

### Papyrus Scripts
The mod also provides global native functions for Papyrus scripts, allowing you to manipulate flasks from scripts (e.g., via MCM or quest scripts).

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
