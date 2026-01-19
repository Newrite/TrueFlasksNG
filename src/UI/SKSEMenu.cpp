module;

#include "library/SKSEMenuFramework.h"
#include <string>

export module TrueFlasks.UI.SKSEMenu;

import TrueFlasks.Config;
import TrueFlasks.UI.Prisma;

namespace ui::skse_menu
{
  using namespace ImGuiMCP;

  void render_flask_settings_group(const char* label, config::flask_settings_base& settings)
  {
    if (ImGui::CollapsingHeader(label)) {
      ImGui::PushID(label);
      bool changed = false;

      if (ImGui::Checkbox("Enable", &settings.enable)) changed = true;
      if (ImGui::Checkbox("Enable for Player", &settings.player)) changed = true;
      if (ImGui::Checkbox("Enable for NPC", &settings.npc)) changed = true;

      // Notify string handling
      char buffer[256];
      strncpy_s(buffer, settings.notify.c_str(), sizeof(buffer) - 1);
      if (ImGui::InputText("Notification", buffer, sizeof(buffer))) {
        settings.notify = buffer;
        changed = true;
      }

      if (ImGui::Checkbox("Parallel Cooldown", &settings.enable_parallel_cooldown)) changed = true;
      if (ImGui::Checkbox("Anti-Spam", &settings.anti_spam)) changed = true;
      if (settings.anti_spam) {
        if (ImGui::DragFloat("Anti-Spam Delay", &settings.anti_spam_delay, 0.1f, 0.0f, 10.0f, "%.1f")) changed = true;
      }

      if (ImGui::DragFloat("Regeneration Mult Base", &settings.regeneration_mult_base, 1.0f, 0.0f, 1000.0f, "%.1f"))
        changed = true;
      if (ImGui::InputInt("Cap Base", &settings.cap_base)) changed = true;
      if (ImGui::DragFloat("Cooldown Base", &settings.cooldown_base, 1.0f, 0.0f, 3600.0f, "%.1f")) changed = true;

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
      if (ImGui::Checkbox("Enable for Player", &settings.player)) changed = true;
      if (ImGui::Checkbox("Enable for NPC", &settings.npc)) changed = true;

      char buffer[256];
      strncpy_s(buffer, settings.notify.c_str(), sizeof(buffer) - 1);
      if (ImGui::InputText("Notification", buffer, sizeof(buffer))) {
        settings.notify = buffer;
        changed = true;
      }

      if (ImGui::Checkbox("Parallel Cooldown", &settings.enable_parallel_cooldown)) changed = true;
      if (ImGui::Checkbox("Anti-Spam", &settings.anti_spam)) changed = true;
      if (settings.anti_spam) {
        if (ImGui::DragFloat("Anti-Spam Delay", &settings.anti_spam_delay, 0.1f, 0.0f, 10.0f, "%.1f")) changed = true;
      }

      if (ImGui::DragFloat("Regeneration Mult Base", &settings.regeneration_mult_base, 1.0f, 0.0f, 1000.0f, "%.1f"))
        changed = true;
      if (ImGui::InputInt("Cap Base", &settings.cap_base)) changed = true;
      if (ImGui::DragFloat("Cooldown Base", &settings.cooldown_base, 1.0f, 0.0f, 3600.0f, "%.1f")) changed = true;

      if (ImGui::Checkbox("Revert Exclusive", &settings.revert_exclusive)) changed = true;

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
      if (ImGui::DragFloat("Y", &settings.y, 0.001f, -1.00f, 1.0f, "%.3f")) changed = true;
      if (ImGui::DragFloat("Size", &settings.size, 0.001f, 0.0f, 5.0f, "%.3f")) changed = true;
      if (ImGui::DragFloat("Opacity", &settings.opacity, 0.001f, 0.0f, 1.0f, "%.3f")) changed = true;

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
    if (ImGui::Checkbox("Auto Hide UI", &config->prisma_widget.auto_hide_ui)) changed = true;

    if (config->prisma_widget.enable) {
      if (ImGui::DragFloat("Global X", &config->prisma_widget.x, 0.001f, -1.00f, 1.0f, "%.3f")) changed = true;
      if (ImGui::DragFloat("Global Y", &config->prisma_widget.y, 0.001f, -1.00f, 1.0f, "%.3f")) changed = true;
      if (ImGui::DragFloat("Global Size", &config->prisma_widget.size, 0.001f, 0.0f, 5.0f, "%.3f")) changed = true;
      if (ImGui::DragFloat("Global Opacity", &config->prisma_widget.opacity, 0.001f, 0.0f, 1.0f, "%.3f")) changed = true;
      if (ImGui::Checkbox("Anchor All Elements", &config->prisma_widget.anchor_all_elements)) changed = true;

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
