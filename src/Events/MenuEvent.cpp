export module TrueFlasks.Events.MenuEvent;

import TrueFlasks.Events.EventsCtx;

namespace events::menu_event {

export struct menu_event_handler final : RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
    static auto get_singleton() -> menu_event_handler*
    {
        static menu_event_handler singleton;
        return std::addressof(singleton);
        
    }  
    
  static auto register_handler() -> void
  {
    logger::info("Start register menu open close handler"sv);
    if (const auto ui = RE::UI::GetSingleton()) {
      ui->AddEventSink(get_singleton());
      logger::info("Finish register menu open close handler"sv);
    }
  }

  auto ProcessEvent(
      const RE::MenuOpenCloseEvent* menu_event,
      RE::BSTEventSource<RE::MenuOpenCloseEvent>* event_source) -> RE::BSEventNotifyControl override
  {
    if (!menu_event || !event_source) {
      return RE::BSEventNotifyControl::kContinue;
    }

    auto ctx = events_ctx::process_event_menu_ctx{menu_event, event_source, menu_event->menuName, menu_event->opening};

    return RE::BSEventNotifyControl::kContinue;
  }
    
};
}
