export module TrueFlasks.Events;

import TrueFlasks.Events.MenuEvent;

namespace events {

export auto register_events() -> void
{
  menu_event::menu_event_handler::register_handler();
}

}
