export module TrueFlasks.Core.HooksCtx;

namespace core::hooks_ctx
{
  export struct on_actor_update final
  {
    RE::Character* actor;
    float delta;
  };

  // ExtraList может быть нуллом, т.к. далеко не все объекты несут в себе дополнительную рантайм информацию
  export struct on_actor_drink_potion final
  {
    RE::Character* actor;
    RE::AlchemyItem* potion;
    RE::ExtraDataList* extra_list;
  };

  export struct on_actor_remove_item final
  {
    RE::Character* actor;
    RE::TESBoundObject* item;
    std::int32_t& count;
    RE::ITEM_REMOVE_REASON reason;
    RE::TESObjectREFR* move_to_ref;
    const RE::NiPoint3* drop_loc;
    const RE::NiPoint3* rotate;
  };
}
