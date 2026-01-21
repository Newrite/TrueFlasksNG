# TrueFlasksNG SKSE API

TrueFlasksNG exports an interface that allows other SKSE plugins to interact with the flask system directly.

### How to Request the Interface

You can request the `TrueFlasksIVTrueFlasks1` interface via the SKSE messaging system.

```cpp
// In your SKSE plugin load or message handler:
void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kPostLoad) {
        // Request the interface from TrueFlasksNG
        // Note: Check source for exact Message ID (e.g. 0x10)
        auto* trueFlasks = (IVTrueFlasks1*)SKSE::GetMessagingInterface()->GetEventDispatcher(
            SKSE::MessagingInterface::kType_GameLoaded 
        )->RequestPluginInterface("TrueFlasksNG");

        if (trueFlasks) {
            // Interface obtained successfully
        }
    }
}
```

### Interface Definition: IVTrueFlasks1

Grab and explore API header from repo `src\API\TrueFlasksAPI.h`

### Create Your Own UI
While the mod comes with a ready-to-use Prisma UI widget, you are not bound to it.
* **Full Data Access:** API provide real-time access to current charges, max slots, and cooldown progress.
* **Freedom:** You can build a completely custom widget (SWF, ImGui) that fits your specific modlist's aesthetic, or integrate the flask status into existing HUD mods seamlessly.
