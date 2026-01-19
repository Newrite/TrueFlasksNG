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
  struct flask_widget_settings
  {
    float x{0.5f};
    float y{0.5f};
    float size{0.5f};
    float opacity{0.5f};
    bool enabled{true};
  };

  struct global_widget_settings
  {
    bool enable{true};
    float x{0.25f};
    float y{0.25f};
    float size{1.0f};
    float opacity{0.5f};
    bool anchor_all{true};
    bool auto_hide{false};

    flask_widget_settings health;
    flask_widget_settings stamina;
    flask_widget_settings magick;
    flask_widget_settings other;
  };
  
  struct flask_update_data
  {
      int typeIndex;
      float percent;
      int count;
      int max_slots;
      bool forceGlow;
  };
  
  bool view_init = false;
  bool first_init = true;

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

    const auto config = config::config_manager::get_singleton();
    const auto& cfg = config->prisma_widget;

    global_widget_settings settings;
    settings.enable = cfg.enable;
    settings.x = cfg.x;
    settings.y = cfg.y;
    settings.size = cfg.size;
    settings.opacity = cfg.opacity;
    settings.anchor_all = cfg.anchor_all_elements;
    settings.auto_hide = config->prisma_widget.auto_hide_ui;

    auto fill_flask_settings = [&](flask_widget_settings& out, const config::prisma_flask_widget_settings& in, const config::flask_settings_base& base) {
        out.x = in.x;
        out.y = in.y;
        out.size = in.size;
        out.opacity = in.opacity;
        out.enabled = base.enable && base.player;
    };

    fill_flask_settings(settings.health, cfg.health, config->flasks_health);
    fill_flask_settings(settings.stamina, cfg.stamina, config->flasks_stamina);
    fill_flask_settings(settings.magick, cfg.magick, config->flasks_magick);
    fill_flask_settings(settings.other, cfg.other, config->flasks_other);

    std::string json;
    if (const auto ec = glz::write_json(settings, json)) {
      logger::error("Failed to serialize global_widget_settings, error code: {}", static_cast<int>(ec.ec));
      return;
    }

    if (init) {
      if (first_init) {
        first_init = false;
        api->Hide(view);
      }
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
      view_init = true;
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

    view = api->CreateView(view_path, on_dom_ready);

    if (view) {
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
    int max_slots{-1};
  };

  flask_state last_states[4];

  void update_flask(PRISMA_UI_API::IVPrismaUI1* api, PrismaView view, RE::Actor* actor, TrueFlasksAPI::FlaskType type,
                    int type_idx, bool force_glow = false)
  {
    // Получаем данные
    float pct = features::true_flasks::api_get_cooldown_pct(actor, type);
    int count = features::true_flasks::api_get_current_slots(actor, type);
    int max_slots = features::true_flasks::api_get_max_slots(actor, type);

    // Проверяем изменения
    // Мы отправляем данные, если:
    // 1. Форсировано свечение (неудачная попытка выпить)
    // 2. Изменился процент заполнения (с порогом для оптимизации)
    // 3. Изменилось количество зарядов (КРИТИЧНО для цифры и финального свечения)
    // 4. Изменилось максимальное количество зарядов (для автоскрытия)
    // bool pct_changed = std::abs(pct - last_states[type_idx].fill_percent) > 0.005f; // Чуть увеличил порог, 0.001 слишком часто
    // bool count_changed = count != last_states[type_idx].count;
    // bool max_slots_changed = max_slots != last_states[type_idx].max_slots;
    
    // if (force_glow || pct_changed || count_changed || max_slots_changed) {
      
      last_states[type_idx].fill_percent = pct;
      last_states[type_idx].count = count;
      last_states[type_idx].max_slots = max_slots;

      flask_update_data data{type_idx, pct, count, max_slots, force_glow};
      
      std::string json;
      // Используем write_json из glaze, как и было
      if (const auto ec = glz::write_json(data, json)) {
        // Логирование ошибки можно убрать в релизе для спама
        return;
      }
      
      // Используем InteropCall для максимальной скорости обновления
      api->InteropCall(view, "updateFlaskData", json.c_str());
    // }
  }

  export void update(const core::hooks_ctx::on_actor_update& ctx)
  {
    
    if (!config::config_manager::get_singleton()->prisma_widget.enable || !view_init) {
      return;
    }
    
    if (!ctx.actor || !ctx.actor->IsPlayerRef()) return;

    auto api = core::mods_api_repository::get_prisma_ui();
    auto& view = get_view_ref();
    if (!api) return;
    
    if (!api->IsValid(view)) {
      return;
    }

    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(ctx.actor->GetFormID());

    bool glow_health = false;
    bool glow_stamina = false;
    bool glow_magick = false;
    bool glow_other = false;

    // Check array for flags
    if (actor_data.failed_drink_types[0]) { glow_health = true; actor_data.failed_drink_types[0] = false; }
    if (actor_data.failed_drink_types[1]) { glow_stamina = true; actor_data.failed_drink_types[1] = false; }
    if (actor_data.failed_drink_types[2]) { glow_magick = true; actor_data.failed_drink_types[2] = false; }
    if (actor_data.failed_drink_types[3]) { glow_other = true; actor_data.failed_drink_types[3] = false; }

    update_flask(api, view, ctx.actor, TrueFlasksAPI::FlaskType::Health, 0, glow_health);
    update_flask(api, view, ctx.actor, TrueFlasksAPI::FlaskType::Stamina, 1, glow_stamina);
    update_flask(api, view, ctx.actor, TrueFlasksAPI::FlaskType::Magick, 2, glow_magick);
    update_flask(api, view, ctx.actor, TrueFlasksAPI::FlaskType::Other, 3, glow_other);
    
    if (api->IsHidden(view)) {
      api->Show(view);
    }
  }
  
  export void update_enable(const bool enable)
  {
    auto api = core::mods_api_repository::get_prisma_ui();
    auto& view = get_view_ref();
    
    if (!api) {
      return;
    }
    
    if (!enable) {
      view_init = false;
      api->Destroy(view);
      view = 0;
      return;
    }
    
    initialize();

  }

  export void on_menu_event(const events::events_ctx::process_event_menu_ctx& ctx)
  {
    static constexpr std::string_view true_hud = "TrueHUD";

    if (!ctx.is_opening && ctx.menu_name == RE::JournalMenu::MENU_NAME) {
      send_settings();
    }

    auto prisma = core::mods_api_repository::get_prisma_ui();
    auto& view = get_view_ref();
    /*
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
    }*/
    
    if (auto ui = RE::UI::GetSingleton()) {
      bool isLoadingMenu = ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME);
      bool shouldShowWidget = !(
          ui->IsMenuOpen(RE::InventoryMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::CraftingMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::BarterMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::TweenMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::GiftMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::ContainerMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::MagicMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::StatsMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::MessageBoxMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::JournalMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::LockpickingMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::SleepWaitMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::RaceSexMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::MapMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::FaderMenu::MENU_NAME) ||
          ui->IsMenuOpen(RE::CursorMenu::MENU_NAME)
          );
      if (shouldShowWidget && !isLoadingMenu) {
        prisma->Invoke(view, "Show()");
        return;
      }
      
      prisma->Invoke(view, "Hide()");
      
    }
    
  }
}
