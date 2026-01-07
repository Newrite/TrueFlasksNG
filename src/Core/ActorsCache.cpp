export module TrueFlasks.Core.ActorsCache;

namespace core::actors_cache
{
struct cache_data final {

    struct actor_data final
    {
    
        struct flask_duration final
        {
        
            explicit flask_duration(const float duration_start) : duration_start(duration_start)
            {
                duration_current = 0.f;
            }
        
            float duration_start;
            float duration_current;
        };
    
        static constexpr auto FLASK_ARRAY_SIZE = 64;
        flask_duration flasks_health[FLASK_ARRAY_SIZE];
        flask_duration flasks_magick[FLASK_ARRAY_SIZE];
        flask_duration flasks_stamina[FLASK_ARRAY_SIZE];
        flask_duration flasks_others[FLASK_ARRAY_SIZE];
        
        std::uint64_t last_tick{GetTickCount64()};
    };
    
private:
    std::map<RE::FormID, actor_data> actors_cache_;
    std::mutex mutex_;
    static constexpr uint64_t GARBAGE_TIME = 5000;
    static constexpr uint32_t LABEL = 'TFNG';
    
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

};
}
