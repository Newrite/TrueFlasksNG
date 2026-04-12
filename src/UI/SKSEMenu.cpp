module;

#include "library/SKSEMenuFramework.h"
#include <array>
#include <string>

export module TrueFlasks.UI.SKSEMenu;

import TrueFlasks.Config;
import TrueFlasks.UI.Prisma;

namespace ui::skse_menu
{
  using namespace ImGuiMCP;

  struct key_option
  {
    std::uint32_t code;
    const char* label;
  };

  constexpr auto keyboard_keys = std::to_array<key_option>({
    key_option{0x00, "None"}, key_option{0x01, "Escape"}, key_option{0x02, "1"}, key_option{0x03, "2"},
    key_option{0x04, "3"}, key_option{0x05, "4"}, key_option{0x06, "5"}, key_option{0x07, "6"},
    key_option{0x08, "7"}, key_option{0x09, "8"}, key_option{0x0A, "9"}, key_option{0x0B, "0"},
    key_option{0x0C, "Minus"}, key_option{0x0D, "Equals"}, key_option{0x0E, "Backspace"}, key_option{0x0F, "Tab"},
    key_option{0x10, "Q"}, key_option{0x11, "W"}, key_option{0x12, "E"}, key_option{0x13, "R"},
    key_option{0x14, "T"}, key_option{0x15, "Y"}, key_option{0x16, "U"}, key_option{0x17, "I"},
    key_option{0x18, "O"}, key_option{0x19, "P"}, key_option{0x1A, "Left Bracket"},
    key_option{0x1B, "Right Bracket"}, key_option{0x1C, "Enter"}, key_option{0x1D, "Left Ctrl"},
    key_option{0x1E, "A"}, key_option{0x1F, "S"}, key_option{0x20, "D"}, key_option{0x21, "F"},
    key_option{0x22, "G"}, key_option{0x23, "H"}, key_option{0x24, "J"}, key_option{0x25, "K"},
    key_option{0x26, "L"}, key_option{0x27, "Semicolon"}, key_option{0x28, "Apostrophe"},
    key_option{0x29, "Tilde"}, key_option{0x2A, "Left Shift"}, key_option{0x2B, "Backslash"},
    key_option{0x2C, "Z"}, key_option{0x2D, "X"}, key_option{0x2E, "C"}, key_option{0x2F, "V"},
    key_option{0x30, "B"}, key_option{0x31, "N"}, key_option{0x32, "M"}, key_option{0x33, "Comma"},
    key_option{0x34, "Period"}, key_option{0x35, "Slash"}, key_option{0x36, "Right Shift"},
    key_option{0x37, "Numpad *"}, key_option{0x38, "Left Alt"}, key_option{0x39, "Space"},
    key_option{0x3A, "Caps Lock"}, key_option{0x3B, "F1"}, key_option{0x3C, "F2"}, key_option{0x3D, "F3"},
    key_option{0x3E, "F4"}, key_option{0x3F, "F5"}, key_option{0x40, "F6"}, key_option{0x41, "F7"},
    key_option{0x42, "F8"}, key_option{0x43, "F9"}, key_option{0x44, "F10"}, key_option{0x45, "Num Lock"},
    key_option{0x46, "Scroll Lock"}, key_option{0x47, "Numpad 7"}, key_option{0x48, "Numpad 8"},
    key_option{0x49, "Numpad 9"}, key_option{0x4A, "Numpad -"}, key_option{0x4B, "Numpad 4"},
    key_option{0x4C, "Numpad 5"}, key_option{0x4D, "Numpad 6"}, key_option{0x4E, "Numpad +"},
    key_option{0x4F, "Numpad 1"}, key_option{0x50, "Numpad 2"}, key_option{0x51, "Numpad 3"},
    key_option{0x52, "Numpad 0"}, key_option{0x53, "Numpad ."}, key_option{0x57, "F11"}, key_option{0x58, "F12"},
    key_option{0x9C, "Numpad Enter"}, key_option{0x9D, "Right Ctrl"}, key_option{0xB5, "Numpad /"},
    key_option{0xB7, "Print Screen"}, key_option{0xB8, "Right Alt"}, key_option{0xC5, "Pause"},
    key_option{0xC7, "Home"}, key_option{0xC8, "Up"}, key_option{0xC9, "Page Up"}, key_option{0xCB, "Left"},
    key_option{0xCD, "Right"}, key_option{0xCF, "End"}, key_option{0xD0, "Down"}, key_option{0xD1, "Page Down"},
    key_option{0xD2, "Insert"}, key_option{0xD3, "Delete"}
  });

  const auto gamepad_keys = [] {
    return std::to_array<key_option>({
      key_option{0, "None"},
      key_option{SKSE::InputMap::GamepadMaskToKeycode(RE::BSWin32GamepadDevice::Keys::kUp), "DPad Up"},
      key_option{SKSE::InputMap::GamepadMaskToKeycode(RE::BSWin32GamepadDevice::Keys::kDown), "DPad Down"},
      key_option{SKSE::InputMap::GamepadMaskToKeycode(RE::BSWin32GamepadDevice::Keys::kLeft), "DPad Left"},
      key_option{SKSE::InputMap::GamepadMaskToKeycode(RE::BSWin32GamepadDevice::Keys::kRight), "DPad Right"},
      key_option{SKSE::InputMap::GamepadMaskToKeycode(RE::BSWin32GamepadDevice::Keys::kStart), "Start"},
      key_option{SKSE::InputMap::GamepadMaskToKeycode(RE::BSWin32GamepadDevice::Keys::kBack), "Back"},
      key_option{SKSE::InputMap::GamepadMaskToKeycode(RE::BSWin32GamepadDevice::Keys::kLeftThumb), "Left Stick"},
      key_option{SKSE::InputMap::GamepadMaskToKeycode(RE::BSWin32GamepadDevice::Keys::kRightThumb), "Right Stick"},
      key_option{SKSE::InputMap::GamepadMaskToKeycode(RE::BSWin32GamepadDevice::Keys::kLeftShoulder), "LB"},
      key_option{SKSE::InputMap::GamepadMaskToKeycode(RE::BSWin32GamepadDevice::Keys::kRightShoulder), "RB"},
      key_option{SKSE::InputMap::GamepadMaskToKeycode(RE::BSWin32GamepadDevice::Keys::kA), "A"},
      key_option{SKSE::InputMap::GamepadMaskToKeycode(RE::BSWin32GamepadDevice::Keys::kB), "B"},
      key_option{SKSE::InputMap::GamepadMaskToKeycode(RE::BSWin32GamepadDevice::Keys::kX), "X"},
      key_option{SKSE::InputMap::GamepadMaskToKeycode(RE::BSWin32GamepadDevice::Keys::kY), "Y"}
    });
  }();

  const char* get_key_label(const std::uint32_t key)
  {
    for (const auto& option : keyboard_keys) {
      if (option.code == key) {
        return option.label;
      }
    }
    return "Unknown";
  }

  bool render_keybind_combo(const char* label, std::uint32_t& key)
  {
    bool changed = false;
    if (ImGui::BeginCombo(label, get_key_label(key))) {
      for (const auto& option : keyboard_keys) {
        const bool selected = option.code == key;
        if (ImGui::Selectable(option.label, selected)) {
          key = option.code;
          changed = true;
        }
        if (selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    return changed;
  }

  const char* get_gamepad_key_label(const std::uint32_t key)
  {
    for (const auto& option : gamepad_keys) {
      if (option.code == key) {
        return option.label;
      }
    }
    return "Unknown";
  }

  bool render_gamepad_keybind_combo(const char* label, std::uint32_t& key)
  {
    bool changed = false;
    if (ImGui::BeginCombo(label, get_gamepad_key_label(key))) {
      for (const auto& option : gamepad_keys) {
        const bool selected = option.code == key;
        if (ImGui::Selectable(option.label, selected)) {
          key = option.code;
          changed = true;
        }
        if (selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    return changed;
  }

  const char* get_inventory_mode_label(const config::inventory_mode mode)
  {
    switch (mode) {
    case config::inventory_mode::disabled: return "Disabled";
    case config::inventory_mode::use: return "Use";
    case config::inventory_mode::deposit: return "Deposit";
    }
    return "Unknown";
  }

  bool render_inventory_mode_combo(const char* label, config::inventory_mode& mode)
  {
    bool changed = false;
    if (ImGui::BeginCombo(label, get_inventory_mode_label(mode))) {
      for (const auto option : {config::inventory_mode::disabled, config::inventory_mode::use,
                                config::inventory_mode::deposit}) {
        const bool selected = option == mode;
        if (ImGui::Selectable(get_inventory_mode_label(option), selected)) {
          mode = option;
          changed = true;
        }
        if (selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    return changed;
  }

  bool render_inventory_mode_combo_no_use(const char* label, config::inventory_mode& mode)
  {
    if (mode == config::inventory_mode::use) {
      mode = config::inventory_mode::disabled;
    }

    bool changed = false;
    if (ImGui::BeginCombo(label, get_inventory_mode_label(mode))) {
      for (const auto option : {config::inventory_mode::disabled, config::inventory_mode::deposit}) {
        const bool selected = option == mode;
        if (ImGui::Selectable(get_inventory_mode_label(option), selected)) {
          mode = option;
          changed = true;
        }
        if (selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    return changed;
  }

  const char* get_inventory_select_mode_label(const config::inventory_select_mode mode)
  {
    switch (mode) {
    case config::inventory_select_mode::weakest_first: return "Weakest First";
    case config::inventory_select_mode::strongest_first: return "Strongest First";
    case config::inventory_select_mode::first_found: return "First Found";
    }
    return "Unknown";
  }

  bool render_inventory_select_mode_combo(const char* label, config::inventory_select_mode& mode)
  {
    bool changed = false;
    if (ImGui::BeginCombo(label, get_inventory_select_mode_label(mode))) {
      for (const auto option : {config::inventory_select_mode::weakest_first,
                                config::inventory_select_mode::strongest_first,
                                config::inventory_select_mode::first_found}) {
        const bool selected = option == mode;
        if (ImGui::Selectable(get_inventory_select_mode_label(option), selected)) {
          mode = option;
          changed = true;
        }
        if (selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    return changed;
  }

  void RenderTooltip(const char* desc)
  {
    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
      ImGui::TextUnformatted(desc);
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
    }
  }

  void render_inventory_settings(config::flask_settings& settings, bool& changed)
  {
    if (render_inventory_mode_combo("Inventory Mode", settings.inventory_mode_value)) changed = true;
    RenderTooltip("Disabled: ignore inventory support. Use: drinking the flask consumes one matching inventory potion and uses its effect; the flask charge count equals the number of matching potions. Deposit: auto-convert matching inventory potions into flask charges for the player.");

    if (render_inventory_select_mode_combo("Inventory Select Mode", settings.inventory_select_mode_value)) changed = true;
    RenderTooltip("How to choose a potion from inventory when several items match the inventory keyword.");
  }

  void render_inventory_settings(config::flask_settings_base& settings, bool& changed)
  {
    if (render_inventory_mode_combo_no_use("Inventory Mode", settings.inventory_mode_value)) changed = true;
    RenderTooltip("Disabled: ignore inventory support. Deposit: auto-convert matching inventory potions into flask charges.");

    if (render_inventory_select_mode_combo("Inventory Select Mode", settings.inventory_select_mode_value)) changed = true;
    RenderTooltip("How to choose a potion from inventory when several items match the inventory keyword.");
  }

  void render_flask_settings_group(const char* label, config::flask_settings& settings)
  {
    if (ImGui::CollapsingHeader(label)) {
      ImGui::PushID(label);
      bool changed = false;

      if (ImGui::Checkbox("Enable", &settings.enable)) changed = true;
      RenderTooltip("Enable or disable this flask type globally.");

      if (ImGui::Checkbox("Enable for Player", &settings.player)) changed = true;
      RenderTooltip("Enable this flask type for the Player.");

      if (ImGui::Checkbox("Enable for NPC", &settings.npc)) changed = true;
      RenderTooltip("Enable this flask type for NPCs.");

      if (render_keybind_combo("Keyboard Hotkey", settings.hotkey)) changed = true;
      RenderTooltip("Keyboard key used to drink this flask type. 'None' disables the keyboard hotkey.");

      if (render_keybind_combo("Keyboard Modifier", settings.hotkey_modifier)) changed = true;
      RenderTooltip("Optional keyboard modifier that must be held together with the keyboard hotkey. 'None' disables the modifier.");

      if (render_gamepad_keybind_combo("Gamepad Hotkey", settings.gamepad_hotkey)) changed = true;
      RenderTooltip("Gamepad button used to drink this flask type. 'None' disables the gamepad hotkey.");

      if (render_gamepad_keybind_combo("Gamepad Modifier", settings.gamepad_hotkey_modifier)) changed = true;
      RenderTooltip("Optional gamepad modifier that must be held together with the gamepad hotkey. 'None' disables the modifier.");

      render_inventory_settings(settings, changed);

      // Notify string handling
      char buffer_notify[512];
      strncpy_s(buffer_notify, settings.notify.c_str(), sizeof(buffer_notify) - 1);
      if (ImGui::InputText("Notification", buffer_notify, sizeof(buffer_notify))) {
        settings.notify = buffer_notify;
        changed = true;
      }
      RenderTooltip("Notification message shown to the player when they cannot drink more of this flask.");
      
      if (ImGui::Checkbox("Enable Fail Audio", &settings.fail_audio)) changed = true;
      RenderTooltip("If enabled, play audio feedback when a flask fails to activate.");

      if (ImGui::Checkbox("Parallel Cooldown", &settings.enable_parallel_cooldown)) changed = true;
      RenderTooltip("If true, all slots cool down simultaneously. If false, they cool down one by one.");

      if (ImGui::Checkbox("Anti-Spam", &settings.anti_spam)) changed = true;
      RenderTooltip("Prevents spamming the flask by adding a small delay between uses.");

      if (settings.anti_spam) {
        if (ImGui::DragFloat("Anti-Spam Delay", &settings.anti_spam_delay, 0.1f, 0.0f, 10.0f, "%.1f")) changed = true;
        RenderTooltip("The delay in seconds for the anti-spam feature.");
      }

      if (ImGui::DragFloat("Regeneration Mult Base", &settings.regeneration_mult_base, 1.0f, 0.0f, 1000.0f, "%.1f"))
        changed = true;
      RenderTooltip("Base multiplier for cooldown regeneration speed (in percentage).");

      if (ImGui::InputInt("Cap Base", &settings.cap_base)) changed = true;
      RenderTooltip("Base maximum number of flask slots available.");

      if (ImGui::DragFloat("Cooldown Base", &settings.cooldown_base, 1.0f, 0.0f, 3600.0f, "%.1f")) changed = true;
      RenderTooltip("Base cooldown duration in seconds for one slot.");

      if (changed) {
        config::config_manager::get_singleton()->save();
        prisma::send_settings();
      }
      ImGui::PopID();
    }
  }

  void __stdcall render_flasks_settings()
  {
    auto config = config::config_manager::get_singleton();

    render_flask_settings_group("Health Flasks", config->flasks_health);
    render_flask_settings_group("Stamina Flasks", config->flasks_stamina);
    render_flask_settings_group("Magick Flasks", config->flasks_magick);

    if (ImGui::CollapsingHeader("Other Flasks")) {
      ImGui::PushID("Other Flasks");
      bool changed = false;
      auto& settings = config->flasks_other;

      if (ImGui::Checkbox("Enable", &settings.enable)) changed = true;
      RenderTooltip("Enable or disable this flask type globally.");

      if (ImGui::Checkbox("Enable for Player", &settings.player)) changed = true;
      RenderTooltip("Enable this flask type for the Player.");

      if (ImGui::Checkbox("Enable for NPC", &settings.npc)) changed = true;
      RenderTooltip("Enable this flask type for NPCs.");

      render_inventory_settings(settings, changed);

      char buffer[256];
      strncpy_s(buffer, settings.notify.c_str(), sizeof(buffer) - 1);
      if (ImGui::InputText("Notification", buffer, sizeof(buffer))) {
        settings.notify = buffer;
        changed = true;
      }
      RenderTooltip("Notification message shown to the player when they cannot drink more of this flask.");
      
      if (ImGui::Checkbox("Enable Fail Audio", &settings.fail_audio)) changed = true;
      RenderTooltip("If enabled, play audio feedback when a flask fails to activate.");

      if (ImGui::Checkbox("Parallel Cooldown", &settings.enable_parallel_cooldown)) changed = true;
      RenderTooltip("If true, all slots cool down simultaneously. If false, they cool down one by one.");

      if (ImGui::Checkbox("Anti-Spam", &settings.anti_spam)) changed = true;
      RenderTooltip("Prevents spamming the flask by adding a small delay between uses.");

      if (settings.anti_spam) {
        if (ImGui::DragFloat("Anti-Spam Delay", &settings.anti_spam_delay, 0.1f, 0.0f, 10.0f, "%.1f")) changed = true;
        RenderTooltip("The delay in seconds for the anti-spam feature.");
      }

      if (ImGui::DragFloat("Regeneration Mult Base", &settings.regeneration_mult_base, 1.0f, 0.0f, 1000.0f, "%.1f"))
        changed = true;
      RenderTooltip("Base multiplier for cooldown regeneration speed (in percentage).");

      if (ImGui::InputInt("Cap Base", &settings.cap_base)) changed = true;
      RenderTooltip("Base maximum number of flask slots available.");

      if (ImGui::DragFloat("Cooldown Base", &settings.cooldown_base, 1.0f, 0.0f, 3600.0f, "%.1f")) changed = true;
      RenderTooltip("Base cooldown duration in seconds for one slot.");

      if (ImGui::Checkbox("Revert Exclusive", &settings.revert_exclusive)) changed = true;
      RenderTooltip("If true, only items WITH the exclusive keyword are treated as 'Other'; otherwise only items WITHOUT it are.");

      if (changed) {
        config->save();
        prisma::send_settings();
      }
      ImGui::PopID();
    }
  }

  void render_prisma_flask_widget_settings(const char* label, config::prisma_flask_widget_settings& settings)
  {
    if (ImGui::TreeNode(label)) {
      ImGui::PushID(label);
      bool changed = false;
      if (ImGui::DragFloat("X", &settings.x, 0.001f, -1.00f, 1.0f, "%.3f")) changed = true;
      RenderTooltip("Relative X position for this flask type.");

      if (ImGui::DragFloat("Y", &settings.y, 0.001f, -1.00f, 1.0f, "%.3f")) changed = true;
      RenderTooltip("Relative Y position for this flask type.");

      if (ImGui::DragFloat("Size", &settings.size, 0.001f, 0.0f, 5.0f, "%.3f")) changed = true;
      RenderTooltip("Scale for this flask type.");

      if (ImGui::DragFloat("Opacity", &settings.opacity, 0.001f, 0.0f, 1.0f, "%.3f")) changed = true;
      RenderTooltip("Opacity for this flask type.");
      
      if (ImGui::Checkbox("Fill Animation", &settings.fill_animation)) changed = true;
      RenderTooltip("If true, enables the visual fill animation for this flask widget.");
      
      if (ImGui::Checkbox("Fill Animation Only Zero", &settings.fill_animation_only_zero)) changed = true;
      RenderTooltip("If true, the fill animation plays ONLY when the flask is completely empty (0 charges).");

      if (changed) {
        config::config_manager::get_singleton()->save();
        prisma::send_settings();
      }
      ImGui::PopID();
      ImGui::TreePop();
    }
  }

  void __stdcall render_prisma_settings()
  {
    auto config = config::config_manager::get_singleton();
    bool changed = false;
    auto last_enable_value = config->prisma_widget.enable;

    if (ImGui::Checkbox("Enable Prisma Widget", &config->prisma_widget.enable)) changed = true;
    RenderTooltip("Enable or disable the UI widget.");

    if (ImGui::Checkbox("Auto Hide UI", &config->prisma_widget.auto_hide_ui)) changed = true;
    RenderTooltip("Automatically hide the UI when flasks are full.");
    
    if (ImGui::Checkbox("Always Show In Combat", &config->prisma_widget.always_show_in_combat)) changed = true;
    RenderTooltip("If true, all widget elements remain visible in combat.");

    if (config->prisma_widget.enable) {
      if (ImGui::DragFloat("Global X", &config->prisma_widget.x, 0.001f, 0.00f, 1.0f, "%.3f")) changed = true;
      RenderTooltip("Global X position of the widget (0.0 to 1.0).");

      if (ImGui::DragFloat("Global Y", &config->prisma_widget.y, 0.001f, 0.00f, 1.0f, "%.3f")) changed = true;
      RenderTooltip("Global Y position of the widget (0.0 to 1.0).");

      if (ImGui::DragFloat("Global Size", &config->prisma_widget.size, 0.001f, 0.0f, 5.0f, "%.3f")) changed = true;
      RenderTooltip("Global scale of the widget.");

      if (ImGui::DragFloat("Global Opacity", &config->prisma_widget.opacity, 0.001f, 0.0f, 1.0f, "%.3f")) changed = true;
      RenderTooltip("Global opacity of the widget (0.0 to 1.0).");

      if (ImGui::Checkbox("Anchor All Elements", &config->prisma_widget.anchor_all_elements)) changed = true;
      RenderTooltip("If true, all flask elements move together relative to the global position.");

      ImGui::Separator();

      render_prisma_flask_widget_settings("Health Widget", config->prisma_widget.health);
      render_prisma_flask_widget_settings("Stamina Widget", config->prisma_widget.stamina);
      render_prisma_flask_widget_settings("Magick Widget", config->prisma_widget.magick);
      render_prisma_flask_widget_settings("Other Widget", config->prisma_widget.other);
    }

    if (changed) {
      config->save();
      if (last_enable_value != config->prisma_widget.enable) {
        prisma::update_enable(config->prisma_widget.enable);
      }
      prisma::send_settings();
    }
  }

  export auto register_skse_menu() -> void
  {
    if (!SKSEMenuFramework::IsInstalled()) {
      logger::warn("SKSEMenuFramework not installed");
      return;
    }

    static constexpr auto main_title = "True Flasks NG";
    SKSEMenuFramework::SetSection(main_title);

    static constexpr auto settings_title = "Flasks Settings";
    SKSEMenuFramework::AddSectionItem(settings_title, render_flasks_settings);

    static constexpr auto prisma_settings = "Prisma Settings";
    SKSEMenuFramework::AddSectionItem(prisma_settings, render_prisma_settings);
  }
}
