#include "register_types.h"
#include "steam.h"
#include "core/object/class_db.h"

static SteamCore *steam_core = nullptr;

void initialize_steam_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    steam_core = memnew(SteamCore);
    print_line("[Steam] Модуль Steam загружен.");
}

void uninitialize_steam_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    if (steam_core) {
        memdelete(steam_core);
        steam_core = nullptr;
    }
    print_line("[Steam] Модуль Steam выгружен.");
}
