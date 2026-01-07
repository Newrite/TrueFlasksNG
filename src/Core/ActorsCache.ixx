export module TrueFlasks.Core.ActorsCache;

namespace core::actors_cache {

export struct data final
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
};

}
