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
      on_update(ctx);
      ui::prisma::update(ctx);

      return on_update_player_character_original(character, delta);
    }

    static auto on_drink_potion(RE::Character* character, RE::AlchemyItem* potion,
                                RE::ExtraDataList* extra_list) -> bool
    {
      auto ctx = hooks_ctx::on_actor_drink_potion{character, potion, extra_list};
      return features::true_flasks::drink_potion(ctx);
    }

    static auto on_drink_potion_character(RE::Character* character, RE::AlchemyItem* potion,
                                          RE::ExtraDataList* extra_list) -> bool
    {
      if (!character || !potion || !extra_list) {
        return on_drink_potion_character_original(character, potion, extra_list);
      }

      return on_drink_potion(character, potion, extra_list) && on_drink_potion_character_original(
               character, potion, extra_list);
    }

    static auto on_drink_potion_player_character(RE::PlayerCharacter* character, RE::AlchemyItem* potion,
                                                 RE::ExtraDataList* extra_list) -> bool
    {
      if (!character || !potion || !extra_list) {
        return on_drink_potion_player_character_original(character, potion, extra_list);
      }

      return on_drink_potion(character, potion, extra_list) && on_drink_potion_player_character_original(
               character, potion, extra_list);
    }

    static inline REL::Relocation<decltype(on_update_character)> on_update_character_original;
    static inline REL::Relocation<decltype(on_update_player_character)> on_update_player_character_original;
    static inline REL::Relocation<decltype(on_drink_potion_character)> on_drink_potion_character_original;
    static inline REL::Relocation<decltype(on_drink_potion_player_character)> on_drink_potion_player_character_original;

    static auto install_hook() -> void
    {
      write_vfunc(REL::Relocation<>{addr_vtable_character},
                  offset_vtable_update,
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
    }
  };

  export auto install_hooks() -> void
  {
    auto& trampoline = SKSE::GetTrampoline();
    trampoline.create(512);

    character::install_hook();
  }
}
