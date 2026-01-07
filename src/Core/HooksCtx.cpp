export module TrueFlasks.Core.HooksCtx;

namespace core::hooks_ctx {

export struct on_actor_update final
{
  RE::Character* actor;
  float delta;
};

}
