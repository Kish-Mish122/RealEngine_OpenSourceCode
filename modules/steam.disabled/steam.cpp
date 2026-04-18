#include "steam.h"
#include "core/string/print_string.h"
#include "core/os/os.h"

SteamCore *SteamCore::singleton = nullptr;

SteamCore::SteamCore() {
    singleton = this;
}

SteamCore::~SteamCore() {
    shutdown();
    if (singleton == this) {
        singleton = nullptr;
    }
}

SteamCore *SteamCore::get_singleton() {
    return singleton;
}

void SteamCore::_bind_methods() {
    // Здесь мы "привязываем" C++ методы, чтобы они были видны в GDScript
    ClassDB::bind_method(D_METHOD("initialize"), &SteamCore::initialize);
    ClassDB::bind_method(D_METHOD("shutdown"), &SteamCore::shutdown);
    ClassDB::bind_method(D_METHOD("is_initialized"), &SteamCore::is_initialized);
    ClassDB::bind_method(D_METHOD("set_achievement", "achievement_name"), &SteamCore::set_achievement);
    ClassDB::bind_method(D_METHOD("store_stats"), &SteamCore::store_stats);
}

bool SteamCore::initialize() {
    if (steam_initialized) {
        return true;
    }

    print_line("[Steam] Инициализация Steam API...");

    // Проверяем, запущен ли Steam
    if (!SteamAPI_IsSteamRunning()) {
        print_line("[Steam] Steam не запущен. Интеграция со Steam отключена.");
        return false;
    }

    // Это и есть главная команда, которая активирует нашу "связь" со Steam
    steam_initialized = SteamAPI_Init();

    if (steam_initialized) {
        print_line("[Steam] Steam API успешно инициализирован!");
    } else {
        print_line("[Steam] Не удалось инициализировать Steam API.");
    }
    return steam_initialized;
}

void SteamCore::shutdown() {
    if (steam_initialized) {
        SteamAPI_Shutdown();
        steam_initialized = false;
        print_line("[Steam] Steam API отключен.");
    }
}

bool SteamCore::is_initialized() const {
    return steam_initialized;
}

bool SteamCore::set_achievement(const String &p_achievement_name) {
    if (!steam_initialized) {
        return false;
    }

    ISteamUserStats *steam_user_stats = SteamUserStats();
    if (!steam_user_stats) {
        return false;
    }

    // Преобразуем удобную строку Godot в понятную для Steam
    CharString achievement_cs = p_achievement_name.utf8();

    // Это главная команда для выдачи достижения!
    bool success = steam_user_stats->SetAchievement(achievement_cs.get_data());
    if (success) {
        print_line("[Steam] Достижение '" + p_achievement_name + "' активировано!");
    } else {
        print_line("[Steam] Ошибка при активации достижения '" + p_achievement_name + "'.");
    }
    return success;
}

bool SteamCore::store_stats() {
    if (!steam_initialized) {
        return false;
    }

    ISteamUserStats *steam_user_stats = SteamUserStats();
    if (!steam_user_stats) {
        return false;
    }

    // Эта команда отправляет все наши изменения (например, выданные достижения) в Steam.
    bool success = steam_user_stats->StoreStats();
    if (success) {
        print_line("[Steam] Статистика успешно отправлена в Steam.");
    } else {
        print_line("[Steam] Ошибка при отправке статистики.");
    }
    return success;
}
