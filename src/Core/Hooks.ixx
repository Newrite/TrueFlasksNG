export module TrueFlasks.Core.Hooks;

import TrueFlasks.Core.HooksCtx;

namespace core::hooks {

template<typename T>
auto write_call(SKSE::Trampoline& trampoline, const uintptr_t src, T& func, REL::Relocation<T>& func_original, const char* label) -> void
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

STATIC_STRUCT(character)

  static constexpr auto addr_vtable_character = RE::Character::VTABLE[0];
  static constexpr auto addr_vtable_player_character = RE::PlayerCharacter::VTABLE[0];
  static constexpr auto offset_vtable_update = RELOCATION_OFFSET(0xAD, 0xAD);

  static auto on_update(RE::Character* character, const float delta) -> void
  {
    auto ctx = hooks_ctx::on_actor_update{character, delta};
  }

  static auto on_update_character(RE::Character* character, const float delta) -> void
  {
    if (!character || !delta) {
      return on_update_character_original(character, delta);
    }

    on_update(character, delta);

    return on_update_character_original(character, delta);
  }

  static auto on_update_player_character(RE::PlayerCharacter* character, const float delta) -> void
  {
    if (!character || !delta) {
      return on_update_player_character_original(character, delta);
    }

    on_update(character, delta);

    return on_update_player_character_original(character, delta);
  }

  static inline REL::Relocation<decltype(on_update_character)> on_update_character_original;
  static inline REL::Relocation<decltype(on_update_player_character)> on_update_player_character_original;

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
  }

};

export auto install_hooks() -> void
{
  auto& trampoline = SKSE::GetTrampoline();
  trampoline.create(512);

  character::install_hook();
}

}
