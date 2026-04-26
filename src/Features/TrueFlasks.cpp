module;

#include "API/TrueFlasksAPI.h"
#include "RE/E/EffectSetting.h"

export module TrueFlasks.Features.TrueFlasks;

#ifdef PlaySound
#undef PlaySound
#endif

#define NOMINMAX
#undef min
#undef max

import TrueFlasks.Core.HooksCtx;
import TrueFlasks.Config;
import TrueFlasks.Core.ActorsCache;
import TrueFlasks.Core.Utility;
import TrueFlasks.API.ModAPI;
import TrueFlasks.Events.EventsCtx;

namespace features::true_flasks
{
  export using flask_type = TrueFlasksAPI::FlaskType;
  
  using inventory_mode = config::inventory_mode;
  using inventory_select_mode = config::inventory_select_mode;

  constexpr int kFlaskTypeCount = core::actors_cache::cache_data::actor_data::FLASK_TYPE_SIZE;
  constexpr int kFlaskMaxCount = core::actors_cache::cache_data::actor_data::FLASK_ARRAY_SIZE;
  constexpr auto kFlaskTypes = std::array{flask_type::Health, flask_type::Stamina, flask_type::Magick, flask_type::Other};
  using effect_flag = RE::EffectSetting::EffectSettingData::Flag;
  using effect_archetype = RE::EffectSetting::Archetype;

  struct pending_inventory_drink
  {
    RE::FormID actor_id{0};
    RE::FormID potion_id{0};
    bool active{false};
  };

  thread_local pending_inventory_drink g_pending_inventory_drink{};

  bool is_valid_flask_type(const flask_type type)
  {
    const auto type_index = static_cast<int>(type);
    return type_index >= 0 && type_index < kFlaskTypeCount;
  }

  int get_slot_limit(const int max_slots)
  {
    return (std::min)((std::max)(max_slots, 0), kFlaskMaxCount);
  }

  bool is_valid_inventory_use_potion(RE::AlchemyItem* potion, const config::flask_settings_base& settings,
                                     const flask_type type);
  bool is_valid_inventory_deposit_potion(RE::AlchemyItem* potion, const config::flask_settings_base& settings);
  RE::AlchemyItem* get_selected_inventory_potion(RE::Actor* actor, const config::flask_settings_base& settings,
                                                 const flask_type type, const bool for_deposit);
  bool consume_pending_inventory_drink(RE::Actor* actor, const RE::AlchemyItem* potion);

  int get_actual_item_count(RE::Actor* actor, RE::AlchemyItem* potion, const int fallback_count)
  {
    if (!actor || !potion) {
      return 0;
    }

    if (actor->IsPlayerRef()) {
      if (auto* player = RE::PlayerCharacter::GetSingleton()) {
        return player->GetItemCount(potion);
      }
    }

    return fallback_count;
  }

  const config::flask_settings_base* get_settings(const config::config_manager* config, const flask_type type)
  {
    switch (type) {
    case flask_type::Health: return &config->flasks_health;
    case flask_type::Stamina: return &config->flasks_stamina;
    case flask_type::Magick: return &config->flasks_magick;
    case flask_type::Other: return &config->flasks_other;
    }
    return nullptr;
  }

  std::optional<flask_type> identify_flask_type(const RE::AlchemyItem* potion, const config::config_manager* config)
  {
    if (config->flasks_health.keyword && core::utility::try_form_has_keyword(potion, config->flasks_health.keyword))
      return flask_type::Health;
    if (config->flasks_stamina.keyword && core::utility::try_form_has_keyword(potion, config->flasks_stamina.keyword))
      return flask_type::Stamina;
    if (config->flasks_magick.keyword && core::utility::try_form_has_keyword(potion, config->flasks_magick.keyword))
      return flask_type::Magick;

    bool is_other = true;
    if (config->flasks_other.exclusive_keyword) {
      const bool has_kw = core::utility::try_form_has_keyword(potion, config->flasks_other.exclusive_keyword);
      if (config->flasks_other.revert_exclusive) {
        is_other = has_kw;
      }
      else {
        is_other = !has_kw;
      }
    }
    else {
      if (config->flasks_other.revert_exclusive) {
        is_other = false;
      }
    }

    if (is_other) {
      return flask_type::Other;
    }

    return std::nullopt;
  }
  
  bool is_in_inventory_mode(const RE::Actor* actor, const flask_type type)
  {
    auto settings = get_settings(config::config_manager::get_singleton(), type);
    if (actor && core::utility::is_player(actor) && settings->inventory_keyword && settings->inventory_mode_value != inventory_mode::disabled) {
      return true;
    }
    return false;
  }
  
  bool is_in_inventory_mode_deposit(const RE::Actor* actor, const flask_type type)
  {
    auto settings = get_settings(config::config_manager::get_singleton(), type);
    if (actor && core::utility::is_player(actor) && settings->inventory_keyword && settings->inventory_mode_value == inventory_mode::deposit) {
      return true;
    }
    return false;
  }
  
  bool is_in_inventory_mod_use(const RE::Actor* actor, const flask_type type)
  {
    if (type == flask_type::Other) {
      return false;
    }
    auto settings = get_settings(config::config_manager::get_singleton(), type);
    if (actor && core::utility::is_player(actor) && settings->inventory_keyword && settings->inventory_mode_value == inventory_mode::use) {
      return true;
    }
    return false;
  }
  
  export auto try_potion_has_keyword(const RE::AlchemyItem* potion, const RE::BGSKeyword* keyword) -> bool
  {
    if (!potion || !keyword) {
      return false;
    }
    
    if (potion->HasKeyword(keyword)) {
      return true;
    }

    for (const auto effect : potion->effects) {
        if (effect && effect->baseEffect && effect->baseEffect->HasKeyword(keyword)) {
          return true;
        }
    }

    return false;
  }
  
  export auto get_potion_count_with_keyword(RE::TESObjectREFR* a_container, const RE::BGSKeyword* keyword) -> std::int32_t
  {
    std::int32_t iResult = 0;
    
    if (!a_container || !keyword) {
      return iResult;
    }

    auto invChanges = a_container->GetInventoryChanges();
    if (invChanges && invChanges->entryList) {
      for (auto& entry : *invChanges->entryList) {
        if (entry && entry->object && entry->object->GetFormType() == RE::FormType::AlchemyItem && try_potion_has_keyword(entry->object->As<RE::AlchemyItem>(), keyword)) {
          if (entry->IsLeveled()) {
            return entry->countDelta > 0 ? entry->countDelta : 0;
          } else {
            iResult = entry->countDelta;
            break;
          }
        }
      }
    }

    auto container = a_container->GetContainer();
    if (container) {
      container->ForEachContainerObject([&](RE::ContainerObject& a_entry) {
        if (a_entry.obj && a_entry.obj->GetFormType() == RE::FormType::AlchemyItem && try_potion_has_keyword(a_entry.obj->As<RE::AlchemyItem>(), keyword)) {
          iResult += a_entry.count;
          return RE::BSContainer::ForEachResult::kStop;
        }
        return RE::BSContainer::ForEachResult::kContinue;
      });
    }

    return iResult > 0 ? iResult : 0;
  }
  
  int get_potions_count(RE::Actor* actor, const config::flask_settings_base& settings, const flask_type type)
  {
    if (!is_in_inventory_mode(actor, type)) {
      return 0;
    }

    int count = 0;
    const auto inventory = actor->GetInventory([](RE::TESBoundObject& object) {
      return object.GetFormType() == RE::FormType::AlchemyItem;
    });

    for (const auto& [item, inv_data] : inventory) {
      const auto& [item_count, entry] = inv_data;
      if (item_count <= 0 || !entry) {
        continue;
      }

      auto* potion = item ? item->As<RE::AlchemyItem>() : nullptr;
      const auto actual_count = get_actual_item_count(actor, potion, item_count);
      if (actual_count <= 0) {
        continue;
      }

      if (is_valid_inventory_use_potion(potion, settings, type)) {
        count += actual_count;
      }
    }

    return count;
  }

  int calculate_max_slots(RE::Actor* actor, const config::flask_settings_base& settings, const flask_type type)
  {
    
    if (is_in_inventory_mod_use(actor, type)) {
      return get_potions_count(actor, settings, type);
    }
    
    auto base = static_cast<float>(settings.cap_base);
    if (settings.cap_keyword) {
      // const auto effects = core::utility::get_active_effects_by_keyword(actor, settings.cap_keyword);
      // base += core::utility::get_magnitude_sum_of_active_effects(&effects);
      base += core::utility::get_sum_of_active_effects_magnitude_with_keyword(actor, settings.cap_keyword);
    }
    return static_cast<int>((std::max)(0.f, base));
  }

  float calculate_cooldown(RE::Actor* actor, const config::flask_settings_base& settings)
  {
    auto base = settings.cooldown_base;
    if (settings.cooldown_keyword) {
      // const auto effects = core::utility::get_active_effects_by_keyword(actor, settings.cooldown_keyword);
      // base += core::utility::get_magnitude_sum_of_active_effects(&effects);
      base += core::utility::get_sum_of_active_effects_magnitude_with_keyword(actor, settings.cooldown_keyword);
    }
    return (std::max)(0.f, base);
  }

  float calculate_regen_mult(RE::Actor* actor, const config::flask_settings_base& settings)
  {
    auto base = settings.regeneration_mult_base;
    if (settings.regeneration_mult_keyword) {
      // const auto effects = core::utility::get_active_effects_by_keyword(actor, settings.regeneration_mult_keyword);
      // base += core::utility::get_magnitude_sum_of_active_effects(&effects);
      base += core::utility::get_sum_of_active_effects_magnitude_with_keyword(actor, settings.regeneration_mult_keyword);
    }
    return (std::max)(0.f, base) / 100.0f;
  }

  float calculate_regen_mult_raw(RE::Actor* actor, const config::flask_settings_base& settings)
  {
    auto base = settings.regeneration_mult_base;
    if (settings.regeneration_mult_keyword) {
      // const auto effects = core::utility::get_active_effects_by_keyword(actor, settings.regeneration_mult_keyword);
      // base += core::utility::get_magnitude_sum_of_active_effects(&effects);
      base += core::utility::get_sum_of_active_effects_magnitude_with_keyword(actor, settings.regeneration_mult_keyword);
    }
    return (std::max)(0.f, base);
  }

  int count_available_flasks(RE::Actor* actor, core::actors_cache::cache_data::actor_data::flask_cooldown* flasks, const flask_type type, const int max_slots)
  {
    if (!flasks) return 0;

    int available = 0;
    const int limit = get_slot_limit(max_slots);
    if (is_in_inventory_mod_use(actor, type)) {
      auto settings = get_settings(config::config_manager::get_singleton(), type);
      auto potion_count = get_potions_count(actor, *settings, type);
      if (potion_count > limit) {
        return limit;
      }
      return potion_count;
    }

    for (const int i : std::views::iota(0, limit)) {
      if (flasks[i].cooldown_current <= 0.f) {
        available++;
      }
    }

    return available;
  }

  int count_recharging_flasks(core::actors_cache::cache_data::actor_data::flask_cooldown* flasks, const int max_slots)
  {
    if (!flasks) return 0;

    int recharging = 0;
    const int limit = get_slot_limit(max_slots);

    for (const int i : std::views::iota(0, limit)) {
      if (flasks[i].cooldown_current > 0.f) {
        recharging++;
      }
    }

    return recharging;
  }

  bool consume_flask_slots(RE::Actor* actor, core::actors_cache::cache_data::actor_data::flask_cooldown* flasks, const flask_type type,
                           const float cooldown_duration, const int max_slots, const int count)
  {
    if (!flasks || count <= 0) return false;

    const int limit = get_slot_limit(max_slots);
    if (limit <= 0 || count_available_flasks(actor, flasks, type, limit) < count) {
      return false;
    }
    
    if (is_in_inventory_mod_use(actor, type)) {
      const auto settings = get_settings(config::config_manager::get_singleton(), type);
      auto* potion = get_selected_inventory_potion(actor, *settings, type, false);
      if (!potion) {
        return false;
      }

      g_pending_inventory_drink = {
        .actor_id = actor->GetFormID(),
        .potion_id = potion->GetFormID(),
        .active = true
      };

      const auto drank = actor->DrinkPotion(potion, nullptr);
      if (g_pending_inventory_drink.active) {
        g_pending_inventory_drink.active = false;
      }
      return drank;
    }

    int consumed = 0;
    for (const int i : std::views::iota(0, limit)) {
      if (flasks[i].cooldown_current <= 0.f) {
        flasks[i].cooldown_start = cooldown_duration;
        flasks[i].cooldown_current = cooldown_duration;
        consumed++;

        if (consumed >= count) {
          return true;
        }
      }
    }

    return false;
  }

  bool restore_flask_slots(core::actors_cache::cache_data::actor_data::flask_cooldown* flasks, const int max_slots,
                           const int count)
  {
    if (!flasks || count <= 0) return false;

    const int limit = get_slot_limit(max_slots);
    if (limit <= 0 || count_recharging_flasks(flasks, limit) < count) {
      return false;
    }

    int restored = 0;
    while (restored < count) {
      int nearest_idx = -1;
      float min_cd = -1.f;

      for (const int i : std::views::iota(0, limit)) {
        if (flasks[i].cooldown_current > 0.f && (min_cd < 0.f || flasks[i].cooldown_current < min_cd)) {
          min_cd = flasks[i].cooldown_current;
          nearest_idx = i;
        }
      }

      if (nearest_idx < 0) {
        return false;
      }

      flasks[nearest_idx].cooldown_start = 0.f;
      flasks[nearest_idx].cooldown_current = 0.f;
      restored++;
    }

    return true;
  }

  core::actors_cache::cache_data::actor_data::flask_cooldown* get_flasks_array(
    core::actors_cache::cache_data::actor_data& data, const flask_type type)
  {
    switch (type) {
    case flask_type::Health: return data.flasks_health;
    case flask_type::Stamina: return data.flasks_stamina;
    case flask_type::Magick: return data.flasks_magick;
    case flask_type::Other: return data.flasks_others;
    }
    return nullptr;
  }

  // Forward declarations needed because API accessors are defined later
  export auto api_get_max_slots(RE::Actor* actor, const flask_type type) -> int;
  export auto api_get_current_slots(RE::Actor* actor, const flask_type type) -> int;
  
  auto get_potion_max_magnitude_with_keyword(RE::AlchemyItem* potion, const RE::BGSKeyword* keyword) -> float
  {
    auto result = 0.f;
    auto found = false;
    if (!potion || !keyword) {
      return 0.f;
    }
    
    for (const auto effect : potion->effects) {
      if (!effect || !effect->baseEffect || !effect->baseEffect->HasKeyword(keyword)) {
        continue;
      }

      const auto& data = effect->baseEffect->data;
      if (!data.flags.any(effect_flag::kRecover) || data.flags.any(effect_flag::kDetrimental)) {
        continue;
      }

      found = true;
      result = (std::max)(result, effect->GetMagnitude());
    }
    
    return found ? result : 0.f;
  }

  auto get_potion_restore_count_with_keyword(RE::AlchemyItem* potion, const RE::BGSKeyword* keyword) -> int
  {
    if (!potion || !keyword) {
      return 0;
    }

    auto magnitude = get_potion_max_magnitude_with_keyword(potion, keyword);
    if (magnitude > 0.f) {
      return (std::max)(1, static_cast<int>(magnitude));
    }

    for (const auto effect : potion->effects) {
      if (!effect || !effect->baseEffect || !effect->baseEffect->HasKeyword(keyword)) {
        continue;
      }

      const auto& data = effect->baseEffect->data;
      if (data.flags.any(effect_flag::kRecover) && !data.flags.any(effect_flag::kDetrimental)) {
        return 1;
      }
    }

    return 0;
  }
  
  auto get_av_by_flask_type(const flask_type type) -> RE::ActorValue
  {
    switch (type) {
    	case flask_type::Health:  return RE::ActorValue::kHealth;
    	case flask_type::Stamina: return RE::ActorValue::kStamina;
    	case flask_type::Magick:  return RE::ActorValue::kMagicka;
    	default:                  return RE::ActorValue::kNone;
    }
  }
  
  auto evalute_magnitude(RE::Effect* effect, RE::EffectSetting::EffectSettingData& data, const RE::ActorValue av) -> float
  {
    auto result = 0.f;
    if (!effect) {
      return result;
    }
    
    auto archetype = data.archetype;
    if (archetype == effect_archetype::kValueModifier && data.primaryAV == av) {
      result = effect->GetMagnitude();
    }

    if (archetype == effect_archetype::kDualValueModifier) {
      if (data.primaryAV == av) {
        result = effect->GetMagnitude();
      }
      if (data.secondaryAV == av) {
        result = result + (effect->GetMagnitude() * data.secondAVWeight);
      }
    }
    
    return result;
    
  }
  
  auto get_potion_max_magnitude_with_actor_value(RE::AlchemyItem* potion, const RE::ActorValue av) -> float
  {
    auto result = -1.f;
    if (!potion || av == RE::ActorValue::kNone) {
      return result;
    }
    
    for (const auto effect : potion->effects) {
      if (effect && effect->baseEffect) {
        auto& data = effect->baseEffect->data;
        if (!data.flags.any(effect_flag::kRecover) && !data.flags.any(effect_flag::kDetrimental)) {
          auto evalued_magnitude = evalute_magnitude(effect, data, av);
          if (result < evalued_magnitude) {
            result = evalued_magnitude;
          }
        }
      }
    }
    
    return result;
  }

  bool is_flask_potion(RE::AlchemyItem* potion)
  {
    if (!potion) {
      return false;
    }

    const auto* config = config::config_manager::get_singleton();
    if (!config) {
      return false;
    }

    if (config->main.no_remove_keyword && core::utility::try_form_has_keyword(potion, config->main.no_remove_keyword)) {
      return true;
    }

    if (config->flasks_health.keyword && core::utility::try_form_has_keyword(potion, config->flasks_health.keyword)) {
      return true;
    }
    if (config->flasks_stamina.keyword && core::utility::try_form_has_keyword(potion, config->flasks_stamina.keyword)) {
      return true;
    }
    if (config->flasks_magick.keyword && core::utility::try_form_has_keyword(potion, config->flasks_magick.keyword)) {
      return true;
    }

    return false;
  }

  bool is_valid_inventory_use_potion(RE::AlchemyItem* potion, const config::flask_settings_base& settings,
                                     const flask_type type)
  {
    if (!potion || !settings.inventory_keyword || type == flask_type::Other) {
      return false;
    }

    if (is_flask_potion(potion)) {
      return false;
    }

    if (!try_potion_has_keyword(potion, settings.inventory_keyword)) {
      return false;
    }

    return get_potion_max_magnitude_with_actor_value(potion, get_av_by_flask_type(type)) > 0.f;
  }

  bool is_valid_inventory_deposit_potion(RE::AlchemyItem* potion, const config::flask_settings_base& settings)
  {
    if (!potion || !settings.inventory_keyword) {
      return false;
    }

    if (is_flask_potion(potion)) {
      return false;
    }

    return get_potion_restore_count_with_keyword(potion, settings.inventory_keyword) > 0;
  }

  RE::AlchemyItem* get_selected_inventory_potion(RE::Actor* actor, const config::flask_settings_base& settings,
                                                 const flask_type type, const bool for_deposit)
  {
    if (!actor || !settings.inventory_keyword) {
      return nullptr;
    }

    const auto inventory = actor->GetInventory([](RE::TESBoundObject& object) {
      return object.GetFormType() == RE::FormType::AlchemyItem;
    });

    RE::AlchemyItem* selected = nullptr;
    float selected_magnitude = 0.f;

    for (const auto& [item, inv_data] : inventory) {
      const auto& [count, entry] = inv_data;
      if (count <= 0 || !entry) {
        continue;
      }

      auto* potion = item ? item->As<RE::AlchemyItem>() : nullptr;
      const auto actual_count = get_actual_item_count(actor, potion, count);
      if (actual_count <= 0) {
        continue;
      }

      const auto is_valid = for_deposit
                              ? is_valid_inventory_deposit_potion(potion, settings)
                              : is_valid_inventory_use_potion(potion, settings, type);
      if (!is_valid) {
        continue;
      }

      if (settings.inventory_select_mode_value == inventory_select_mode::first_found) {
        return potion;
      }

      const auto magnitude = for_deposit
                               ? static_cast<float>(get_potion_restore_count_with_keyword(potion, settings.inventory_keyword))
                               : get_potion_max_magnitude_with_actor_value(potion, get_av_by_flask_type(type));
      if (!selected) {
        selected = potion;
        selected_magnitude = magnitude;
        continue;
      }

      if (settings.inventory_select_mode_value == inventory_select_mode::weakest_first) {
        if (magnitude < selected_magnitude) {
          selected = potion;
          selected_magnitude = magnitude;
        }
      } else if (magnitude > selected_magnitude) {
        selected = potion;
        selected_magnitude = magnitude;
      }
    }

    return selected;
  }

  bool consume_pending_inventory_drink(RE::Actor* actor, const RE::AlchemyItem* potion)
  {
    if (!g_pending_inventory_drink.active || !actor || !potion) {
      return false;
    }

    const auto is_match = g_pending_inventory_drink.actor_id == actor->GetFormID() &&
                          g_pending_inventory_drink.potion_id == potion->GetFormID();
    if (is_match) {
      g_pending_inventory_drink.active = false;
      return true;
    }

    return false;
  }
  
  bool consume_flask_slot(const flask_type type, RE::Actor* actor, const int count)
  {
    if (!actor || !is_valid_flask_type(type) || count <= 0) {
      return false;
    }
    
    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(actor->GetFormID());
    const auto settings = get_settings(config::config_manager::get_singleton(), type);
    if (!settings) {
      return false;
    }
    
    const auto max_slots = calculate_max_slots(actor, *settings, type);
    const auto cooldown = calculate_cooldown(actor, *settings);

    auto flasks = get_flasks_array(actor_data, type);

    if (consume_flask_slots(actor, flasks, type, cooldown, max_slots, count)) {
      logger::info("Consumed {} flask slot(s): type {}, cooldown {:.1f}, slots {}/{}", count,
                   static_cast<int>(type), cooldown,
                   api_get_current_slots(actor, type), max_slots);
      return true;
    }
    
    return false;
  }
  
  bool restore_flask_slot(const flask_type type, RE::Actor* actor, const int count)
  {
    if (!actor || !is_valid_flask_type(type) || count <= 0) {
      return false;
    }
    
    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(actor->GetFormID());
    const auto settings = get_settings(config::config_manager::get_singleton(), type);
    if (!settings) {
      return false;
    }

    const auto max_slots = calculate_max_slots(actor, *settings, type);
    auto flasks = get_flasks_array(actor_data, type);

    if (restore_flask_slots(flasks, max_slots, count)) {
      logger::info("Restored {} flask slot(s): type {}, slots {}/{}", count, static_cast<int>(type),
                   api_get_current_slots(actor, type), max_slots);
      return true;
    }
    
    return false;
  }

  void try_inventory_deposit(RE::Actor* actor, core::actors_cache::cache_data::actor_data& actor_data,
                             const config::flask_settings_base& settings, const flask_type type)
  {
    if (!actor) {
      return;
    }

    const auto max_slots = calculate_max_slots(actor, settings, type);
    if (max_slots <= 0) {
      return;
    }

    auto flasks = get_flasks_array(actor_data, type);
    if (!flasks) {
      return;
    }

    auto current_slots = count_available_flasks(actor, flasks, type, max_slots);
    while (current_slots < max_slots) {
      auto* potion = get_selected_inventory_potion(actor, settings, type, true);
      if (!potion) {
        break;
      }

      const auto restore_count = get_potion_restore_count_with_keyword(potion, settings.inventory_keyword);
      if (restore_count <= 0) {
        break;
      }

      const auto restore_amount = (std::min)(max_slots - current_slots, restore_count);
      if (!restore_flask_slots(flasks, max_slots, restore_amount)) {
        break;
      }

      logger::info("Inventory deposit restored {} slot(s): type {}, potion {}, missing_before {}",
                   restore_amount, static_cast<int>(type), potion->GetName(), max_slots - current_slots);
      current_slots += restore_amount;
      actor->RemoveItem(potion, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
    }
  }

  export bool drink_potion(const core::hooks_ctx::on_actor_drink_potion& ctx)
  {
    logger::info("DrinkPotion: Character -> {} Potion -> {} IsFood -> {} IsPoison -> {}",
                 ctx.actor->GetDisplayFullName(), ctx.potion->GetFullName(), ctx.potion->IsFood(),
                 ctx.potion->IsPoison());

    if (ctx.potion->IsFood() || ctx.potion->IsPoison()) {
      return true;
    }

    if (consume_pending_inventory_drink(ctx.actor, ctx.potion)) {
      return true;
    }

    const auto config = config::config_manager::get_singleton();
    const auto type_opt = identify_flask_type(ctx.potion, config);

    if (!type_opt.has_value()) {
      logger::info("Potion not identified as flask: {}", ctx.potion->GetName());
      return true;
    }

    const auto type = type_opt.value();
    const auto settings = get_settings(config, type);

    if (!settings->enable) {
      logger::info("Flask type {} disabled", static_cast<int>(type));
      return true;
    }
    
    if (is_in_inventory_mode(ctx.actor, type) && try_potion_has_keyword(ctx.potion, settings->inventory_keyword)) {
      logger::info("Flask in_inventory_mode consumed ", ctx.potion->GetName());
      return true;
    }

    const auto is_player = core::utility::is_player(ctx.actor);
    if (is_player && !settings->player) return true;
    if (!is_player && !settings->npc) return true;

    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(ctx.actor->GetFormID());

    if (settings->anti_spam && actor_data.anti_spam_durations[static_cast<int>(type)] > 0.f) {
      logger::info("Anti-spam blocked drink for actor {:08X}", ctx.actor->GetFormID());
      return false;
    }
    
    if (consume_flask_slot(type, ctx.actor, 1)) {
      if (settings->anti_spam) {
        actor_data.anti_spam_durations[static_cast<int>(type)] = settings->anti_spam_delay;
      }
      return true;
    }

    if (is_player) {
      if (!settings->notify.empty()) {
        RE::SendHUDMessage::ShowHUDMessage(settings->notify.c_str());
      }
      logger::info("Player fail drink flask type -> {}", static_cast<int>(type));
      if (settings->fail_audio && settings->fail_audio_form) {
        // RE::PlaySound(core::utility::get_editor_id(settings->fail_audio_form));
        core::utility::game::try_play_sound_at(ctx.actor, settings->fail_audio_form);
      }
      // Trigger glow in UI via flag
      actor_data.failed_drink_types[static_cast<int>(type)] = true;
      logger::info("Flask usage failed (no slots): type {}", static_cast<int>(type));
    }

    return false;
  }

  export void update(const core::hooks_ctx::on_actor_update& ctx)
  {
    
    const auto config = config::config_manager::get_singleton();
    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(ctx.actor->GetFormID());

    auto d_data = core::actors_cache::cache_data::actor_data::delta_data{};
    d_data.delta = ctx.delta;

    auto calc_delta = [&](const flask_type type) {
      const auto settings = get_settings(config, type);
      const auto mult = calculate_regen_mult(ctx.actor, *settings);
      return ctx.delta * mult;
    };

    d_data.delta_health = calc_delta(flask_type::Health);
    d_data.delta_stamina = calc_delta(flask_type::Stamina);
    d_data.delta_magick = calc_delta(flask_type::Magick);
    d_data.delta_other = calc_delta(flask_type::Other);

    d_data.parallel_health = config->flasks_health.enable_parallel_cooldown;
    d_data.parallel_stamina = config->flasks_stamina.enable_parallel_cooldown;
    d_data.parallel_magick = config->flasks_magick.enable_parallel_cooldown;
    d_data.parallel_other = config->flasks_other.enable_parallel_cooldown;


    actor_data.update(d_data);
  }
  
  export void update_1s(const core::hooks_ctx::on_actor_update& ctx)
  {
    
    const auto config = config::config_manager::get_singleton();
    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(ctx.actor->GetFormID());

    for (auto type : kFlaskTypes) {
      if (is_in_inventory_mode_deposit(ctx.actor, type)) {
        const auto settings = get_settings(config, type);
        if (settings) {
          try_inventory_deposit(ctx.actor, actor_data, *settings, type);
        }
      }
    }
  }
  
  export void update_ui(const core::hooks_ctx::on_actor_update& ctx)
  {
    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(ctx.actor->GetFormID());
    
    const auto flask_glow_callbacks = api::mod_api::get_play_flasks_glow_callbacks();
    
    for (int i : std::views::iota(0, core::actors_cache::cache_data::actor_data::FLASK_TYPE_SIZE)) {
      
      if (actor_data.failed_drink_types[i]) {
        
        const auto type = static_cast<TrueFlasksAPI::FlaskType>(i);
        for (const auto& flask_glow_call : *flask_glow_callbacks | std::views::values) {
          flask_glow_call(type);
        }
        
        actor_data.failed_drink_types[i] = false;
      }
    }
    
  }

  export void remove_item(core::hooks_ctx::on_actor_remove_item& ctx)
  {
    if (ctx.reason != RE::ITEM_REMOVE_REASON::kRemove) return;
    if (ctx.move_to_ref || ctx.drop_loc || ctx.rotate) return;
    if (ctx.count <= 0 || !ctx.item) return;

    const auto potion = ctx.item->As<RE::AlchemyItem>();
    if (!potion) return;

    const auto config = config::config_manager::get_singleton();

    // Check for NoRemoveKeyword
    if (config->main.no_remove_keyword && core::utility::try_form_has_keyword(potion, config->main.no_remove_keyword)) {
      const auto type_opt = identify_flask_type(potion, config);
      if (!type_opt.has_value()) return;

      const auto type = type_opt.value();
      const auto settings = get_settings(config, type);

      const auto is_player = core::utility::is_player(ctx.actor);
      logger::info(
        "Checking flask for removal: type {}, settings enable: {}, player: {}, npc: {}, IsPlayer: {}, FormID {}",
        static_cast<int>(type), settings->enable, settings->player, settings->npc, is_player, ctx.actor->GetFormID());

      if (!settings->enable) return;

      if (is_player && !settings->player) return;
      if (!is_player && !settings->npc) return;

      // If we reached here, it's a flask that should not be removed
      ctx.count = 0;
      logger::info("Prevented removal of flask: {} from actor: {:08X}", potion->GetName(), ctx.actor->GetFormID());
    }
  }
  
  export auto on_input_event(const events::events_ctx::process_event_input_ctx& ctx) 
  {
    if (!ctx.button_event || !ctx.button_event->IsDown()) {
      return;
    }

    auto player = RE::PlayerCharacter::GetSingleton();
    auto equip_manager = RE::ActorEquipManager::GetSingleton();
    auto config = config::config_manager::get_singleton();
    if (!player || !equip_manager || !config) {
      return;
    }

    if (core::utility::is_any_menu_open() || player->IsDead() || player->IsInKillMove()) {
      return;
    }

    const auto get_input_device = [&](const RE::INPUT_DEVICE device) -> RE::BSInputDevice* {
      auto input_manager = RE::BSInputDeviceManager::GetSingleton();
      if (!input_manager) {
        return nullptr;
      }

      switch (device) {
      case RE::INPUT_DEVICE::kGamepad:
        return input_manager->GetGamepad();
      case RE::INPUT_DEVICE::kKeyboard:
        return input_manager->GetKeyboard();
      default: 
        return nullptr;
      }
    };

    const auto matches_binding = [&](const config::flask_settings& settings) {
      if (ctx.device == RE::INPUT_DEVICE::kGamepad) {
        if (settings.gamepad_hotkey == 0) {
          return false;
        }

        if (settings.gamepad_hotkey_modifier == 0) {
          return ctx.key == settings.gamepad_hotkey;
        }

        auto input_device = get_input_device(ctx.device);
        return input_device && ctx.key == settings.gamepad_hotkey_modifier &&
               input_device->IsPressed(settings.gamepad_hotkey);
      }

      if (settings.hotkey == 0) {
        return false;
      }

      if (settings.hotkey_modifier == 0) {
        return ctx.key == settings.hotkey;
      }

      auto* input_device = get_input_device(ctx.device);
      return input_device && ctx.key == settings.hotkey_modifier &&
             input_device->IsPressed(settings.hotkey);
    };

    const auto try_drink = [&](const config::flask_settings& settings) {
      if (!matches_binding(settings) || !settings.enable || !settings.player || !settings.keyword) {
        return false;
      }

      auto potion = core::utility::game::get_object_in_inventory_by_keyword(
        player, settings.keyword, RE::FormType::AlchemyItem);
      if (!potion) {
        return false;
      }

      equip_manager->EquipObject(player, potion);
      return true;
    };

    if (try_drink(config->flasks_health)) {
      return;
    }

    if (try_drink(config->flasks_stamina)) {
      return;
    }

    try_drink(config->flasks_magick);
  }

  // API Functions

  export auto api_get_max_slots(RE::Actor* actor, const flask_type type) -> int
  {
    if (!actor) return 0;
    const auto config = config::config_manager::get_singleton();
    const auto settings = get_settings(config, type);
    if (!settings) return 0;
    return calculate_max_slots(actor, *settings, type);
  }

  export auto api_get_current_slots(RE::Actor* actor, const flask_type type) -> int
  {
    if (!actor) return 0;

    const auto max_slots = api_get_max_slots(actor, type);
    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(actor->GetFormID());
    auto flasks = get_flasks_array(actor_data, type);

    if (!flasks) return 0;

    int available = 0;
    const int limit = get_slot_limit(max_slots);
    
    auto settings = get_settings(config::config_manager::get_singleton(), type);
    if (is_in_inventory_mod_use(actor, type)) {
      auto potion_count = get_potions_count(actor, *settings, type);
      if (potion_count > limit) {
        return limit;
      }
      return potion_count;
    }

    for (const int i : std::views::iota(0, limit)) {
      if (flasks[i].cooldown_current <= 0.f) {
        available++;
      }
    }
    return available;
  }

  export auto api_get_next_cooldown(RE::Actor* actor, const flask_type type) -> float
  {
    if (!actor) return 0.f;
    
    if (is_in_inventory_mod_use(actor, type)) {
      return 0.f;
    }

    const auto max_slots = api_get_max_slots(actor, type);
    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(actor->GetFormID());
    auto flasks = get_flasks_array(actor_data, type);

    if (!flasks) return 0.f;

    float min_cd = -1.f;
    const int limit = get_slot_limit(max_slots);

    for (const int i : std::views::iota(0, limit)) {
      if (flasks[i].cooldown_current > 0.f) {
        if (min_cd < 0.f || flasks[i].cooldown_current < min_cd) {
          min_cd = flasks[i].cooldown_current;
        }
      }
    }
    return (std::max)(0.f, min_cd);
  }

  export auto api_can_regenerate(RE::Actor* actor, const flask_type type) -> bool
  {
    if (!actor) return false;

    const auto config = config::config_manager::get_singleton();
    const auto settings = get_settings(config, type);
    if (!settings) return false;

    return calculate_regen_mult_raw(actor, *settings) > 0.0f;
  }

  export auto api_modify_cooldown(RE::Actor* actor, const flask_type type, const float amount,
                                  const bool all_slots) -> void
  {
    if (!actor) return;

    const auto max_slots = api_get_max_slots(actor, type);
    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(actor->GetFormID());
    auto flasks = get_flasks_array(actor_data, type);

    if (!flasks) return;

    const int limit = get_slot_limit(max_slots);
    
    const bool is_restore_flask = amount < 0;

    if (all_slots) {
      for (const int i : std::views::iota(0, limit)) {
        
        if (flasks[i].cooldown_current > 0.f && is_restore_flask) {
          flasks[i].cooldown_current = (std::max)(0.f, flasks[i].cooldown_current + amount);
        } else {
          flasks[i].cooldown_current = (std::max)(0.f, flasks[i].cooldown_current + amount);
        }
      }
    }
    else {
      // Find nearest cooldown
      int nearest_idx = -1;
      float min_cd = -1.f;

      for (const int i : std::views::iota(0, limit)) {
        if (flasks[i].cooldown_current > 0.f && is_restore_flask) {
          if (min_cd < 0.f || flasks[i].cooldown_current < min_cd) {
            min_cd = flasks[i].cooldown_current;
            nearest_idx = i;
          }
        } else {
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

  export auto api_get_regen_mult(RE::Actor* actor, const flask_type type) -> float
  {
    if (!actor) return 0.f;
    const auto config = config::config_manager::get_singleton();
    const auto settings = get_settings(config, type);
    if (!settings) return 0.f;
    return calculate_regen_mult_raw(actor, *settings);
  }

  export auto api_get_cooldown_pct(RE::Actor* actor, const flask_type type) -> float
  {
    if (!actor) return 1.0f;
    
    if (is_in_inventory_mod_use(actor, type)) {
      return 1.f;
    }

    const auto max_slots = api_get_max_slots(actor, type);
    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(actor->GetFormID());
    auto flasks = get_flasks_array(actor_data, type);

    if (!flasks) return 1.0f;

    int nearest_idx = -1;
    float min_cd = -1.f;
    const int limit = get_slot_limit(max_slots);

    for (const int i : std::views::iota(0, limit)) {
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

  export auto api_get_flask_info(RE::AlchemyItem* potion) -> std::pair<int, bool>
  {
    const auto config = config::config_manager::get_singleton();
    const auto type_opt = identify_flask_type(potion, config);

    if (!type_opt.has_value()) {
      return {-1, false};
    }

    return {static_cast<int>(type_opt.value()), true};
  }
  
  export auto api_play_flask_glow(RE::Actor* actor, const flask_type type) -> void
  {
    if (!actor || !is_valid_flask_type(type)) return;
    auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(actor->GetFormID());
    actor_data.failed_drink_types[static_cast<int>(type)] = true;
  }
  
  export auto api_consume_flask_slot(RE::Actor* actor, const flask_type type, const int count) -> bool
  {
    if (!actor || !is_valid_flask_type(type)) return false;
    return consume_flask_slot(type, actor, std::abs(count));
  }
  
  export auto api_restore_flask_slot(RE::Actor* actor, const flask_type type, const int count) -> bool
  {
    if (!actor || !is_valid_flask_type(type)) return false;
    return restore_flask_slot(type, actor, std::abs(count));
  }

  export auto api_get_flask_settings(const flask_type type) -> std::optional<TrueFlasksAPI::FlaskSettings>
  {
    const auto config = config::config_manager::get_singleton();
    const auto settings = get_settings(config, type);

    if (!settings) return std::nullopt;

    TrueFlasksAPI::FlaskSettings out;
    out.enable = settings->enable;
    out.npc = settings->npc;
    out.player = settings->player;
    out.enable_parallel_cooldown = settings->enable_parallel_cooldown;
    out.anti_spam = settings->anti_spam;
    out.anti_spam_delay = settings->anti_spam_delay;
    out.regeneration_mult_base = settings->regeneration_mult_base;
    out.regeneration_mult_keyword = settings->regeneration_mult_keyword;
    out.cap_base = settings->cap_base;
    out.cap_keyword = settings->cap_keyword;
    out.cooldown_base = settings->cooldown_base;
    out.cooldown_keyword = settings->cooldown_keyword;
    out.fail_audio = settings->fail_audio;
    out.fail_audio_form = settings->fail_audio_form;

    // Fill specific fields
    out.keyword = nullptr;
    out.exclusive_keyword = nullptr;
    out.revert_exclusive = false;

    if (type == flask_type::Other) {
      out.exclusive_keyword = config->flasks_other.exclusive_keyword;
      out.revert_exclusive = config->flasks_other.revert_exclusive;
    }
    else {
      // For Health, Stamina, Magick, we need to cast back to flask_settings to get the keyword
      // Since get_settings returns base pointer, we can do a safe cast or just access directly from config
      switch (type) {
      case flask_type::Health: out.keyword = config->flasks_health.keyword;
        break;
      case flask_type::Stamina: out.keyword = config->flasks_stamina.keyword;
        break;
      case flask_type::Magick: out.keyword = config->flasks_magick.keyword;
        break;
      default: break;
      }
    }

    return out;
  }
}
