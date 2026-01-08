export module TrueFlasks.Core.HooksCtx;

namespace core::hooks_ctx
{
  export struct on_actor_update final
  {
    RE::Character* actor;
    float delta;
  };

  export struct on_actor_drink_potion final
  {
    RE::Character* actor;
    RE::AlchemyItem* potion;
    RE::ExtraDataList* extra_list;
  };
}
