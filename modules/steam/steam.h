#ifndef STEAM_MODULE_H
#define STEAM_MODULE_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"

// Эта магическая строка помогает избежать конфликта имен на Windows
#ifdef _WIN32
#define _WINSOCKAPI_
#endif

// Подключаем заголовочные файлы из Steamworks SDK, который мы скачали.
// Путь указан относительно папки `modules/steam`
#include "../steam/steam/steam_api_common.h"
#include "../steam/steam/steam_api.h"
#include "../steam/steam/isteamuserstats.h"

class SteamCore : public RefCounted {
    GDCLASS(SteamCore, RefCounted);

private:
    static SteamCore *singleton;
    bool steam_initialized = false;

protected:
    static void _bind_methods();

public:
    SteamCore();
    ~SteamCore();

    static SteamCore *get_singleton();

    // Методы, которые мы будем вызывать из движка
    bool initialize();
    void shutdown();
    bool is_initialized() const;

    // Метод для выдачи достижения
    bool set_achievement(const String &p_achievement_name);
    bool store_stats();
};

#endif // STEAM_MODULE_H
