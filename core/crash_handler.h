/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */
#pragma once

#include "core/string/ustring.h"

#ifdef WINDOWS_ENABLED
struct _EXCEPTION_POINTERS;
#endif

#define CRASH_HANDLER_VERSION "1.2"

class CrashHandler {
public:
    static void setup();
    static void add_log_line(const String &p_line);
    static void set_log_buffer_size(int p_size);
    static void shutdown();
    static void set_project_name(const String &p_name);     // новое
    static void set_is_editor(bool p_is_editor);           // новое

private:
    static void _save_crash_logs(const String &p_signal, const String &p_error_code);
    static void _crash_handler(int p_signal);
    static void _show_dialog(const String &p_signal, const String &p_error_code, const String &p_last_log);
    static void _open_email(const String &p_signal, const String &p_error_code, const String &p_last_log);
    static void _open_vk_play();

    static Vector<String> log_buffer;
    static int max_log_lines;
    static bool handler_installed;
    static bool normal_exit;
    static bool dialog_shown;
    static bool is_editor;              // true = редактор, false = игра
    static String project_name;         // имя проекта (только для игры)

#ifdef WINDOWS_ENABLED
    friend LONG WINAPI vectored_handler(_EXCEPTION_POINTERS *);
#endif
};
