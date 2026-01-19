module;

#include "library/SKSEMenuFramework.h"
#include <string>

export module TrueFlasks.UI.SKSEMenu;

import TrueFlasks.Config;
import TrueFlasks.UI.Prisma;

namespace ui::skse_menu
{
  using namespace ImGuiMCP;

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

  void render_flask_settings_group(const char* label, config::flask_settings_base& settings)
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

      // Notify string handling
      char buffer[256];
      strncpy_s(buffer, settings.notify.c_str(), sizeof(buffer) - 1);
      if (ImGui::InputText("Notification", buffer, sizeof(buffer))) {
        settings.notify = buffer;
        changed = true;
      }
      RenderTooltip("Notification message shown to the player when they cannot drink more of this flask.");

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

      char buffer[256];
      strncpy_s(buffer, settings.notify.c_str(), sizeof(buffer) - 1);
      if (ImGui::InputText("Notification", buffer, sizeof(buffer))) {
        settings.notify = buffer;
        changed = true;
      }
      RenderTooltip("Notification message shown to the player when they cannot drink more of this flask.");

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
      RenderTooltip("If true, inverts the logic for the exclusive keyword (items WITH the keyword are NOT 'Other').");

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
