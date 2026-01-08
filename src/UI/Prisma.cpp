module;

#include "library/PrismaUI_API.h"
#include "API/TrueFlasksAPI.h"

export module TrueFlasks.UI.Prisma;

import TrueFlasks.Core.ModsAPIRepository;
import TrueFlasks.Core.HooksCtx;
import TrueFlasks.Config;
import TrueFlasks.Features.TrueFlasks;
import TrueFlasks.Core.ActorsCache;
import TrueFlasks.Events.EventsCtx;

namespace ui::prisma
{
  auto get_view_ref() -> PrismaView&
  {
    static PrismaView view = 0;
    return view;
  }

  export void send_settings(const bool init = false)
  {
    auto api = core::mods_api_repository::get_prisma_ui();
    auto& view = get_view_ref();
    if (!api || !view) return;

    const auto& cfg = config::config_manager::get_singleton()->prisma_widget;

    std::string json = std::format(
      R"({{ "enable": {}, "x": {:.2f}, "y": {:.2f}, "size": {:.2f}, "opacity": {:.2f}, "anchor_all": {}, 
            "health": {{ "x": {:.2f}, "y": {:.2f}, "size": {:.2f}, "opacity": {:.2f} }},
            "stamina": {{ "x": {:.2f}, "y": {:.2f}, "size": {:.2f}, "opacity": {:.2f} }},
            "magick": {{ "x": {:.2f}, "y": {:.2f}, "size": {:.2f}, "opacity": {:.2f} }},
            "other": {{ "x": {:.2f}, "y": {:.2f}, "size": {:.2f}, "opacity": {:.2f} }} }})",
      cfg.enable ? "true" : "false", cfg.x, cfg.y, cfg.size, cfg.opacity, cfg.anchor_all_elements ? "true" : "false",
      cfg.health.x, cfg.health.y, cfg.health.size, cfg.health.opacity,
      cfg.stamina.x, cfg.stamina.y, cfg.stamina.size, cfg.stamina.opacity,
      cfg.magick.x, cfg.magick.y, cfg.magick.size, cfg.magick.opacity,
      cfg.other.x, cfg.other.y, cfg.other.size, cfg.other.opacity
      );

    if (init) {
      api->InteropCall(view, "setWidgetSettingsInit", json.c_str());
      return;
    }

    api->InteropCall(view, "setWidgetSettings", json.c_str());
  }

  void on_dom_ready(PrismaView)
  {
    logger::info("TrueFlasksNG View DOM is ready.");

    auto api = core::mods_api_repository::get_prisma_ui();
    if (api) {
      send_settings(true);
    }
  }

  export auto initialize() -> void
  {
    auto api = core::mods_api_repository::get_prisma_ui();
    if (!api) {
      logger::warn("PrismaUI not installed or API not available.");
      return;
    }

    constexpr auto view_path = "TrueFlasksNG/TrueFlasksWidgetIndex.html";

    auto& view = get_view_ref();


    if (api->IsValid(view)) {
      api->Destroy(view);
    }

    view = api->CreateView(view_path, on_dom_ready);

    if (view) {
      api->Hide(view);
      logger::info("TrueFlasksNG view created successfully.");
    }
    else {
      logger::error("Failed to create TrueFlasksNG view.");
    }
  }

  struct flask_state
  {
    float fill_percent{-1.0f};
    int count{-1};
  };

  flask_state last_states[4];

  void update_flask(PRISMA_UI_API::IVPrismaUI1* api, PrismaView view, RE::Actor* actor, TrueFlasksAPI::FlaskType type,
                    int type_idx, bool force_glow = false)
  {
    float pct = features::true_flasks::api_get_cooldown_pct(actor, type);
    int count = features::true_flasks::api_get_current_slots(actor, type);

    if (force_glow || std::abs(pct - last_states[type_idx].fill_percent) > 0.001f || count != last_states[type_idx].
        count) {
      last_states[type_idx].fill_percent = pct;
      last_states[type_idx].count = count;

      // InteropCall accepts only one string argument.
      // We use CSV format: "type,fillPercent,count,shouldGlow"
      std::string args = std::format("{},{:.3f},{},{}", type_idx, pct, count, force_glow ? "1" : "0");
      api->InteropCall(view, "updateFlaskData", args.c_str());
    }
  }

  export void update(const core::hooks_ctx::on_actor_update& ctx)
  {
    if (!ctx.actor || !ctx.actor->IsPlayerRef()) return;

    auto api = core::mods_api_repository::get_prisma_ui();
    auto& view = get_view_ref();
    if (!api || !view) return;

    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(ctx.actor->GetFormID());

    bool glow_health = false;
    bool glow_stamina = false;
    bool glow_magick = false;
    bool glow_other = false;

    if (actor_data.failed_drink_type.has_value()) {
      switch (*actor_data.failed_drink_type) {
      case TrueFlasksAPI::FlaskType::Health: glow_health = true;
        break;
      case TrueFlasksAPI::FlaskType::Stamina: glow_stamina = true;
        break;
      case TrueFlasksAPI::FlaskType::Magick: glow_magick = true;
        break;
      case TrueFlasksAPI::FlaskType::Other: glow_other = true;
        break;
      }
      actor_data.failed_drink_type.reset();
    }

    update_flask(api, view, ctx.actor, TrueFlasksAPI::FlaskType::Health, 0, glow_health);
    update_flask(api, view, ctx.actor, TrueFlasksAPI::FlaskType::Stamina, 1, glow_stamina);
    update_flask(api, view, ctx.actor, TrueFlasksAPI::FlaskType::Magick, 2, glow_magick);
    update_flask(api, view, ctx.actor, TrueFlasksAPI::FlaskType::Other, 3, glow_other);
  }

  export void on_menu_event(const events::events_ctx::process_event_menu_ctx& ctx)
  {
    static constexpr std::string_view true_hud = "TrueHUD";

    if (!ctx.is_opening && ctx.menu_name == RE::JournalMenu::MENU_NAME) {
      send_settings();
    }

    auto prisma = core::mods_api_repository::get_prisma_ui();
    auto& view = get_view_ref();

    if (!prisma) {
      return;
    }

    if (ctx.menu_name == RE::LoadingMenu::MENU_NAME) {
      ctx.is_opening ? prisma->Hide(view) : prisma->Show(view);
      return;
    }

    if (RE::UI::GetSingleton()->IsMenuOpen(RE::LoadingMenu::MENU_NAME)) {
      prisma->Hide(view);
      return;
    }

    if (ctx.menu_name == RE::CursorMenu::MENU_NAME && !RE::UI::GetSingleton()->IsMenuOpen(RE::LoadingMenu::MENU_NAME)) {
      ctx.is_opening ? prisma->Hide(view) : prisma->Show(view);
    }


    if (ctx.menu_name == RE::HUDMenu::MENU_NAME || ctx.menu_name == true_hud) {
      ctx.is_opening ? prisma->Show(view) : prisma->Hide(view);
    }
  }
}
