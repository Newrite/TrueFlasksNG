export module TrueFlasks.Core.Hooks;

import TrueFlasks.Core.HooksCtx;
import TrueFlasks.Features.TrueFlasks;
import TrueFlasks.UI.Prisma;

namespace core::hooks
{
  template<typename T>
  auto write_call(SKSE::Trampoline& trampoline, const uintptr_t src, T& func, REL::Relocation<T>& func_original,
                  const char* label) -> void
  {
    logger::info("Hook write_call: {}", label);
    func_original = trampoline.write_call<5>(src, func);
  }

  template<typename T>
  auto write_vfunc(REL::Relocation<> addr,
                   const uintptr_t offset,
                   T& func,
                   REL::Relocation<T>& func_original, const char* label) -> void
  {
    logger::info("Hook write_vfunc: {}", label);
    func_original = addr.write_vfunc(offset, func);
  }

  float last_player_delta = 0.1f;

  STATIC_STRUCT(character)

    static constexpr auto addr_vtable_character = RE::Character::VTABLE[0];
    static constexpr auto addr_vtable_player_character = RE::PlayerCharacter::VTABLE[0];
    static constexpr auto offset_vtable_update = RELOCATION_OFFSET(0xAD, 0xAD);
    static constexpr auto offset_vtable_drink_potion = RELOCATION_OFFSET(0x10F, 0x10F);
    static constexpr auto offset_vtable_drink_potion_vr = 0x111;
    static constexpr auto offset_vtable_remove_item = RELOCATION_OFFSET(0x56, 0x56);

    static auto on_update(hooks_ctx::on_actor_update& ctx) -> void
    {
      features::true_flasks::update(ctx);
    }

    static auto on_update_character(RE::Character* character, const float delta) -> void
    {
      if (!character || !delta) {
        return on_update_character_original(character, delta);
      }

      auto ctx = hooks_ctx::on_actor_update{character, last_player_delta};
      on_update(ctx);

      return on_update_character_original(character, delta);
    }
  
    static auto on_update_player_character(RE::PlayerCharacter* character, const float delta) -> void
    {
      last_player_delta = delta;

      if (!character || !delta) {
        return on_update_player_character_original(character, delta);
      }
      
      auto ctx = hooks_ctx::on_actor_update{character, last_player_delta};
      
      // Push all flask data, but only every 0.1 s
      static float delta_counter = 0.f;
      delta_counter += delta;
      if (delta_counter >= 0.1f) {
        delta_counter = 0.f;
        ui::prisma::update(ctx);
        features::true_flasks::update_ui(ctx);
      }

      on_update(ctx);
      
      return on_update_player_character_original(character, delta);
    }

    // ExtraList может быть нуллом, т.к. далеко не все объекты несут в себе дополнительную рантайм информацию
    static auto on_drink_potion(RE::Character* character, RE::AlchemyItem* potion,
                                RE::ExtraDataList* extra_list) -> bool
    {
      auto ctx = hooks_ctx::on_actor_drink_potion{character, potion, extra_list};
      return features::true_flasks::drink_potion(ctx);
    }

    static auto on_drink_potion_character(RE::Character* character, RE::AlchemyItem* potion,
                                          RE::ExtraDataList* extra_list) -> bool
    {
      if (!character || !potion) {
        return on_drink_potion_character_original(character, potion, extra_list);
      }

      return on_drink_potion(character, potion, extra_list) && on_drink_potion_character_original(
               character, potion, extra_list);
    }

    static auto on_drink_potion_player_character(RE::PlayerCharacter* character, RE::AlchemyItem* potion,
                                                 RE::ExtraDataList* extra_list) -> bool
    {
      if (!character || !potion) {
        return on_drink_potion_player_character_original(character, potion, extra_list);
      }

      return on_drink_potion(character, potion, extra_list) && on_drink_potion_player_character_original(
               character, potion, extra_list);
    }
  
  static auto on_remove_item_character(RE::Character* actor, RE::ObjectRefHandle* ret_handle, RE::TESBoundObject* item, std::int32_t count, RE::ITEM_REMOVE_REASON reason, RE::ExtraDataList* extra_list, RE::TESObjectREFR* move_to_ref, const RE::NiPoint3* drop_loc, const RE::NiPoint3* rotate) -> RE::ObjectRefHandle*
  {
      auto ctx = hooks_ctx::on_actor_remove_item{
        .actor = actor, 
        .item = item, 
        .count = count,
        .reason = reason, 
        .move_to_ref = move_to_ref, 
        .drop_loc = drop_loc, 
        .rotate = rotate
      };
      
      features::true_flasks::remove_item(ctx);

      return on_remove_item_character_original(actor, ret_handle, item, count, reason, extra_list, move_to_ref, drop_loc, rotate);
  }

  static auto on_remove_item_player_character(RE::PlayerCharacter* actor, RE::ObjectRefHandle* ret_handle, RE::TESBoundObject* item, std::int32_t count, RE::ITEM_REMOVE_REASON reason, RE::ExtraDataList* extra_list, RE::TESObjectREFR* move_to_ref, const RE::NiPoint3* drop_loc, const RE::NiPoint3* rotate) -> RE::ObjectRefHandle*
  {
      auto ctx = hooks_ctx::on_actor_remove_item{
        .actor = actor, 
        .item = item, 
        .count = count, 
        .reason = reason, 
        .move_to_ref = move_to_ref, 
        .drop_loc = drop_loc, 
        .rotate = rotate
      };

      features::true_flasks::remove_item(ctx);

      return on_remove_item_player_character_original(actor, ret_handle, item, ctx.count, reason, extra_list, move_to_ref, drop_loc, rotate);
  }

    static inline REL::Relocation<decltype(on_update_character)> on_update_character_original;
    static inline REL::Relocation<decltype(on_update_player_character)> on_update_player_character_original;
    static inline REL::Relocation<decltype(on_drink_potion_character)> on_drink_potion_character_original;
    static inline REL::Relocation<decltype(on_drink_potion_player_character)> on_drink_potion_player_character_original;
    static inline REL::Relocation<decltype(on_remove_item_character)> on_remove_item_character_original;
    static inline REL::Relocation<decltype(on_remove_item_player_character)> on_remove_item_player_character_original;

    static auto install_hook() -> void
    {
      constexpr auto is_vr = REL::Module::IsVR();
      
      write_vfunc(REL::Relocation<>{addr_vtable_character},
                  is_vr ? offset_vtable_drink_potion_vr : offset_vtable_update,
                  on_update_character,
                  on_update_character_original, "on_update_character");
      write_vfunc(REL::Relocation<>{addr_vtable_player_character},
                  offset_vtable_update,
                  on_update_player_character,
                  on_update_player_character_original, "on_update_player_character");
      write_vfunc(REL::Relocation<>{addr_vtable_character},
                  offset_vtable_drink_potion,
                  on_drink_potion_character,
                  on_drink_potion_character_original, "on_drink_potion_character");
      write_vfunc(REL::Relocation<>{addr_vtable_player_character},
                  offset_vtable_drink_potion,
                  on_drink_potion_player_character,
                  on_drink_potion_player_character_original, "on_drink_potion_player_character");
      write_vfunc(REL::Relocation<>{addr_vtable_character},
                  offset_vtable_remove_item,
                  on_remove_item_character,
                  on_remove_item_character_original, "on_remove_item_character");
      write_vfunc(REL::Relocation<>{addr_vtable_player_character},
                  offset_vtable_remove_item,
                  on_remove_item_player_character,
                  on_remove_item_player_character_original, "on_remove_item_player_character");
    }
  };

  export auto install_hooks() -> void
  {
    auto& trampoline = SKSE::GetTrampoline();
    trampoline.create(512);

    character::install_hook();
  }
}
