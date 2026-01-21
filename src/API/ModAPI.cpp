module;

#include "TrueFlasksAPI.h"

export module TrueFlasks.API.ModAPI;

namespace api::mod_api
{
    using PlayFlaskGlowCallback = TrueFlasksAPI::PlayFlaskGlowCallback;
  
    export auto get_play_flasks_glow_callbacks() -> std::map<SKSE::PluginHandle, PlayFlaskGlowCallback>*
    {
      static std::map<SKSE::PluginHandle, PlayFlaskGlowCallback> callbacks{};
      return std::addressof(callbacks);
    }
  
}
