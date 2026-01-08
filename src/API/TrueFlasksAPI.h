#pragma once

#include <stdint.h>
#include <Windows.h>
#include <utility>

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
