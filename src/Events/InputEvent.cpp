export module TrueFlasks.Events.InputEvent;

import TrueFlasks.Events.EventsCtx;
import TrueFlasks.Features.TrueFlasks;

namespace events::input_event {

  export struct input_event_handler final : RE::BSTEventSink<RE::InputEvent*>
  {
    
    static auto get_singleton() -> input_event_handler*
    {
      static input_event_handler singleton;
      return std::addressof(singleton);
        
    }  
    
    static auto register_handler() -> void
    {
      const auto device_manager = RE::BSInputDeviceManager::GetSingleton();
      logger::info("Start register input event handler"sv);
      if (device_manager) {
        device_manager->AddEventSink(get_singleton());
        logger::info("Finish register input event handler"sv);
      }
    }

    auto ProcessEvent(RE::InputEvent* const* event,
                      RE::BSTEventSource<RE::InputEvent*>* event_source)
      -> RE::BSEventNotifyControl override
    {
      for (auto input_event = *event; input_event; input_event = input_event->next) {
        if (const auto button = input_event->AsButtonEvent(); button) {
          const auto device = input_event->GetDevice();

          auto key = button->GetIDCode();

          switch (device) {
          case RE::INPUT_DEVICE::kMouse:
            key += SKSE::InputMap::kMacro_MouseButtonOffset;
            break;
          case RE::INPUT_DEVICE::kGamepad:
            key = SKSE::InputMap::GamepadMaskToKeycode(key);
            break;
          default:
            break;
          }

          auto ctx = events_ctx::process_event_input_ctx{event, event_source, button, device, key};
          features::true_flasks::on_input_event(ctx);
        }
      }

      return RE::BSEventNotifyControl::kContinue;
    }
  };
  
}
