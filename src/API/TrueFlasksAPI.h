#pragma once

namespace TrueFlasksAPI
{
  /// <summary>
  /// The name of the plugin.
  /// </summary>
  constexpr const auto TrueFlasksPluginName = "TrueFlasksNG";

  /// <summary>
  /// Supported interface versions.
  /// </summary>
  enum class InterfaceVersion : uint8_t
  {
    V1
  };

  /// <summary>
  /// Enumeration of available flask types.
  /// </summary>
  enum class FlaskType : uint8_t
  {
    Health = 0,
    Stamina = 1,
    Magick = 2,
    Other = 3
  };

  /// <summary>
  /// Result codes for API operations.
  /// </summary>
  enum class APIResult : uint8_t
  {
    OK = 0,
    AlreadyRegistered = 1,
    NotRegistered = 2,
  };

  /// <summary>
  /// Structure containing configuration settings for a specific flask type.
  /// </summary>
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
    
      bool fail_audio;
      std::string fail_audio_edid;

      // Specifics (nullptr/false if not applicable)
      RE::BGSKeyword* keyword; 
      RE::BGSKeyword* exclusive_keyword;
      bool revert_exclusive;
  };

  /// <summary>
  /// Callback function signature for handling flask glow events.
  /// Parameters:
  ///   - type: The type of flask.
  /// </summary>
  using PlayFlaskGlowCallback = std::function<void(FlaskType)>;

  /// <summary>
  /// Interface for interacting with the TrueFlasksNG plugin.
  /// </summary>
  class IVTrueFlasks1
  {
  public:
    /// <summary>
    /// Modifies the cooldown of a specific flask type for an actor.
    /// </summary>
    /// <param name="actor">The target actor.</param>
    /// <param name="type">The type of flask to modify.</param>
    /// <param name="amount">The amount of time (in seconds) to reduce the cooldown by (positive values reduce cooldown).</param>
    /// <param name="all_slots">If true, modifies all slots; otherwise, modifies the slot with the lowest cooldown.</param>
    virtual void ModifyCooldown(RE::Actor* actor, FlaskType type, float amount, bool all_slots) noexcept = 0;

    /// <summary>
    /// Gets the remaining cooldown time for the next available slot of a specific flask type.
    /// </summary>
    /// <param name="actor">The target actor.</param>
    /// <param name="type">The flask type.</param>
    /// <returns>Remaining cooldown in seconds.</returns>
    virtual float GetNextCooldown(RE::Actor* actor, FlaskType type) noexcept = 0;

    /// <summary>
    /// Gets the maximum number of slots for a specific flask type.
    /// </summary>
    /// <param name="actor">The target actor.</param>
    /// <param name="type">The flask type.</param>
    /// <returns>Maximum number of slots.</returns>
    virtual int GetMaxSlots(RE::Actor* actor, FlaskType type) noexcept = 0;

    /// <summary>
    /// Gets the number of currently available (ready to use) slots for a specific flask type.
    /// </summary>
    /// <param name="actor">The target actor.</param>
    /// <param name="type">The flask type.</param>
    /// <returns>Number of available slots.</returns>
    virtual int GetCurrentSlots(RE::Actor* actor, FlaskType type) noexcept = 0;

    /// <summary>
    /// Gets the regeneration multiplier for a specific flask type.
    /// </summary>
    /// <param name="actor">The target actor.</param>
    /// <param name="type">The flask type.</param>
    /// <returns>Regeneration multiplier.</returns>
    virtual float GetRegenMult(RE::Actor* actor, FlaskType type) noexcept = 0;

    /// <summary>
    /// Gets the cooldown percentage (0.0 to 1.0) for the current recharging slot.
    /// </summary>
    /// <param name="actor">The target actor.</param>
    /// <param name="type">The flask type.</param>
    /// <returns>Cooldown percentage (1.0 means fully charged).</returns>
    virtual float GetCooldownPct(RE::Actor* actor, FlaskType type) noexcept = 0;

    /// <summary>
    /// Identifies if a potion is a flask and returns its type and slot consumption status.
    /// </summary>
    /// <param name="potion">The alchemy item to check.</param>
    /// <returns>A pair containing the FlaskType (as int) and a boolean indicating if it consumes a slot.</returns>
    virtual std::pair<int, bool> GetFlaskInfo(RE::AlchemyItem* potion) noexcept = 0;

    /// <summary>
    /// Triggers a visual glow effect for the specified flask type on the HUD.
    /// </summary>
    /// <param name="actor">The target actor.</param>
    /// <param name="type">The flask type.</param>
    virtual void PlayFlaskGlow(RE::Actor* actor, FlaskType type) noexcept = 0;

    /// <summary>
    /// Retrieves the current settings for a specific flask type.
    /// </summary>
    /// <param name="type">The flask type.</param>
    /// <returns>Optional containing FlaskSettings if successful.</returns>
    virtual std::optional<FlaskSettings> GetFlaskSettings(FlaskType type) noexcept = 0;

    /// <summary>
    /// Registers a callback to be executed when a flask glow event occurs (e.g. via PlayFlaskGlow).
    /// </summary>
    /// <param name="plugin_handle">The SKSE plugin handle of the registering plugin.</param>
    /// <param name="glow_callback">The callback function to execute.</param>
    /// <returns>APIResult::OK if registered successfully.</returns>
    virtual APIResult AddPlayFlaskGlowCallback(SKSE::PluginHandle plugin_handle, PlayFlaskGlowCallback glow_callback) noexcept = 0;

    /// <summary>
    /// Unregisters the flask glow callback for the specified plugin.
    /// </summary>
    /// <param name="plugin_handle">The SKSE plugin handle of the registering plugin.</param>
    /// <returns>APIResult::OK if removed successfully.</returns>
    virtual APIResult RemovePlayFlaskGlowCallback(SKSE::PluginHandle plugin_handle) noexcept = 0;
  };

  typedef void* (*_RequestPluginAPI)(const InterfaceVersion interfaceVersion);

  /// <summary>
  /// Requests the plugin API interface.
  /// </summary>
  /// <param name="a_interfaceVersion">The requested interface version.</param>
  /// <returns>Pointer to the API interface or nullptr if not found.</returns>
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
