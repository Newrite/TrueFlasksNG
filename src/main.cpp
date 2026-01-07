import TrueFlasks.Core.LoggerSetup;
import TrueFlasks.UI.Prisma;
import TrueFlasks.UI.SKSEMenu;
import TrueFlasks.Events;
import TrueFlasks.Core.Hooks;

auto skse_message_handle(SKSE::MessagingInterface::Message* message) -> void
{
  switch (message->type) {
    case SKSE::MessagingInterface::kPostLoad: {
      break;
    }
    case SKSE::MessagingInterface::kPostPostLoad: {
      break;
    }
    case SKSE::MessagingInterface::kInputLoaded: {
      break;
    }
    case SKSE::MessagingInterface::kDataLoaded: {
      core::hooks::install_hooks();
      ui::prisma::initialize_ui();
      events::register_events();
      break;
    }
    case SKSE::MessagingInterface::kNewGame:
    case SKSE::MessagingInterface::kPreLoadGame:
    case SKSE::MessagingInterface::kPostLoadGame:
    case SKSE::MessagingInterface::kSaveGame:
    case SKSE::MessagingInterface::kDeleteGame:
    default:
      break;
  }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{

  core::logger_setup::setup_log();
    
    const auto plugin = SKSE::PluginDeclaration::GetSingleton();
    logger::info("{} v{} is loading...", plugin->GetName(), plugin->GetVersion());


  SKSE::Init(skse);

  SKSE::GetMessagingInterface()->RegisterListener(skse_message_handle);
  ui::skse_menu::register_skse_menu();

  logger::info("{} has finished loading.", plugin->GetName());

  return true;

}
