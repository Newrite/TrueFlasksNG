module;

#define MINI_CASE_SENSITIVE
#include "library/ini.h"

export module TrueFlasks.Config;

import TrueFlasks.Events.EventsCtx;
import TrueFlasks.Core.Utility;

namespace config {

    export struct flask_settings_base
    {
        bool enable{ true };
        bool npc{ true };
        bool player{ true };
        std::string notify{ "Я не могу выпить больше зелий." };
        bool enable_parallel_cooldown{ true };
        bool anti_spam{ true };
        float anti_spam_delay{ 0.1f };

        float regeneration_mult_base{ 100.0f };
        RE::BGSKeyword* regeneration_mult_keyword{ nullptr };
        
        int cap_base{ 1 };
        RE::BGSKeyword* cap_keyword{ nullptr };

        float cooldown_base{ 300.0f };
        RE::BGSKeyword* cooldown_keyword{ nullptr };
    };

    export struct flask_settings : flask_settings_base
    {
        RE::BGSKeyword* keyword{ nullptr };
    };

    export struct flask_other_settings : flask_settings_base
    {
        RE::BGSKeyword* exclusive_keyword{ nullptr };
        bool revert_exclusive{ false };
    };

    export struct main_settings
    {
        RE::BGSKeyword* no_remove_keyword{ nullptr };
        bool auto_hide_ui{ false };
    };

    export struct prisma_flask_widget_settings
    {
        float x{ 0.5f };
        float y{ 0.5f };
        float size{ 0.5f };
        float opacity{ 0.5f };
    };

    export struct prisma_widget_settings
    {
        bool enable{ true };
        float x{ 0.25f };
        float y{ 0.25f };
        float size{ 1.0f };
        float opacity{ 0.5f };
        bool anchor_all_elements{ true };

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

    private:
        std::filesystem::path config_path_;
        std::mutex mutex_;

        void parse_int(const std::string& val, int& out) {
            if (auto res = core::utility::str_to_int64(val); res.has_value()) {
                out = static_cast<int>(*res);
            }
        }
        void parse_bool(const std::string& val, bool& out) {
            if (auto res = core::utility::str_to_int64(val); res.has_value()) {
                out = *res != 0;
            }
        }
        void parse_float(const std::string& val, float& out) {
            if (auto res = core::utility::string_to_float(val); res.has_value()) {
                out = *res;
            }
        }

        [[nodiscard]] auto parse_keyword(const std::string& val) -> RE::BGSKeyword* {
            auto res = core::utility::resolved_form_id_from_string(val);
            if (!res.has_value()) return nullptr;
            auto form = core::utility::get_form_from_form_id_and_mod_name(res->id, res->mod_name);
            if (form) {
                if (auto kw = form->As<RE::BGSKeyword>()) {
                    logger::info("Resolved keyword: {} -> {:08X}", val, kw->GetFormID());
                    return kw;
                } else {
                    logger::warn("Form is not a keyword: {}", val);
                }
            } else {
                logger::warn("Keyword form not found: {}", val);
            }
            return nullptr;
        }

        [[nodiscard]] auto keyword_to_string(RE::BGSKeyword* keyword, const std::string& default_val) -> std::string {
            if (!keyword) return default_val;
            return core::utility::get_form_id_hex_and_mod_name_as_string(keyword, true);
        }

        void read_flask_base(const mINI::INIStructure& ini, const std::string& section, flask_settings_base& settings) {
            std::string regen_kw = "0x800~Mod.esp";
            std::string cap_kw = "0x800~Mod.esp";
            std::string cd_kw = "0x800~Mod.esp";
            
            if (ini.has(section)) {
                logger::info("Reading section: [{}]", section);
                const auto& collection = ini.get(section);
                if (collection.has("FlasksEnable")) parse_bool(collection.get("FlasksEnable"), settings.enable);
                if (collection.has("FlasksNPC")) parse_bool(collection.get("FlasksNPC"), settings.npc);
                if (collection.has("FlasksPlayer")) parse_bool(collection.get("FlasksPlayer"), settings.player);
                if (collection.has("FlasksNotify")) settings.notify = collection.get("FlasksNotify");
                if (collection.has("FlasksEnableParallelCooldown")) parse_bool(collection.get("FlasksEnableParallelCooldown"), settings.enable_parallel_cooldown);
                if (collection.has("FlasksAntiSpam")) parse_bool(collection.get("FlasksAntiSpam"), settings.anti_spam);
                if (collection.has("FlasksAntiSpamDelay")) parse_float(collection.get("FlasksAntiSpamDelay"), settings.anti_spam_delay);

                if (collection.has("FlasksRegenerationMultBase")) parse_float(collection.get("FlasksRegenerationMultBase"), settings.regeneration_mult_base);
                if (collection.has("FlasksRegenerationMultKeyword")) regen_kw = collection.get("FlasksRegenerationMultKeyword");
                if (collection.has("FlasksCapBase")) parse_int(collection.get("FlasksCapBase"), settings.cap_base);
                if (collection.has("FlasksCapKeyword")) cap_kw = collection.get("FlasksCapKeyword");
                if (collection.has("FlasksCooldownBase")) parse_float(collection.get("FlasksCooldownBase"), settings.cooldown_base);
                if (collection.has("FlasksCooldownKeyword")) cd_kw = collection.get("FlasksCooldownKeyword");
            } else {
                logger::info("Section [{}] not found, using defaults", section);
            }

            settings.regeneration_mult_keyword = parse_keyword(regen_kw);
            settings.cap_keyword = parse_keyword(cap_kw);
            settings.cooldown_keyword = parse_keyword(cd_kw);
        }

        void populate_ini(mINI::INIStructure& ini) {
             ini["TrueFlasksNG"]["NoRemoveKeyword"] = keyword_to_string(main.no_remove_keyword, "0x800~Mod.esp");
             ini["TrueFlasksNG"]["AutoHideUI"] = main.auto_hide_ui ? "1" : "0";
             
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
             };

             write_flask("FlasksOther", flasks_other);
             ini["FlasksOther"]["FlasksOtherExclusiveKeyword"] = keyword_to_string(flasks_other.exclusive_keyword, "0x800~Mod.esp");
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

        void generate_default(const mINI::INIFile& file, mINI::INIStructure& ini) {
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
                if (sec.has("AutoHideUI")) parse_bool(sec.get("AutoHideUI"), main.auto_hide_ui);
            }
            main.no_remove_keyword = parse_keyword(no_remove_kw);

            // [FlasksOther]
            read_flask_base(ini, "FlasksOther", flasks_other);
            std::string exclusive_kw = "0x800~Mod.esp";
            if (ini.has("FlasksOther")) {
                const auto& sec = ini.get("FlasksOther");
                if (sec.has("FlasksOtherExclusiveKeyword")) exclusive_kw = sec.get("FlasksOtherExclusiveKeyword");
                if (sec.has("FlasksRevertExclusive")) parse_bool(sec.get("FlasksRevertExclusive"), flasks_other.revert_exclusive);
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
                if (sec.has("PrismaPositionX")) parse_float(sec.get("PrismaPositionX"), prisma_widget.x);
                if (sec.has("PrismaPositionY")) parse_float(sec.get("PrismaPositionY"), prisma_widget.y);
                if (sec.has("PrismaSize")) parse_float(sec.get("PrismaSize"), prisma_widget.size);
                if (sec.has("PrismaOpacity")) parse_float(sec.get("PrismaOpacity"), prisma_widget.opacity);
                if (sec.has("PrismaAnchorAllElements")) parse_bool(sec.get("PrismaAnchorAllElements"), prisma_widget.anchor_all_elements);

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

        auto save() -> void
        {
            std::lock_guard<std::mutex> lock(mutex_);
            
            mINI::INIFile file(config_path_);
            mINI::INIStructure ini;
            
            file.read(ini);
            
            populate_ini(ini);
            
            if (file.generate(ini, true)) {
                logger::info("Configuration saved.");
            } else {
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
