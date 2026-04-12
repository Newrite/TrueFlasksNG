export module TrueFlasks.Events.EventsCtx;

export namespace events::events_ctx
{
  struct process_event_menu_ctx final
  {
    const RE::MenuOpenCloseEvent* menu_event;
    RE::BSTEventSource<RE::MenuOpenCloseEvent>* event_source;
    RE::BSFixedString menu_name;
    bool is_opening;
  };
  
  struct process_event_input_ctx final
  {
    RE::InputEvent* const* event;
    RE::BSTEventSource<RE::InputEvent*>* event_source;
    RE::ButtonEvent* const button_event;
    RE::INPUT_DEVICE device;
    uint32_t key;
  };
  
}
