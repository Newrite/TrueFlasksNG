module;

#include "API/TrueFlasksAPI.h"

export module TrueFlasks.Papyrus;

import TrueFlasks.Features.TrueFlasks;

namespace papyrus {

    using namespace features::true_flasks;

    // Helper to convert int to flask_type
    flask_type int_to_flask_type(int type) {
        switch (type) {
            case 0: return flask_type::health;
            case 1: return flask_type::stamina;
            case 2: return flask_type::magick;
            default: return flask_type::other;
        }
    }

    void ModifyCooldown(RE::StaticFunctionTag*, RE::Actor* actor, int type, float amount, bool all_slots) {
        if (!actor) return;
        api_modify_cooldown(actor, int_to_flask_type(type), amount, all_slots);
    }

    float GetNextCooldown(RE::StaticFunctionTag*, RE::Actor* actor, int type) {
        if (!actor) return 0.0f;
        return api_get_next_cooldown(actor, int_to_flask_type(type));
    }

    int GetMaxSlots(RE::StaticFunctionTag*, RE::Actor* actor, int type) {
        if (!actor) return 0;
        return api_get_max_slots(actor, int_to_flask_type(type));
    }

    int GetCurrentSlots(RE::StaticFunctionTag*, RE::Actor* actor, int type) {
        if (!actor) return 0;
        return api_get_current_slots(actor, int_to_flask_type(type));
    }

    float GetRegenMult(RE::StaticFunctionTag*, RE::Actor* actor, int type) {
        if (!actor) return 0.0f;
        return api_get_regen_mult(actor, int_to_flask_type(type));
    }

    float GetCooldownPct(RE::StaticFunctionTag*, RE::Actor* actor, int type) {
        if (!actor) return 1.0f;
        return api_get_cooldown_pct(actor, int_to_flask_type(type));
    }

    export bool Register(RE::BSScript::IVirtualMachine* vm) {
        vm->RegisterFunction("ModifyCooldown", "TrueFlasksNG", ModifyCooldown);
        vm->RegisterFunction("GetNextCooldown", "TrueFlasksNG", GetNextCooldown);
        vm->RegisterFunction("GetMaxSlots", "TrueFlasksNG", GetMaxSlots);
        vm->RegisterFunction("GetCurrentSlots", "TrueFlasksNG", GetCurrentSlots);
        vm->RegisterFunction("GetRegenMult", "TrueFlasksNG", GetRegenMult);
        vm->RegisterFunction("GetCooldownPct", "TrueFlasksNG", GetCooldownPct);
        return true;
    }
}
