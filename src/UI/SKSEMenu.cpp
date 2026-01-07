module;

#include "library/SKSEMenuFramework.h"

export module TrueFlasks.UI.SKSEMenu;

namespace ui::skse_menu {

using namespace ImGuiMCP;
export auto register_skse_menu() -> void
{

  if (!SKSEMenuFramework::IsInstalled()) {
    logger::warn("SKSEMenuFramework not installed");
    return;
  }

  static constexpr auto main_title = "True Flasks NG";
  SKSEMenuFramework::SetSection(main_title);

}


}
