module;

#include "library/PrismaUI_API.h"

export module TrueFlasks.UI.Prisma ;

import TrueFlasks.Core.ModsAPIRepository;

namespace ui::prisma::flasks_widget {

auto get_view_ref() -> PrismaView&
{
  static PrismaView view;
  return view;
}

auto create_view(PrismaView) -> void
{
  logger::info("View DOM is ready");
}

}

namespace ui::prisma {

export auto set_visible_menu(const bool visible) -> void
{
  auto prisma_ui = core::mods_api_repository::get_prisma_ui();
  if (!prisma_ui) {
    return;
  }
    
    const auto& view = flasks_widget::get_view_ref();
    visible ? prisma_ui->Show(view) : prisma_ui->Hide(view);
}

export auto initialize_ui() -> void
{

  auto prisma_ui = core::mods_api_repository::get_prisma_ui();

  if (!prisma_ui) {
    logger::warn("PrismaUI not installed when invoke initialize_ui");
    return;
  }

  constexpr auto index_path_to_flasks_widget_index = "TrueFlasksNG/FlasksWidgetIndex.html"sv;
  auto& flasks_view = flasks_widget::get_view_ref();
  flasks_view = prisma_ui->CreateView(index_path_to_flasks_widget_index.data(), flasks_widget::create_view);

  logger::info("PrismaUI initialized");
  prisma_ui->Hide(flasks_view);
  prisma_ui->Unfocus(flasks_view);

}

}
