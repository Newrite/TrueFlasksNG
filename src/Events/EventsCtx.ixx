export module TrueFlasks.Events.EventsCtx;

export namespace events::events_ctx {

struct process_event_menu_ctx final
{
  const RE::MenuOpenCloseEvent* menu_event;
  RE::BSTEventSource<RE::MenuOpenCloseEvent>* event_source;
  RE::BSFixedString menu_name;
  bool is_opening;
};

}
