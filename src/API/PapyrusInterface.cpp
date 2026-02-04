module;

#include "API/TrueFlasksAPI.h"
#include <vector>

export module TrueFlasks.Papyrus;

import TrueFlasks.Features.TrueFlasks;

namespace papyrus
{
  using namespace features::true_flasks;

  // Helper to convert int to flask_type
  flask_type int_to_flask_type(int type)
  {
    switch (type) {
    case 0: return flask_type::Health;
    case 1: return flask_type::Stamina;
    case 2: return flask_type::Magick;
    default: return flask_type::Other;
    }
  }

  void ModifyCooldown(RE::StaticFunctionTag*, RE::Actor* actor, int type, float amount, bool all_slots)
  {
    if (!actor) return;
    api_modify_cooldown(actor, int_to_flask_type(type), amount, all_slots);
  }

  float GetNextCooldown(RE::StaticFunctionTag*, RE::Actor* actor, int type)
  {
    if (!actor) return 0.0f;
    return api_get_next_cooldown(actor, int_to_flask_type(type));
  }

  int GetMaxSlots(RE::StaticFunctionTag*, RE::Actor* actor, int type)
  {
    if (!actor) return 0;
    return api_get_max_slots(actor, int_to_flask_type(type));
  }

  int GetCurrentSlots(RE::StaticFunctionTag*, RE::Actor* actor, int type)
  {
    if (!actor) return 0;
    return api_get_current_slots(actor, int_to_flask_type(type));
  }

  float GetRegenMult(RE::StaticFunctionTag*, RE::Actor* actor, int type)
  {
    if (!actor) return 0.0f;
    return api_get_regen_mult(actor, int_to_flask_type(type));
  }

  float GetCooldownPct(RE::StaticFunctionTag*, RE::Actor* actor, int type)
  {
    if (!actor) return 1.0f;
    return api_get_cooldown_pct(actor, int_to_flask_type(type));
  }

  std::vector<int> GetFlaskInfo(RE::StaticFunctionTag*, RE::AlchemyItem* potion)
  {
    if (!potion) return {-1, 0};
    auto [type, has_slot] = api_get_flask_info(potion);
    return {type, has_slot ? 1 : 0};
  }

  void PlayFlaskGlow(RE::StaticFunctionTag*, RE::Actor* actor, int type)
  {
    if (!actor) return;
    api_play_flask_glow(actor, int_to_flask_type(type));
  }
  
  bool ConsumeFlaskSlot(RE::StaticFunctionTag*, RE::Actor* actor, int type)
  {
    if (!actor) return false;
    return api_consume_flask_slot(actor, int_to_flask_type(type));
  }

  export bool Register(RE::BSScript::IVirtualMachine* vm)
  {
    vm->RegisterFunction("ModifyCooldown", "TrueFlasksNG", ModifyCooldown);
    vm->RegisterFunction("GetNextCooldown", "TrueFlasksNG", GetNextCooldown);
    vm->RegisterFunction("GetMaxSlots", "TrueFlasksNG", GetMaxSlots);
    vm->RegisterFunction("GetCurrentSlots", "TrueFlasksNG", GetCurrentSlots);
    vm->RegisterFunction("GetRegenMult", "TrueFlasksNG", GetRegenMult);
    vm->RegisterFunction("GetCooldownPct", "TrueFlasksNG", GetCooldownPct);
    vm->RegisterFunction("GetFlaskInfo", "TrueFlasksNG", GetFlaskInfo);
    vm->RegisterFunction("PlayFlaskGlow", "TrueFlasksNG", PlayFlaskGlow);
    vm->RegisterFunction("ConsumeFlaskSlot", "TrueFlasksNG", ConsumeFlaskSlot);
    return true;
  }
}
