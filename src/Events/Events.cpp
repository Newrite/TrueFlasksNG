export module TrueFlasks.Events;

import TrueFlasks.Events.InputEvent;
import TrueFlasks.Events.MenuEvent;

namespace events
{
  export auto register_events() -> void
  {
    input_event::input_event_handler::register_handler();
    menu_event::menu_event_handler::register_handler();
  }
}
