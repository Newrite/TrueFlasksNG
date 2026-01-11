#pragma once

#include <stdint.h>
#include <Windows.h>
#include <utility>
#include <optional>

namespace TrueFlasksAPI
{
  constexpr const auto TrueFlasksPluginName = "TrueFlasksNG";

  enum class InterfaceVersion : uint8_t
  {
    V1
  };

  enum class FlaskType : uint8_t
  {
    Health = 0,
    Stamina = 1,
    Magick = 2,
    Other = 3
  };

  struct FlaskSettings
  {
      bool enable;
      bool npc;
      bool player;
      bool enable_parallel_cooldown;
      bool anti_spam;
      float anti_spam_delay;

      float regeneration_mult_base;
      RE::BGSKeyword* regeneration_mult_keyword;
      
      int cap_base;
      RE::BGSKeyword* cap_keyword;

      float cooldown_base;
      RE::BGSKeyword* cooldown_keyword;

      // Specifics (nullptr/false if not applicable)
      RE::BGSKeyword* keyword; 
      RE::BGSKeyword* exclusive_keyword;
      bool revert_exclusive;
  };

  class IVTrueFlasks1
  {
  public:
    virtual void ModifyCooldown(RE::Actor* actor, FlaskType type, float amount, bool all_slots) noexcept = 0;
    virtual float GetNextCooldown(RE::Actor* actor, FlaskType type) noexcept = 0;
    virtual int GetMaxSlots(RE::Actor* actor, FlaskType type) noexcept = 0;
    virtual int GetCurrentSlots(RE::Actor* actor, FlaskType type) noexcept = 0;
    virtual float GetRegenMult(RE::Actor* actor, FlaskType type) noexcept = 0;
    virtual float GetCooldownPct(RE::Actor* actor, FlaskType type) noexcept = 0;
    virtual std::pair<int, bool> GetFlaskInfo(RE::AlchemyItem* potion) noexcept = 0;
    virtual void PlayFlaskGlow(RE::Actor* actor, FlaskType type) noexcept = 0;
    virtual std::optional<FlaskSettings> GetFlaskSettings(FlaskType type) noexcept = 0;
  };

  typedef void* (*_RequestPluginAPI)(const InterfaceVersion interfaceVersion);

  [[nodiscard]] inline void* RequestPluginAPI(const InterfaceVersion a_interfaceVersion = InterfaceVersion::V1)
  {
    auto pluginHandle = GetModuleHandleA("TrueFlasksNG.dll");
    _RequestPluginAPI requestAPIFunction = (_RequestPluginAPI)GetProcAddress(pluginHandle, "RequestPluginAPI");

    if (requestAPIFunction) {
      return requestAPIFunction(a_interfaceVersion);
    }

    return nullptr;
  }
}
