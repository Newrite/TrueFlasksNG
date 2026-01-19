module;

#include <expected>
#include <Windows.h>

export module TrueFlasks.Core.Utility;

namespace core::utility::strings
{
  export auto trim(const std::string& str) -> std::string
  {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
      return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
  }

  export auto split(const std::string& str, const char delimiter) -> std::vector<std::string>
  {
    return str | std::views::split(delimiter) | std::ranges::to<std::vector<std::string>>();
  }

  export auto trim_left(std::string_view s) -> std::string
  {
    auto view = s | std::views::drop_while(isspace);
    return {view.begin(), view.end()};
  }

  export auto to_upper(std::string str) -> std::string
  {
    std::ranges::transform(str,
                           str.begin(),
                           [](unsigned char c) { return std::toupper(c); });
    return str;
  }

  export auto to_lower(std::string str) -> std::string
  {
    std::ranges::transform(str,
                           str.begin(),
                           [](unsigned char c) { return std::tolower(c); });
    return str;
  }
}

namespace core::utility
{
  export struct success
  {
  };

  export constexpr success success_instance{};

  export enum class string_to_number_error
  {
    out_of_range,
    invalid_string
  };

  enum class resolve_form_id_error
  {
    parts_after_split_not_equal_two,
    parts_after_split_is_empty,
    cant_parse_to_formId_from_string
  };

  using ModName = std::string;

  export struct resolved_form_id
  {
    RE::FormID id;
    ModName mod_name;
  };

#define NOMINMAX
#undef max
#undef min
  export auto hex_to_int64(const std::string& hex_str) -> std::expected<int64_t, string_to_number_error>
  {
    char* endptr;
    errno = 0;
    const int64_t value = strtol(hex_str.c_str(), &endptr, 16);

    if (errno == ERANGE || value > (std::numeric_limits<int64_t>::max()) || value < (std::numeric_limits<
          int64_t>::min())) {
      return std::unexpected(string_to_number_error::out_of_range);
    }

    if (*endptr != '\0') {
      return std::unexpected(string_to_number_error::invalid_string);
    }

    return value;
  }

  export auto hex_to_uint32(const std::string& hex_str) -> std::expected<uint32_t, string_to_number_error>
  {
    char* endptr;
    errno = 0;
    const uint32_t value = strtol(hex_str.c_str(), &endptr, 16);

    if (errno == ERANGE || value > (std::numeric_limits<uint32_t>::max()) || value < (std::numeric_limits<
          uint32_t>::min())) {
      return std::unexpected(string_to_number_error::out_of_range);
    }

    if (*endptr != '\0') {
      return std::unexpected(string_to_number_error::invalid_string);
    }

    return value;
  }

  export auto str_to_int64(const std::string& hex_str) -> std::expected<int64_t, string_to_number_error>
  {
    char* endptr;
    errno = 0;
    const int64_t value = strtol(hex_str.c_str(), &endptr, 0);

    if (errno == ERANGE || value > (std::numeric_limits<int64_t>::max()) || value < (std::numeric_limits<
          int64_t>::min())) {
      return std::unexpected(string_to_number_error::out_of_range);
    }

    if (*endptr != '\0') {
      return std::unexpected(string_to_number_error::invalid_string);
    }

    return value;
  }

  export auto str_to_uint32(const std::string& hex_str) -> std::expected<uint32_t, string_to_number_error>
  {
    char* endptr;
    errno = 0;
    const uint32_t value = strtol(hex_str.c_str(), &endptr, 0);

    if (errno == ERANGE || value > (std::numeric_limits<uint32_t>::max()) || value < (std::numeric_limits<
          uint32_t>::min())) {
      return std::unexpected(string_to_number_error::out_of_range);
    }

    if (*endptr != '\0') {
      return std::unexpected(string_to_number_error::invalid_string);
    }

    return value;
  }

  export auto str_to_uint16(const std::string& hex_str) -> std::expected<uint16_t, string_to_number_error>
  {
    char* endptr;
    errno = 0;
    const uint16_t value = static_cast<uint16_t>(strtol(hex_str.c_str(), &endptr, 0));

    if (errno == ERANGE || value > (std::numeric_limits<uint16_t>::max()) || value < (std::numeric_limits<
          uint16_t>::min())) {
      return std::unexpected(string_to_number_error::out_of_range);
    }

    if (*endptr != '\0') {
      return std::unexpected(string_to_number_error::invalid_string);
    }

    return value;
  }

  export auto string_to_float(const std::string& str) -> std::expected<float, string_to_number_error>
  {
    char* endptr;
    errno = 0;
    const double value = strtod(str.c_str(), &endptr);

    if (errno == ERANGE || value > std::numeric_limits<float>::max() || value < std::numeric_limits<float>::lowest()) {
      return std::unexpected(string_to_number_error::out_of_range);
    }

    if (*endptr != '\0') {
      return std::unexpected(string_to_number_error::invalid_string);
    }

    return static_cast<float>(value);
  }


  export auto resolved_form_id_from_string(
    const std::string& line) -> std::expected<resolved_form_id, resolve_form_id_error>
  {
    constexpr auto FORM_ID_DELIMITER = '~';
    const auto split_line = strings::split(line, FORM_ID_DELIMITER);
    if (split_line.empty()) {
      return std::unexpected(resolve_form_id_error::parts_after_split_is_empty);
    }
    if (split_line.size() != 2) {
      return std::unexpected(resolve_form_id_error::parts_after_split_not_equal_two);
    }
    const auto form_id = hex_to_int64(split_line.at(0));
    if (!form_id.has_value()) {
      return std::unexpected(resolve_form_id_error::cant_parse_to_formId_from_string);
    }
    return resolved_form_id{static_cast<RE::FormID>(form_id.value()), split_line.at(1)};
  }

  export auto resolved_form_id_from_string_form_id_and_mod_name(
    const std::string& form_id_str,
    const std::string& mod_name_str) -> std::expected<resolved_form_id, resolve_form_id_error>
  {
    const auto form_id = hex_to_int64(form_id_str);
    if (!form_id.has_value()) {
      return std::unexpected(resolve_form_id_error::cant_parse_to_formId_from_string);
    }
    return resolved_form_id{static_cast<RE::FormID>(form_id.value()), mod_name_str};
  }

  export auto get_form_from_form_id_and_mod_name(const RE::FormID form_id, const std::string& mod_name) -> RE::TESForm*
  {
    auto data_handler = RE::TESDataHandler::GetSingleton();
    return data_handler->LookupForm(form_id, mod_name);
  }

  export auto vector_int_to_vector_av(const std::vector<int>& vector_int) -> std::vector<RE::ActorValue>
  {
    std::vector<RE::ActorValue> result;
    result.reserve(vector_int.size());

    for (const auto value : vector_int) {
      result.push_back(static_cast<RE::ActorValue>(value));
    }

    return result;
  }

  using _GetFormEditorID = const char* (*)(std::uint32_t);

  export auto get_editor_id(const RE::TESForm* a_form) -> const char*
  {
    
    if (!a_form) {
      return "NullEDID";
    }
    
    switch (a_form->GetFormType()) {
    case RE::FormType::Keyword:
    case RE::FormType::LocationRefType:
    case RE::FormType::Action:
    case RE::FormType::MenuIcon:
    case RE::FormType::Global:
    case RE::FormType::HeadPart:
    case RE::FormType::Race:
    case RE::FormType::Sound:
    case RE::FormType::Script:
    case RE::FormType::Navigation:
    case RE::FormType::Cell:
    case RE::FormType::WorldSpace:
    case RE::FormType::Land:
    case RE::FormType::NavMesh:
    case RE::FormType::Dialogue:
    case RE::FormType::Quest:
    case RE::FormType::Idle:
    case RE::FormType::AnimatedObject:
    case RE::FormType::ImageAdapter:
    case RE::FormType::VoiceType:
    case RE::FormType::Ragdoll:
    case RE::FormType::DefaultObject:
    case RE::FormType::MusicType:
    case RE::FormType::StoryManagerBranchNode:
    case RE::FormType::StoryManagerQuestNode:
    case RE::FormType::StoryManagerEventNode:
      return a_form->GetFormEditorID();
    default: {
      static auto tweaks = GetModuleHandle("po3_Tweaks");
      static auto func = reinterpret_cast<_GetFormEditorID>(GetProcAddress(tweaks, "GetFormEditorID"));
      if (func) {
        return func(a_form->formID);
      }
      return "";
    }
    }
  }
  

  export auto form_info(const RE::TESForm* form) -> std::string
  {
    if (!form) {
      return "NULL_FORM";
    }
    
    const char* fileName = "NullFile";
    if (const auto file = form->GetFile()) {
      fileName = file->fileName;
    }
    
    const char* editorId = get_editor_id(form);
    if (!editorId) editorId = "NoEditorID"; 
    
    const char* formName = form->GetName();
    if (!formName) formName = "NoName";

    const char* objectTypeName = form->GetObjectTypeName();
    if (!objectTypeName) objectTypeName = "NoObjectTypeName";

    return std::format("{:#x}~{}|{} ({}-{})",
                       form->formID,
                       fileName,
                       editorId,
                       formName,
                       objectTypeName);
  }

  export auto get_form_id_hex_as_string(const RE::TESForm* form, const bool is_local_id) -> std::string
  {
    if (!form) {
      return "NULL_FORM";
    }

    return std::format("{:#x}", is_local_id ? form->GetLocalFormID() : form->GetFormID());
  }

  export auto get_form_id_hex_and_mod_name_as_string(const RE::TESForm* form, const bool is_local_id) -> std::string
  {
    if (!form) {
      return "NULL_FORM";
    }

    return std::format("{:#x}~{}", is_local_id ? form->GetLocalFormID() : form->GetFormID(), form->GetFile()->fileName);
  }

  export auto get_actor_from_ni_actor(RE::NiPointer<RE::Actor>& ni_actor) -> RE::Actor*
  {
    if (!ni_actor) {
      return nullptr;
    }

    return ni_actor.get();
  }

  export auto get_actor_from_actor_handle(RE::ActorHandle& actor_handle) -> RE::Actor*
  {
    if (!actor_handle) {
      return nullptr;
    }

    auto ni_actor = actor_handle.get();
    if (!ni_actor) {
      return nullptr;
    }

    return ni_actor.get();
  }

  export auto get_active_effects_by_keyword(RE::Actor* actor,
                                            const RE::BGSKeyword* keyword,
                                            const bool is_only_active = true) -> std::vector<RE::ActiveEffect*>
  {
    auto result = std::vector<RE::ActiveEffect*>();

    if (!actor || !keyword) {
      return result;
    }

    auto active_effects = actor->AsMagicTarget()->GetActiveEffectList();
    if (!active_effects) {
      return result;
    }

    for (const auto active_effect : *active_effects) {
      if (active_effect && active_effect->effect && active_effect->effect->baseEffect) {
        const auto is_effect_active = !active_effect->flags.any(RE::ActiveEffect::Flag::kInactive);
        const auto baseEffect = active_effect->effect->baseEffect;

        if (is_only_active && !is_effect_active) {
          continue;
        }

        if (baseEffect->HasKeyword(keyword)) {
          result.push_back(active_effect);
        }
      }
    }

    return result;
  }
  
  export auto is_player(const RE::Actor* form) -> bool
  {
    if (!form) {
      return true;
    }

    return form->IsPlayerRef() || form->IsPlayer();
  }

  export auto get_magnitude_sum_of_active_effects(const std::vector<RE::ActiveEffect*>* active_effects,
                                                  const bool is_only_active = true,
                                                  const bool is_only_recover = false) -> float
  {
    float result = 0.0f;

    if (!active_effects) {
      return result;
    }

    for (const auto active_effect : *active_effects) {
      if (active_effect && active_effect->effect && active_effect->effect->baseEffect) {
        const auto is_effect_active = !active_effect->flags.any(RE::ActiveEffect::Flag::kInactive);
        const auto is_detrimental = active_effect->effect->baseEffect->data.flags.any(
          RE::EffectSetting::EffectSettingData::Flag::kDetrimental);
        const auto is_recover = active_effect->effect->baseEffect->data.flags.any(
          RE::EffectSetting::EffectSettingData::Flag::kRecover);
        const auto magnitude = active_effect->GetMagnitude();

        if (is_only_active && !is_effect_active) {
          continue;
        }

        if (is_only_recover && !is_recover) {
          continue;
        }

        if (is_detrimental) {
          result -= magnitude;
          continue;
        }
        result += magnitude;
      }
    }

    return result;
  }

  // Checking if Wheeler exists, getting check function if so
  using IsWheelerOpen_t = bool(*)();

  IsWheelerOpen_t GetIsWheelerOpen()
  {
    HMODULE wheelerHandle = GetModuleHandleA("wheeler.dll");

    if (!wheelerHandle)
      return nullptr;

    return reinterpret_cast<IsWheelerOpen_t>(
      GetProcAddress(wheelerHandle, "IsWheelerOpen"));
  }

  // Perform check for wheeler, and do not dodge if the mod exists and menu is open
  bool CheckIfWheelerOpen()
  {
    if (auto isOpenFunc = GetIsWheelerOpen()) {
      if (isOpenFunc()) {
        return true;
      }
      return false;
    }
    return false;
  }

  export auto is_any_menu_open() -> bool
  {
    const auto ui = RE::UI::GetSingleton();
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (player && ui &&
        (ui->GameIsPaused() || ui->IsMenuOpen(RE::TweenMenu::MENU_NAME) || ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)
         ||
         ui->IsMenuOpen(RE::MagicMenu::MENU_NAME) || ui->IsMenuOpen(RE::InventoryMenu::MENU_NAME) ||
         ui->IsMenuOpen(RE::LockpickingMenu::MENU_NAME) || ui->IsMenuOpen(RE::RaceSexMenu::MENU_NAME) ||
         ui->IsMenuOpen(RE::StatsMenu::MENU_NAME) || ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME) ||
         ui->IsMenuOpen(RE::Console::MENU_NAME) || ui->IsMenuOpen(RE::FaderMenu::MENU_NAME) ||
         ui->IsMenuOpen(RE::FavoritesMenu::MENU_NAME) || ui->IsMenuOpen(RE::SleepWaitMenu::MENU_NAME) ||
         ui->IsMenuOpen(RE::JournalMenu::MENU_NAME) || ui->IsMenuOpen(RE::BarterMenu::MENU_NAME) ||
         ui->IsMenuOpen(RE::MainMenu::MENU_NAME) || ui->IsMenuOpen(RE::BookMenu::MENU_NAME) ||
         ui->IsMenuOpen(RE::ContainerMenu::MENU_NAME) || ui->IsMenuOpen(RE::GiftMenu::MENU_NAME) || ui->
         IsMenuOpen(RE::MessageBoxMenu::MENU_NAME) || ui->IsMenuOpen(RE::TrainingMenu::MENU_NAME) || ui->
         IsMenuOpen(RE::MapMenu::MENU_NAME) || ui->IsMenuOpen(RE::TutorialMenu::MENU_NAME) || ui->
         IsMenuOpen(RE::LevelUpMenu::MENU_NAME) || ui->IsMenuOpen(RE::CreditsMenu::MENU_NAME) || ui->
         IsMenuOpen("LootMenuIE") || ui->IsMenuOpen("LootMenu") || CheckIfWheelerOpen())) {
      return true;
    }
    return false;
  }

  export auto get_game_setting_float(const char* game_setting, const float default_value) -> float
  {
    auto settings = RE::GameSettingCollection::GetSingleton();
    if (!settings) {
      return default_value;
    }

    auto setting = settings->GetSetting(game_setting);
    if (!settings) {
      return default_value;
    }

    return setting->GetFloat();
  }

  export auto try_form_has_keyword(const RE::TESForm* form, const RE::BGSKeyword* keyword) -> bool
  {
    if (!form || !keyword) {
      return false;
    }

    const auto keyword_form = form->As<RE::BGSKeywordForm>();

    if (!keyword_form) {
      return false;
    }

    return keyword_form->HasKeyword(keyword);
  }

  export auto get_actor_value_max(RE::Actor* actor, const RE::ActorValue av) -> float
  {
    if (!actor) {
      return 0.f;
    }
    return actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, av) + actor->AsActorValueOwner()->
           GetPermanentActorValue(av);
  }

  export auto get_actor_value_owner_as_actor(RE::ActorValueOwner* actor_value_owner) -> RE::Actor*
  {
    const auto actor = (RE::Actor*)((char*)actor_value_owner - 0xB0);
    if (!actor) {
      return nullptr;
    }
    return actor;
  }
}

namespace core::utility::game
{
  export auto place_at_me(RE::TESObjectREFR* target,
                          RE::TESForm* object,
                          bool persistent,
                          bool disabled,
                          std::uint32_t count = 1) -> RE::TESObjectREFR*
  {
    if (!target || !object) {
      return nullptr;
    }
    if (object->GetFormType() == RE::FormType::Explosion && disabled) {
      return nullptr;
    }
    using func_t = RE::TESObjectREFR*(*)(std::int64_t,
                                         std::int32_t,
                                         RE::TESObjectREFR*,
                                         RE::TESForm*,
                                         std::int32_t,
                                         bool,
                                         bool);
    static REL::Relocation<func_t> func{RELOCATION_ID(55672, 56203)};
    return func(NULL, NULL, target, object, count, persistent, disabled);
  }
}
