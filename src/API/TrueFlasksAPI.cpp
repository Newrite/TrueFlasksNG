#include "API/TrueFlasksAPI.h"

import TrueFlasks.Features.TrueFlasks;

namespace api
{
  using namespace TrueFlasksAPI;

  class TrueFlasksAPI_V1 : public IVTrueFlasks1
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
  };

  static TrueFlasksAPI_V1 g_TrueFlasksAPI_V1;

  extern "C" __declspec(dllexport) void* RequestPluginAPI(const InterfaceVersion interfaceVersion)
  {
    switch (interfaceVersion) {
    case InterfaceVersion::V1:
      return &g_TrueFlasksAPI_V1;
    default:
      return nullptr;
    }
  }
}
