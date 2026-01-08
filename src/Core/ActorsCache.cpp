module;

#include <optional>
#include "API/TrueFlasksAPI.h"

export module TrueFlasks.Core.ActorsCache;

namespace core::actors_cache
{

export struct cache_data final {

    struct actor_data final
    {
      
      struct delta_data final
      {
        float delta;
        float delta_health;
        float delta_stamina;
        float delta_magick;
        float delta_other;
      };
    
        struct flask_cooldown final
        {
            flask_cooldown() : cooldown_start(0.f), cooldown_current(0.f) {}
            flask_cooldown(const float cooldown_start) : cooldown_start(cooldown_start), cooldown_current(cooldown_start) {}
        
            float cooldown_start;
            float cooldown_current;
        };
    
        static constexpr auto FLASK_ARRAY_SIZE = 64;
        flask_cooldown flasks_health[FLASK_ARRAY_SIZE];
        flask_cooldown flasks_magick[FLASK_ARRAY_SIZE];
        flask_cooldown flasks_stamina[FLASK_ARRAY_SIZE];
        flask_cooldown flasks_others[FLASK_ARRAY_SIZE];
      
      float anti_spam_duration{0.f};
      std::optional<TrueFlasksAPI::FlaskType> failed_drink_type;
        
        std::uint64_t last_tick{GetTickCount64()};
      
      void update(const delta_data& delta_data)
      {
        last_tick = GetTickCount64();
        if (anti_spam_duration > 0.f) {
          anti_spam_duration = anti_spam_duration - delta_data.delta;
        }

        auto update_flasks = [](flask_cooldown* flasks, float delta) {
            for (int i = 0; i < FLASK_ARRAY_SIZE; ++i) {
                if (flasks[i].cooldown_current > 0.f) {
                    flasks[i].cooldown_current -= delta;
                }
            }
        };

        update_flasks(flasks_health, delta_data.delta_health);
        update_flasks(flasks_stamina, delta_data.delta_stamina);
        update_flasks(flasks_magick, delta_data.delta_magick);
        update_flasks(flasks_others, delta_data.delta_other);
      }
    };
    
private:
    std::map<RE::FormID, actor_data> actors_cache_;
    std::mutex mutex_;
    static constexpr uint64_t GARBAGE_TIME = 5000;
    static constexpr uint32_t SERIALIZATION_VERSION = 1;
    static constexpr uint32_t LABEL = 'CDAD';
    
    [[nodiscard]] static auto is_garbage(const actor_data& data) -> bool
    {
        return (GetTickCount64() - data.last_tick) >= GARBAGE_TIME;
    }

    auto clean_cache_map(const std::vector<RE::FormID>* keys_to_delete) -> void
    {
        if (!keys_to_delete || keys_to_delete->empty()) {
            return;
        }
        for (auto key : *keys_to_delete) {
            actors_cache_.erase(key);
        }
    }

    // Нежелательно одновременно итерироваться по коллекции и мутировать ее при этом (в данном случае - удалять значения из мапы при проходе по мапе)
    // В некоторых случаях это может приводить к ошибкам 
    auto garbage_collector() -> void
    {

        auto keys_to_delete = std::vector<RE::FormID>{};
        for (const auto& [form_id, data] : actors_cache_) {
            if (is_garbage(data)) {
                keys_to_delete.push_back(form_id);
            }
        }

        clean_cache_map(std::addressof(keys_to_delete));
    }
    
        auto load(const SKSE::SerializationInterface* a_interface) -> void
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        uint32_t type;
        uint32_t version;
        uint32_t length;

        actors_cache_.clear();

        while (a_interface->GetNextRecordInfo(type, version, length)) {
            if (type == LABEL) {
                uint32_t serialization_version;
                if (!a_interface->ReadRecordData(serialization_version)) {
                    actors_cache_.clear();
                    return;
                }

                if (serialization_version != SERIALIZATION_VERSION) {
                    return;
                }

                size_t size;
                if (!a_interface->ReadRecordData(size)) {
                    break;
                }

                for (size_t i = 0; i < size; ++i) {
                    RE::FormID form_id;
                    if (!a_interface->ReadRecordData(form_id)) {
                        break;
                    }

                    actor_data data;
                    if (!a_interface->ReadRecordData(data)) {
                        break;
                    }
                    actors_cache_[form_id] = data;
                }
            }
        }
    }

    auto save(SKSE::SerializationInterface* a_interface) -> void
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!a_interface->OpenRecord(LABEL, SERIALIZATION_VERSION)) {
            return;
        }

        garbage_collector();

        if (!a_interface->WriteRecordData(SERIALIZATION_VERSION)) {
            return;
        }

        const size_t size = actors_cache_.size();
        if (!a_interface->WriteRecordData(size)) {
            return;
        }

        for (const auto& [form_id, data] : actors_cache_) {
            if (!a_interface->WriteRecordData(form_id)) {
                return;
            }
            if (!a_interface->WriteRecordData(data)) {
                return;
            }
        }
    }
    
public:
    
    static auto get_singleton() -> cache_data*
    {
        static cache_data singleton;
        return std::addressof(singleton);
    }

    auto get_or_add(const RE::FormID form_id) -> actor_data&
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return actors_cache_.insert_or_assign(form_id, actor_data{}).first->second;
    }

    static auto skse_save_callback(SKSE::SerializationInterface* serialization_interface) -> void
    {
        get_singleton()->save(serialization_interface);
    }

    static auto skse_load_callback(SKSE::SerializationInterface* serialization_interface) -> void
    {
        get_singleton()->load(serialization_interface);
    }

};
}
