module;

#define NOMINMAX
#undef min
#undef max

export module TrueFlasks.Features.TrueFlasks;

import TrueFlasks.Core.HooksCtx;
import TrueFlasks.Config;
import TrueFlasks.Core.ActorsCache;
import TrueFlasks.Core.Utility;

namespace features::true_flasks
{

export enum class flask_type
{
  health,
  stamina,
  magick,
  other
};

const config::flask_settings_base* get_settings(const config::config_manager* config, const flask_type type) {
    switch (type) {
        case flask_type::health: return &config->flasks_health;
        case flask_type::stamina: return &config->flasks_stamina;
        case flask_type::magick: return &config->flasks_magick;
        case flask_type::other: return &config->flasks_other;
    }
    return nullptr;
}

std::optional<flask_type> identify_flask_type(const RE::AlchemyItem* potion, const config::config_manager* config) {
    if (config->flasks_health.keyword && core::utility::try_form_has_keyword(potion, config->flasks_health.keyword)) return flask_type::health;
    if (config->flasks_stamina.keyword && core::utility::try_form_has_keyword(potion, config->flasks_stamina.keyword)) return flask_type::stamina;
    if (config->flasks_magick.keyword && core::utility::try_form_has_keyword(potion, config->flasks_magick.keyword)) return flask_type::magick;
    
    return flask_type::other;
}

int calculate_max_slots(RE::Actor* actor, const config::flask_settings_base& settings) {
    auto base = static_cast<float>(settings.cap_base);
    if (settings.cap_keyword) {
        const auto effects = core::utility::get_active_effects_by_keyword(actor, settings.cap_keyword);
        base += core::utility::get_magnitude_sum_of_active_effects(&effects);
    }
    return static_cast<int>((std::max)(0.f, base));
}

float calculate_cooldown(RE::Actor* actor, const config::flask_settings_base& settings) {
    auto base = settings.cooldown_base;
    if (settings.cooldown_keyword) {
        const auto effects = core::utility::get_active_effects_by_keyword(actor, settings.cooldown_keyword);
        base += core::utility::get_magnitude_sum_of_active_effects(&effects);
    }
    return (std::max)(0.f, base);
}

float calculate_regen_mult(RE::Actor* actor, const config::flask_settings_base& settings) {
    auto base = settings.regeneration_mult_base;
    if (settings.regeneration_mult_keyword) {
        const auto effects = core::utility::get_active_effects_by_keyword(actor, settings.regeneration_mult_keyword);
        base += core::utility::get_magnitude_sum_of_active_effects(&effects);
    }
    return (std::max)(0.f, base) / 100.0f;
}

float calculate_regen_mult_raw(RE::Actor* actor, const config::flask_settings_base& settings) {
    auto base = settings.regeneration_mult_base;
    if (settings.regeneration_mult_keyword) {
        const auto effects = core::utility::get_active_effects_by_keyword(actor, settings.regeneration_mult_keyword);
        base += core::utility::get_magnitude_sum_of_active_effects(&effects);
    }
    return (std::max)(0.f, base);
}

bool try_use_flask(core::actors_cache::cache_data::actor_data::flask_cooldown* flasks, const float cooldown_duration, int max_slots) {
    if (!flasks) return false;
    
    if (max_slots > core::actors_cache::cache_data::actor_data::FLASK_ARRAY_SIZE) 
        max_slots = core::actors_cache::cache_data::actor_data::FLASK_ARRAY_SIZE;

    for (int i = 0; i < max_slots; ++i) {
        if (flasks[i].cooldown_current <= 0.f) {
            flasks[i].cooldown_start = cooldown_duration;
            flasks[i].cooldown_current = cooldown_duration;
            return true;
        }
    }
    return false;
}

core::actors_cache::cache_data::actor_data::flask_cooldown* get_flasks_array(core::actors_cache::cache_data::actor_data& data, const flask_type type) {
    switch (type) {
        case flask_type::health: return data.flasks_health;
        case flask_type::stamina: return data.flasks_stamina;
        case flask_type::magick: return data.flasks_magick;
        case flask_type::other: return data.flasks_others;
    }
    return nullptr;
}

export bool drink_potion(const core::hooks_ctx::on_actor_drink_potion& ctx)
{
  if (ctx.potion->IsFood() || ctx.potion->IsPoison()) {
    return true;
  }
  
  const auto config = config::config_manager::get_singleton();
  const auto type_opt = identify_flask_type(ctx.potion, config);
  
  if (!type_opt.has_value()) {
      return true;
  }
  
  const auto type = type_opt.value();
  
  // Special logic for Other
  if (type == flask_type::other) {
      const auto has_kw = config->flasks_other.exclusive_keyword && core::utility::try_form_has_keyword(ctx.potion, config->flasks_other.exclusive_keyword);
      
      // Если revert_exclusive == has_kw, то тратим слот. Иначе - не тратим.
      if (config->flasks_other.revert_exclusive != has_kw) {
          return true; // Не тратим слот, разрешаем пить
      }
  }
  
  const auto settings = get_settings(config, type);
  
  if (!settings->enable) return true;
  
  const auto is_player = ctx.actor->IsPlayerRef();
  if (is_player && !settings->player) return true;
  if (!is_player && !settings->npc) return true;

  auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(ctx.actor->GetFormID());
  
  if (settings->anti_spam && actor_data.anti_spam_duration > 0.f) {
      return false;
  }
  
  const auto max_slots = calculate_max_slots(ctx.actor, *settings);
  const auto cooldown = calculate_cooldown(ctx.actor, *settings);
  
  auto flasks = get_flasks_array(actor_data, type);
  
  if (try_use_flask(flasks, cooldown, max_slots)) {
      if (settings->anti_spam) {
          actor_data.anti_spam_duration = settings->anti_spam_delay;
      }
      return true;
  }
  
  if (is_player && !settings->notify.empty()) {
      RE::DebugNotification(settings->notify.c_str());
  }
  
  return false;
}

export void update(const core::hooks_ctx::on_actor_update& ctx)
{
  auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(ctx.actor->GetFormID());
  const auto config = config::config_manager::get_singleton();
  
  auto d_data = core::actors_cache::cache_data::actor_data::delta_data{};
  d_data.delta = ctx.delta;
  
  auto calc_delta = [&](const flask_type type) {
      const auto settings = get_settings(config, type);
      const auto mult = calculate_regen_mult(ctx.actor, *settings);
      return ctx.delta * mult;
  };
  
  d_data.delta_health = calc_delta(flask_type::health);
  d_data.delta_stamina = calc_delta(flask_type::stamina);
  d_data.delta_magick = calc_delta(flask_type::magick);
  d_data.delta_other = calc_delta(flask_type::other);
  
  actor_data.update(d_data);
}

// API Functions

export auto api_get_max_slots(RE::Actor* actor, const flask_type type) -> int {
    if (!actor) return 0;
    const auto config = config::config_manager::get_singleton();
    const auto settings = get_settings(config, type);
    if (!settings) return 0;
    return calculate_max_slots(actor, *settings);
}

export auto api_get_current_slots(RE::Actor* actor, const flask_type type) -> int {
    if (!actor) return 0;
    
    const auto max_slots = api_get_max_slots(actor, type);
    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(actor->GetFormID());
    auto flasks = get_flasks_array(actor_data, type);
    
    if (!flasks) return 0;
    
    int available = 0;
    const int limit = (std::min)(max_slots, core::actors_cache::cache_data::actor_data::FLASK_ARRAY_SIZE);
    
    for (int i = 0; i < limit; ++i) {
        if (flasks[i].cooldown_current <= 0.f) {
            available++;
        }
    }
    return available;
}

export auto api_get_next_cooldown(RE::Actor* actor, const flask_type type) -> float {
    if (!actor) return 0.f;
    
    const auto max_slots = api_get_max_slots(actor, type);
    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(actor->GetFormID());
    auto flasks = get_flasks_array(actor_data, type);
    
    if (!flasks) return 0.f;
    
    float min_cd = -1.f;
    const int limit = (std::min)(max_slots, core::actors_cache::cache_data::actor_data::FLASK_ARRAY_SIZE);

    for (int i = 0; i < limit; ++i) {
        if (flasks[i].cooldown_current > 0.f) {
            if (min_cd < 0.f || flasks[i].cooldown_current < min_cd) {
                min_cd = flasks[i].cooldown_current;
            }
        }
    }
    return (std::max)(0.f, min_cd);
}

export auto api_modify_cooldown(RE::Actor* actor, const flask_type type, const float amount, const bool all_slots) -> void {
    if (!actor) return;
    
    const auto max_slots = api_get_max_slots(actor, type);
    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(actor->GetFormID());
    auto flasks = get_flasks_array(actor_data, type);
    
    if (!flasks) return;
    
    const int limit = (std::min)(max_slots, core::actors_cache::cache_data::actor_data::FLASK_ARRAY_SIZE);

    if (all_slots) {
        for (int i = 0; i < limit; ++i) {
            if (flasks[i].cooldown_current > 0.f) {
                flasks[i].cooldown_current = (std::max)(0.f, flasks[i].cooldown_current + amount);
            }
        }
    } else {
        // Find nearest cooldown
        int nearest_idx = -1;
        float min_cd = -1.f;
        
        for (int i = 0; i < limit; ++i) {
            if (flasks[i].cooldown_current > 0.f) {
                if (min_cd < 0.f || flasks[i].cooldown_current < min_cd) {
                    min_cd = flasks[i].cooldown_current;
                    nearest_idx = i;
                }
            }
        }
        
        if (nearest_idx >= 0) {
            flasks[nearest_idx].cooldown_current = (std::max)(0.f, flasks[nearest_idx].cooldown_current + amount);
        }
    }
}

export auto api_get_regen_mult(RE::Actor* actor, const flask_type type) -> float {
    if (!actor) return 0.f;
    const auto config = config::config_manager::get_singleton();
    const auto settings = get_settings(config, type);
    if (!settings) return 0.f;
    return calculate_regen_mult_raw(actor, *settings);
}

export auto api_get_cooldown_pct(RE::Actor* actor, const flask_type type) -> float {
    if (!actor) return 1.0f;
    
    const auto max_slots = api_get_max_slots(actor, type);
    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(actor->GetFormID());
    auto flasks = get_flasks_array(actor_data, type);
    
    if (!flasks) return 1.0f;
    
    int nearest_idx = -1;
    float min_cd = -1.f;
    const int limit = (std::min)(max_slots, core::actors_cache::cache_data::actor_data::FLASK_ARRAY_SIZE);

    for (int i = 0; i < limit; ++i) {
        if (flasks[i].cooldown_current > 0.f) {
            if (min_cd < 0.f || flasks[i].cooldown_current < min_cd) {
                min_cd = flasks[i].cooldown_current;
                nearest_idx = i;
            }
        }
    }
    
    if (nearest_idx >= 0) {
        if (flasks[nearest_idx].cooldown_start <= 0.f) return 1.0f;
        return 1.0f - (flasks[nearest_idx].cooldown_current / flasks[nearest_idx].cooldown_start);
    }
    
    return 1.0f;
}

}
