#include "discord_rpc.h"
#include "core/string/print_string.h"
#include "editor/editor_node.h"
#include "editor/settings/editor_settings.h"
#include "core/config/project_settings.h"
#include "scene/main/timer.h"

#ifdef WINDOWS_ENABLED
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <discord_rpc.h>
#endif

DiscordRPC *DiscordRPC::singleton = nullptr;

void DiscordRPC::_bind_methods() {
}

DiscordRPC::DiscordRPC() {
    singleton = this;
    enabled = false;
    discord_connected = false;
    start_time = 0;

#ifdef WINDOWS_ENABLED
    // Загружаем настройки
    if (EditorSettings::get_singleton()->has_setting("discord/enabled")) {
        enabled = EditorSettings::get_singleton()->get_setting("discord/enabled");
    }

    if (enabled) {
        callable_mp(this, &DiscordRPC::_delayed_init).call_deferred();
    }
#endif
}

DiscordRPC::~DiscordRPC() {
    shutdown();
    if (singleton == this) {
        singleton = nullptr;
    }
}

void DiscordRPC::_delayed_init() {
#ifdef WINDOWS_ENABLED
    // Проверяем наличие DLL
    HMODULE test = LoadLibraryA("discord-rpc.dll");
    if (!test) {
        print_line("[Discord] discord-rpc.dll not found");
        return;
    }
    FreeLibrary(test);

    initialize();
#endif
}

void DiscordRPC::initialize() {
#ifdef WINDOWS_ENABLED
    if (discord_connected) return;

    application_id = "1479336248308011168";

    DiscordEventHandlers handlers = {};
    Discord_Initialize(application_id.utf8().get_data(), &handlers, 1, nullptr);

    discord_connected = true;
    start_time = OS::get_singleton()->get_unix_time();

    // Запускаем таймер для обновления
    Timer *timer = memnew(Timer);
    timer->set_wait_time(15.0);
    timer->connect("timeout", callable_mp(this, &DiscordRPC::_update_callback));
    EditorNode::get_singleton()->add_child(timer);
    timer->start();

    print_line("[REAL DISCORD] Rich Presence initialized");
#endif
}

void DiscordRPC::shutdown() {
#ifdef WINDOWS_ENABLED
    if (!discord_connected) return;

    Discord_ClearPresence();
    Discord_Shutdown();
    discord_connected = false;
#endif
}

void DiscordRPC::update_presence(const String &p_state, const String &p_details) {
#ifdef WINDOWS_ENABLED
    if (!discord_connected || !enabled) return;

    DiscordRichPresence presence = {};
    presence.state = p_state.utf8().get_data();
    presence.details = p_details.utf8().get_data();
    presence.largeImageKey = "logo";

    String large_text = "Real Engine v" + String(VERSION_FULL_CONFIG);
    presence.largeImageText = large_text.utf8().get_data();

    presence.startTimestamp = start_time;

    if (!current_project.is_empty()) {
        presence.smallImageKey = "project";
        String small_text = "Project: " + current_project;
        presence.smallImageText = small_text.utf8().get_data();
    }

    Discord_UpdatePresence(&presence);
#endif
}

void DiscordRPC::update_project(const String &p_project) {
    current_project = p_project;

    if (!discord_connected || !enabled) return;

    String state = "Working on: " + (current_project.is_empty() ? "No project" : current_project);
    String details = current_scene.is_empty() ? "Idle" : "Editing: " + current_scene;

    update_presence(state, details);
}

void DiscordRPC::update_scene(const String &p_scene) {
    current_scene = p_scene;
    update_project(current_project);
}

void DiscordRPC::clear_presence() {
#ifdef WINDOWS_ENABLED
    if (!discord_connected) return;
    Discord_ClearPresence();
#endif
}

void DiscordRPC::set_enabled(bool p_enabled) {
    enabled = p_enabled;

    if (enabled && !discord_connected) {
        _delayed_init();
    } else if (!enabled && discord_connected) {
        shutdown();
    }

    EditorSettings::get_singleton()->set_setting("discord/enabled", enabled);
    EditorSettings::get_singleton()->save();
}

void DiscordRPC::_update_callback() {
#ifdef WINDOWS_ENABLED
    if (discord_connected) {
        Discord_RunCallbacks();
    }
#endif
}
