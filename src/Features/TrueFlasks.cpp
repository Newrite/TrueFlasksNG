export module TrueFlasks.Features.TrueFlasks;

import TrueFlasks.Core.HooksCtx;
import TrueFlasks.Config;
import TrueFlasks.Core.ActorsCache;

namespace features::true_flasks
{

bool drink_potion(core::hooks_ctx::on_actor_drink_potion& ctx)
{
  
  const auto& actor_data = core::actors_cache::cache_data::get_singleton()->get_or_add(ctx.actor->GetFormID());
  const auto config = config::config_manager::get_singleton();
  
  return true;
}

}
