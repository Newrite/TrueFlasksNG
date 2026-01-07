module;

#include "library/PrismaUI_API.h"

export module TrueFlasks.Core.ModsAPIRepository;

namespace core::mods_api_repository {

export auto get_prisma_ui() -> PRISMA_UI_API::IVPrismaUI1*
{
  static PRISMA_UI_API::IVPrismaUI1* prisma_ui = nullptr;
  if (!prisma_ui) {
    prisma_ui = static_cast<PRISMA_UI_API::IVPrismaUI1*>(PRISMA_UI_API::RequestPluginAPI(
        PRISMA_UI_API::InterfaceVersion::V1));
    prisma_ui ? logger::info("Success request PrismaUI") : logger::info("Failed request PrismaUI");
  }
  return prisma_ui;
}
}
