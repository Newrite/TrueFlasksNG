module;

#define MINI_CASE_SENSITIVE
#include "library/ini.h"

export module TrueFlasks.Config;

import TrueFlasks.Events.EventsCtx;
import TrueFlasks.Core.Utility;

namespace config
{
  export struct flask_settings_base
  {
    bool enable{true};
    bool npc{false};
    bool player{true};
    std::string notify{"I can't drink any more potions."};
    bool enable_parallel_cooldown{false};
    bool anti_spam{true};
    float anti_spam_delay{0.1f};

    float regeneration_mult_base{100.0f};
    RE::BGSKeyword* regeneration_mult_keyword{nullptr};

    int cap_base{2};
    RE::BGSKeyword* cap_keyword{nullptr};

    float cooldown_base{30.0f};
    RE::BGSKeyword* cooldown_keyword{nullptr};
    
    bool fail_audio{false};
    RE::BGSSoundDescriptorForm* fail_audio_form{nullptr};
  };

  export struct flask_settings : flask_settings_base
  {
    RE::BGSKeyword* keyword{nullptr};
  };

  export struct flask_other_settings : flask_settings_base
  {
    RE::BGSKeyword* exclusive_keyword{nullptr};
    bool revert_exclusive{false};
  };

  export struct main_settings
  {
    RE::BGSKeyword* no_remove_keyword{nullptr};
  };

  export struct prisma_flask_widget_settings
  {
    float x{0.5f};
    float y{0.5f};
    float size{0.5f};
    float opacity{1.0f};
  };

  export struct prisma_widget_settings
  {
    bool enable{true};
    bool auto_hide_ui{false};
    float x{0.10f};
    float y{0.72f};
    float size{0.50f};
    float opacity{1.00f};
    bool anchor_all_elements{true};

    prisma_flask_widget_settings health;
    prisma_flask_widget_settings stamina;
    prisma_flask_widget_settings magick;
    prisma_flask_widget_settings other;
  };

  export class config_manager final
  {
  public:
    main_settings main;
    flask_other_settings flasks_other;
    flask_settings flasks_health;
    flask_settings flasks_stamina;
    flask_settings flasks_magick;
    prisma_widget_settings prisma_widget;

    config_manager()
    {
      // Health Flasks Defaults
      flasks_health.notify = "I can't drink any more Healing Flasks.";
      
      // Stamina Flasks Defaults
      flasks_stamina.notify = "I can't drink any more Flasks of Vigor.";
      
      // Magick Flasks Defaults
      flasks_magick.notify = "I can't drink any more Flasks of Magick.";
      
      // Other Flasks Defaults
      flasks_other.cap_base = 5;
      flasks_other.cooldown_base = 180.0f;
      flasks_other.anti_spam = false;

      // Prisma Widget Flasks Defaults
      prisma_widget.health = { 0.51f, 0.28f, 0.50f, 1.00f };
      prisma_widget.stamina = { 0.04f, 0.00f, 0.50f, 0.99f };
      prisma_widget.magick = { 0.50f, 0.63f, 0.50f, 1.00f };
      prisma_widget.other = { 0.50f, 0.50f, 0.50f, 1.00f };
    }

  private:
    std::filesystem::path config_path_;
    std::mutex mutex_;

    void parse_int(const std::string& val, int& out)
    {
      if (auto res = core::utility::str_to_int64(val); res.has_value()) {
        out = static_cast<int>(*res);
      }
    }

    void parse_bool(const std::string& val, bool& out)
    {
      if (auto res = core::utility::str_to_int64(val); res.has_value()) {
        out = *res != 0;
      }
    }

    void parse_float(const std::string& val, float& out)
    {
      if (auto res = core::utility::string_to_float(val); res.has_value()) {
        out = *res;
      }
    }

    [[nodiscard]] auto parse_keyword(const std::string& val) -> RE::BGSKeyword*
    {
      auto res = core::utility::resolved_form_id_from_string(val);
      if (!res.has_value()) return nullptr;
      auto form = core::utility::get_form_from_form_id_and_mod_name(res->id, res->mod_name);
      if (form) {
        if (auto kw = form->As<RE::BGSKeyword>()) {
          logger::info("Resolved keyword: {} -> {:08X}", val, kw->GetFormID());
          return kw;
        }
        else {
          logger::warn("Form is not a keyword: {}", val);
        }
      }
      else {
        logger::warn("Keyword form not found: {}", val);
      }
      return nullptr;
    }
    
    [[nodiscard]] auto parse_sound_descriptor(const std::string& val) -> RE::BGSSoundDescriptorForm*
    {
      auto res = core::utility::resolved_form_id_from_string(val);
      if (!res.has_value()) return nullptr;
      auto form = core::utility::get_form_from_form_id_and_mod_name(res->id, res->mod_name);
      if (form) {
        if (auto kw = form->As<RE::BGSSoundDescriptorForm>()) {
          logger::info("Resolved sound descriptor: {} -> {:08X}", val, kw->GetFormID());
          return kw;
        }
        else {
          logger::warn("Form is not a sound descriptor: {}", val);
        }
      }
      else {
        logger::warn("sound descriptor form not found: {}", val);
      }
      return nullptr;
    }

    [[nodiscard]] auto keyword_to_string(RE::BGSKeyword* keyword, const std::string& default_val) -> std::string
    {
      if (!keyword) return default_val;
      return core::utility::get_form_id_hex_and_mod_name_as_string(keyword, true);
    }
    
    [[nodiscard]] auto sound_descriptor_to_string(RE::BGSSoundDescriptorForm* sound_desc, const std::string& default_val) -> std::string
    {
      if (!sound_desc) return default_val;
      return core::utility::get_form_id_hex_and_mod_name_as_string(sound_desc, true);
    }

    void read_flask_base(const mINI::INIStructure& ini, const std::string& section, flask_settings_base& settings)
    {
      std::string regen_kw = "0x800~Mod.esp";
      std::string cap_kw = "0x800~Mod.esp";
      std::string cd_kw = "0x800~Mod.esp";
      std::string fail_sound = "0x800~Mod.esp";

      if (ini.has(section)) {
        logger::info("Reading section: [{}]", section);
        const auto& collection = ini.get(section);
        if (collection.has("FlasksEnable")) parse_bool(collection.get("FlasksEnable"), settings.enable);
        if (collection.has("FlasksNPC")) parse_bool(collection.get("FlasksNPC"), settings.npc);
        if (collection.has("FlasksPlayer")) parse_bool(collection.get("FlasksPlayer"), settings.player);
        if (collection.has("FlasksNotify")) settings.notify = collection.get("FlasksNotify");
        if (collection.has("FlasksEnableParallelCooldown"))
          parse_bool(collection.get("FlasksEnableParallelCooldown"),
                     settings.enable_parallel_cooldown);
        if (collection.has("FlasksAntiSpam")) parse_bool(collection.get("FlasksAntiSpam"), settings.anti_spam);
        if (collection.has("FlasksAntiSpamDelay"))
          parse_float(collection.get("FlasksAntiSpamDelay"),
                      settings.anti_spam_delay);

        if (collection.has("FlasksRegenerationMultBase"))
          parse_float(collection.get("FlasksRegenerationMultBase"),
                      settings.regeneration_mult_base);
        if (collection.has("FlasksRegenerationMultKeyword")) regen_kw = collection.get("FlasksRegenerationMultKeyword");
        if (collection.has("FlasksCapBase")) parse_int(collection.get("FlasksCapBase"), settings.cap_base);
        if (collection.has("FlasksCapKeyword")) cap_kw = collection.get("FlasksCapKeyword");
        if (collection.has("FlasksCooldownBase"))
          parse_float(collection.get("FlasksCooldownBase"),
                      settings.cooldown_base);
        if (collection.has("FlasksCooldownKeyword")) cd_kw = collection.get("FlasksCooldownKeyword");
        
        if (collection.has("FlasksFailAudio")) parse_bool(collection.get("FlasksFailAudio"), settings.fail_audio);
        if (collection.has("FlasksFailAudioSound")) fail_sound = collection.get("FlasksFailAudioSound");
      }
      else {
        logger::info("Section [{}] not found, using defaults", section);
      }

      settings.regeneration_mult_keyword = parse_keyword(regen_kw);
      settings.cap_keyword = parse_keyword(cap_kw);
      settings.cooldown_keyword = parse_keyword(cd_kw);
      settings.fail_audio_form = parse_sound_descriptor(fail_sound);
    }

    void populate_ini(mINI::INIStructure& ini)
    {
      ini["TrueFlasksNG"]["NoRemoveKeyword"] = keyword_to_string(main.no_remove_keyword, "0x800~Mod.esp");

      auto write_flask = [&](const std::string& section, const flask_settings_base& s) {
        ini[section]["FlasksEnable"] = s.enable ? "1" : "0";
        ini[section]["FlasksNPC"] = s.npc ? "1" : "0";
        ini[section]["FlasksPlayer"] = s.player ? "1" : "0";
        ini[section]["FlasksNotify"] = s.notify;
        ini[section]["FlasksEnableParallelCooldown"] = s.enable_parallel_cooldown ? "1" : "0";
        ini[section]["FlasksAntiSpam"] = s.anti_spam ? "1" : "0";
        ini[section]["FlasksAntiSpamDelay"] = std::format("{:.1f}", s.anti_spam_delay);

        ini[section]["FlasksRegenerationMultBase"] = std::format("{:.1f}", s.regeneration_mult_base);
        ini[section]["FlasksRegenerationMultKeyword"] = keyword_to_string(s.regeneration_mult_keyword, "0x800~Mod.esp");
        ini[section]["FlasksCapBase"] = std::to_string(s.cap_base);
        ini[section]["FlasksCapKeyword"] = keyword_to_string(s.cap_keyword, "0x800~Mod.esp");
        ini[section]["FlasksCooldownBase"] = std::format("{:.1f}", s.cooldown_base);
        ini[section]["FlasksCooldownKeyword"] = keyword_to_string(s.cooldown_keyword, "0x800~Mod.esp");
        ini[section]["FlasksFailAudio"] = s.fail_audio ? "1" : "0";
        ini[section]["FlasksFailAudioSound"] = sound_descriptor_to_string(s.fail_audio_form, "0x800~Mod.esp");
      };

      write_flask("FlasksOther", flasks_other);
      ini["FlasksOther"]["FlasksOtherExclusiveKeyword"] = keyword_to_string(
        flasks_other.exclusive_keyword, "0x800~Mod.esp");
      ini["FlasksOther"]["FlasksRevertExclusive"] = flasks_other.revert_exclusive ? "1" : "0";

      auto write_flask_full = [&](const std::string& section, const flask_settings& s) {
        write_flask(section, s);
        ini[section]["FlasksKeyword"] = keyword_to_string(s.keyword, "0x800~Mod.esp");
      };

      write_flask_full("FlasksHealth", flasks_health);
      write_flask_full("FlasksStamina", flasks_stamina);
      write_flask_full("FlasksMagick", flasks_magick);

      // PrismaWidget
      ini["PrismaWidget"]["PrismaEnable"] = prisma_widget.enable ? "1" : "0";
      ini["PrismaWidget"]["AutoHideUI"] = prisma_widget.auto_hide_ui ? "1" : "0";
      ini["PrismaWidget"]["PrismaPositionX"] = std::format("{:.2f}", prisma_widget.x);
      ini["PrismaWidget"]["PrismaPositionY"] = std::format("{:.2f}", prisma_widget.y);
      ini["PrismaWidget"]["PrismaSize"] = std::format("{:.2f}", prisma_widget.size);
      ini["PrismaWidget"]["PrismaOpacity"] = std::format("{:.2f}", prisma_widget.opacity);
      ini["PrismaWidget"]["PrismaAnchorAllElements"] = prisma_widget.anchor_all_elements ? "1" : "0";

      auto write_prisma_flask = [&](const std::string& prefix, const prisma_flask_widget_settings& s) {
        ini["PrismaWidget"][prefix + "X"] = std::format("{:.2f}", s.x);
        ini["PrismaWidget"][prefix + "Y"] = std::format("{:.2f}", s.y);
        ini["PrismaWidget"][prefix + "Size"] = std::format("{:.2f}", s.size);
        ini["PrismaWidget"][prefix + "Opacity"] = std::format("{:.2f}", s.opacity);
      };

      write_prisma_flask("PrismaFlasksHealth", prisma_widget.health);
      write_prisma_flask("PrismaFlasksStamina", prisma_widget.stamina);
      write_prisma_flask("PrismaFlasksMagick", prisma_widget.magick);
      write_prisma_flask("PrismaFlasksOther", prisma_widget.other);
    }

    void generate_default(const mINI::INIFile& file, mINI::INIStructure& ini)
    {
      populate_ini(ini);

      if (file.generate(ini, true)) {
        logger::info("Default configuration generated at {}", config_path_.string());
        return;
      }

      logger::error("Error when try create default configuration at {}", config_path_.string());
    }

  public:
    static auto get_singleton() -> config_manager*
    {
      static config_manager singleton;
      return std::addressof(singleton);
    }

    auto initialize() -> void
    {
      config_path_ = "Data/SKSE/Plugins/TrueFlasksNG.ini";
      load();
    }

    auto load() -> void
    {
      std::lock_guard<std::mutex> lock(mutex_);

      logger::info("Loading configuration from {}", config_path_.string());

      mINI::INIFile file(config_path_);
      mINI::INIStructure ini;

      if (!file.read(ini)) {
        logger::info("Configuration file not found, generating default.");
        generate_default(file, ini);
        return;
      }

      // [TrueFlasksNG]
      std::string no_remove_kw = "0x800~Mod.esp";
      if (ini.has("TrueFlasksNG")) {
        const auto& sec = ini.get("TrueFlasksNG");
        if (sec.has("NoRemoveKeyword")) no_remove_kw = sec.get("NoRemoveKeyword");
      }
      main.no_remove_keyword = parse_keyword(no_remove_kw);

      // [FlasksOther]
      read_flask_base(ini, "FlasksOther", flasks_other);
      std::string exclusive_kw = "0x800~Mod.esp";
      if (ini.has("FlasksOther")) {
        const auto& sec = ini.get("FlasksOther");
        if (sec.has("FlasksOtherExclusiveKeyword")) exclusive_kw = sec.get("FlasksOtherExclusiveKeyword");
        if (sec.has("FlasksRevertExclusive"))
          parse_bool(sec.get("FlasksRevertExclusive"),
                     flasks_other.revert_exclusive);
      }
      flasks_other.exclusive_keyword = parse_keyword(exclusive_kw);

      auto read_flask_full = [&](const std::string& section, flask_settings& settings) {
        read_flask_base(ini, section, settings);
        std::string kw = "0x800~Mod.esp";
        if (ini.has(section)) {
          const auto& sec = ini.get(section);
          if (sec.has("FlasksKeyword")) kw = sec.get("FlasksKeyword");
        }
        settings.keyword = parse_keyword(kw);
      };

      read_flask_full("FlasksHealth", flasks_health);
      read_flask_full("FlasksStamina", flasks_stamina);
      read_flask_full("FlasksMagick", flasks_magick);

      // [PrismaWidget]
      if (ini.has("PrismaWidget")) {
        const auto& sec = ini.get("PrismaWidget");
        if (sec.has("PrismaEnable")) parse_bool(sec.get("PrismaEnable"), prisma_widget.enable);
        if (sec.has("AutoHideUI")) parse_bool(sec.get("AutoHideUI"), prisma_widget.auto_hide_ui);
        if (sec.has("PrismaPositionX")) parse_float(sec.get("PrismaPositionX"), prisma_widget.x);
        if (sec.has("PrismaPositionY")) parse_float(sec.get("PrismaPositionY"), prisma_widget.y);
        if (sec.has("PrismaSize")) parse_float(sec.get("PrismaSize"), prisma_widget.size);
        if (sec.has("PrismaOpacity")) parse_float(sec.get("PrismaOpacity"), prisma_widget.opacity);
        if (sec.has("PrismaAnchorAllElements"))
          parse_bool(sec.get("PrismaAnchorAllElements"),
                     prisma_widget.anchor_all_elements);

        auto read_prisma_flask = [&](const std::string& prefix, prisma_flask_widget_settings& s) {
          if (sec.has(prefix + "X")) parse_float(sec.get(prefix + "X"), s.x);
          if (sec.has(prefix + "Y")) parse_float(sec.get(prefix + "Y"), s.y);
          if (sec.has(prefix + "Size")) parse_float(sec.get(prefix + "Size"), s.size);
          if (sec.has(prefix + "Opacity")) parse_float(sec.get(prefix + "Opacity"), s.opacity);
        };

        read_prisma_flask("PrismaFlasksHealth", prisma_widget.health);
        read_prisma_flask("PrismaFlasksStamina", prisma_widget.stamina);
        read_prisma_flask("PrismaFlasksMagick", prisma_widget.magick);
        read_prisma_flask("PrismaFlasksOther", prisma_widget.other);
      }

      logger::info("Configuration loaded successfully.");
    }
    
// Функция для сохранения с сохранением комментариев
bool save_ini_preserving_comments(const std::filesystem::path& path, mINI::INIStructure& data) {
    std::ifstream file_in(path);
    if (!file_in.is_open()) return false;

    std::vector<std::string> lines;
    std::string line;
    std::string current_section;

    // Регулярка для поиска секций: [SectionName]
    std::regex section_regex(R"(^\s*\[([^\]]+)\])");
    // Регулярка для поиска ключей: Key = Value (и захват комментария после ;)
    // Группа 1: Всё до значения (ключ + =)
    // Группа 2: Значение (до ; или конца строки)
    // Группа 3: Остаток строки (комментарий)
    std::regex key_regex(R"(^(\s*[^=;]+\s*=\s*)([^;]*)(.*))");
    
    std::smatch match;

    while (std::getline(file_in, line)) {
        // 1. Проверяем, не началась ли новая секция
        if (std::regex_search(line, match, section_regex)) {
            current_section = match[1];
            lines.push_back(line);
            continue;
        }

        // 2. Если мы внутри секции, ищем ключи
        if (!current_section.empty() && std::regex_search(line, match, key_regex)) {
            std::string full_part_before = match[1].str();
            std::string key_part = full_part_before.substr(0, full_part_before.find('='));
            std::string key = core::utility::strings::trim(key_part);
            
            std::string comments = match[3].str(); // Сохраняем комментарии в конце строки

            // Если такой ключ есть в наших новых данных
            if (data.has(current_section) && data[current_section].has(key)) {
                std::string new_value = data[current_section][key];
                
                // Формируем новую строку: "Ключ = " + "НовоеЗначение" + " ; Комментарий"
                std::string new_line = match[1].str() + new_value + comments;
                lines.push_back(new_line);
                
                // (Опционально) Можно помечать ключ как записанный, чтобы потом добавить новые, 
                // если их не было в файле. Но для простого конфига это часто лишнее.
            } else {
                // Если ключа нет в новых данных (или удален), оставляем как есть
                lines.push_back(line); 
            }
        } else {
            // Комментарии и пустые строки просто копируем
            lines.push_back(line);
        }
    }
    file_in.close();

    // 3. Записываем обновленные строки обратно в файл
    std::ofstream file_out(path);
    if (!file_out.is_open()) return false;
    
    for (const auto& l : lines) {
        file_out << l << "\n";
    }
    
    return true;
}

    auto save() -> void
    {
      std::lock_guard<std::mutex> lock(mutex_);

      // 1. Сначала подготавливаем данные как обычно
      mINI::INIFile file(config_path_);
      mINI::INIStructure ini;
  
      // Если файла нет, mINI должен его создать с нуля (тогда комментариев всё равно нет)
      // Поэтому сначала проверяем чтение
      bool file_exists = file.read(ini);

      populate_ini(ini);

      bool success = false;

      if (file_exists) {
        // 2. Если файл существует — используем наш умный метод сохранения
        success = save_ini_preserving_comments(config_path_, ini);
      } else {
        // 3. Если файла нет — создаем новый стандартным методом (комментариев терять не жалко)
        success = file.generate(ini, true);
      }

      if (success) {
        logger::info("Configuration saved.");
      }
      else {
        logger::error("Failed to save configuration.");
      }
    }
  };

  export void on_menu_event(const events::events_ctx::process_event_menu_ctx& ctx)
  {
    if (!ctx.is_opening && ctx.menu_name == RE::MainMenu::MENU_NAME) {
      config_manager::get_singleton()->load();
    }
  }
}
