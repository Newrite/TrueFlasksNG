#include "API/TrueFlasksAPI.h"

import TrueFlasks.Features.TrueFlasks;
import TrueFlasks.API.ModAPI;

namespace api
{
  using namespace TrueFlasksAPI;

  class TrueFlasksAPI : public IVTrueFlasks2
  {
  public:
    void ModifyCooldown(RE::Actor* actor, FlaskType type, float amount, bool all_slots) noexcept override
    {
      features::true_flasks::api_modify_cooldown(actor, static_cast<features::true_flasks::flask_type>(type), amount,
                                                 all_slots);
    }

    float GetNextCooldown(RE::Actor* actor, FlaskType type) noexcept override
    {
      return features::true_flasks::api_get_next_cooldown(actor, static_cast<features::true_flasks::flask_type>(type));
    }

    int GetMaxSlots(RE::Actor* actor, FlaskType type) noexcept override
    {
      return features::true_flasks::api_get_max_slots(actor, static_cast<features::true_flasks::flask_type>(type));
    }

    int GetCurrentSlots(RE::Actor* actor, FlaskType type) noexcept override
    {
      return features::true_flasks::api_get_current_slots(actor, static_cast<features::true_flasks::flask_type>(type));
    }

    float GetRegenMult(RE::Actor* actor, FlaskType type) noexcept override
    {
      return features::true_flasks::api_get_regen_mult(actor, static_cast<features::true_flasks::flask_type>(type));
    }

    float GetCooldownPct(RE::Actor* actor, FlaskType type) noexcept override
    {
      return features::true_flasks::api_get_cooldown_pct(actor, static_cast<features::true_flasks::flask_type>(type));
    }

    std::pair<int, bool> GetFlaskInfo(RE::AlchemyItem* potion) noexcept override
    {
      return features::true_flasks::api_get_flask_info(potion);
    }

    void PlayFlaskGlow(RE::Actor* actor, FlaskType type) noexcept override
    {
      features::true_flasks::api_play_flask_glow(actor, static_cast<features::true_flasks::flask_type>(type));
    }

    std::optional<FlaskSettings> GetFlaskSettings(FlaskType type) noexcept override
    {
        return features::true_flasks::api_get_flask_settings(static_cast<features::true_flasks::flask_type>(type));
    }

    APIResult AddPlayFlaskGlowCallback(SKSE::PluginHandle plugin_handle,
      PlayFlaskGlowCallback glow_callback) noexcept override
    {
      auto callbacks = mod_api::get_play_flasks_glow_callbacks();
      if (callbacks->contains(plugin_handle)) {
        return APIResult::AlreadyRegistered;
      }
      callbacks->emplace(plugin_handle, glow_callback);
      return APIResult::OK;
    }
    
    APIResult RemovePlayFlaskGlowCallback(SKSE::PluginHandle plugin_handle) noexcept override
    {
      auto callbacks = mod_api::get_play_flasks_glow_callbacks();
      if (!callbacks->contains(plugin_handle)) {
        return APIResult::NotRegistered;
      }
      callbacks->erase(plugin_handle);
      return APIResult::OK;
    }
    
    bool ConsumeFlaskSlot(RE::Actor* actor, FlaskType type) noexcept override
    {
      return features::true_flasks::api_consume_flask_slot(actor, static_cast<features::true_flasks::flask_type>(type));
    }
    
  };
  
  static TrueFlasksAPI g_TrueFlasksAPI;

  extern "C" __declspec(dllexport) void* RequestPluginAPI(const InterfaceVersion interfaceVersion)
  {
    switch (interfaceVersion) {
    case InterfaceVersion::V1:
    case InterfaceVersion::V2:
      return &g_TrueFlasksAPI;
    default:
      return nullptr;
    }
  }
}
