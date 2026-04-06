/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#include "real_coffee.h"
#include "editor/editor_node.h"
#include "editor/settings/editor_settings.h"
#include "core/string/print_string.h"

#ifdef WINDOWS_ENABLED
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")
#endif

void RealCoffee::_bind_methods() {
    ClassDB::bind_method(D_METHOD("start_timer"), &RealCoffee::start_timer);
    ClassDB::bind_method(D_METHOD("stop_timer"), &RealCoffee::stop_timer);
    ClassDB::bind_method(D_METHOD("restart_timer"), &RealCoffee::restart_timer);
    ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &RealCoffee::set_enabled);
    ClassDB::bind_method(D_METHOD("is_enabled"), &RealCoffee::is_enabled);
    ClassDB::bind_method(D_METHOD("set_interval", "minutes"), &RealCoffee::set_interval);
    ClassDB::bind_method(D_METHOD("get_interval"), &RealCoffee::get_interval);
    ClassDB::bind_method(D_METHOD("_on_timer_timeout"), &RealCoffee::_on_timer_timeout);
}

RealCoffee::RealCoffee() {
    _load_settings();
}

RealCoffee::~RealCoffee() {
    if (coffee_timer) {
        coffee_timer->stop();
        memdelete(coffee_timer);
    }
}

void RealCoffee::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_READY: {
            if (enabled) {
                start_timer();
            }
        } break;

        case NOTIFICATION_EXIT_TREE: {
            stop_timer();
        } break;
    }
}

void RealCoffee::_load_settings() {
    print_line("[REAL COFFEE]: Loading Settings...");
    EditorSettings *settings = EditorSettings::get_singleton();
    if (!settings) return;

    if (settings->has_setting("real_coffee/enabled")) {
        enabled = settings->get("real_coffee/enabled");
    }
    if (settings->has_setting("real_coffee/interval")) {
        interval_minutes = settings->get("real_coffee/interval");
    }
    if (settings->has_setting("real_coffee/show_notifications")) {
        show_notifications = settings->get("real_coffee/show_notifications");
    }
}

void RealCoffee::start_timer() {
    if (!enabled) return;

    if (!coffee_timer) {
        coffee_timer = memnew(Timer);
        add_child(coffee_timer);
        coffee_timer->set_one_shot(false);
        coffee_timer->connect("timeout", callable_mp(this, &RealCoffee::_on_timer_timeout));
    }

    coffee_timer->set_wait_time(interval_minutes * 60);
    coffee_timer->start();
}

void RealCoffee::stop_timer() {
    if (coffee_timer) {
        coffee_timer->stop();
    }
}

void RealCoffee::restart_timer() {
    stop_timer();
    start_timer();
}

void RealCoffee::set_enabled(bool p_enabled) {
    enabled = p_enabled;
    if (enabled) {
        start_timer();
    } else {
        stop_timer();
    }

    EditorSettings::get_singleton()->set("real_coffee/enabled", enabled);
}

void RealCoffee::set_interval(int p_minutes) {
    interval_minutes = p_minutes;
    if (coffee_timer && coffee_timer->is_inside_tree()) {
        coffee_timer->set_wait_time(interval_minutes * 60);
        coffee_timer->start();
    }

    EditorSettings::get_singleton()->set("real_coffee/interval", interval_minutes);
}

void RealCoffee::_on_timer_timeout() {
    _show_notification();
}

void RealCoffee::_show_notification() {
    if (!show_notifications) return;


#ifdef WINDOWS_ENABLED
    // Get window handle
    HWND hwnd = (HWND)DisplayServer::get_singleton()->window_get_native_handle(DisplayServer::WINDOW_HANDLE);

    // Simple message box for now (easier than toast notifications)
    MessageBoxW(hwnd,
                L"Take a short break and grab some coffee.",
                L"Real Engine - Take a Break",
                MB_OK | MB_ICONINFORMATION);
#endif
}
