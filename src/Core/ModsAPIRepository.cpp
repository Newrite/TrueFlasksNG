module;

#include "library/PrismaUI_API.h"

export module TrueFlasks.Core.ModsAPIRepository;

namespace core::mods_api_repository
{
  struct prisma_api_holder
  {
    PRISMA_UI_API::IVPrismaUI1* v1{nullptr};
    PRISMA_UI_API::IVPrismaUI2* v2{nullptr};
    bool requested{false};
  };

  auto get_holder() -> prisma_api_holder&
  {
    static prisma_api_holder holder;
    if (!holder.requested) {
      holder.requested = true;

      holder.v2 = PRISMA_UI_API::RequestPluginAPI<PRISMA_UI_API::IVPrismaUI2>();
      if (holder.v2) {
        holder.v1 = holder.v2;
        logger::info("Success request PrismaUI (V2 interface)");
      }
      else {
        holder.v1 = PRISMA_UI_API::RequestPluginAPI<PRISMA_UI_API::IVPrismaUI1>();
        holder.v1
          ? logger::info("Success request PrismaUI (V1 interface, console callback unavailable)")
          : logger::info("Failed request PrismaUI");
      }
    }
    return holder;
  }

  export auto get_prisma_ui() -> PRISMA_UI_API::IVPrismaUI1*
  {
    return get_holder().v1;
  }

  export auto get_prisma_ui_v2() -> PRISMA_UI_API::IVPrismaUI2*
  {
    return get_holder().v2;
  }
}
