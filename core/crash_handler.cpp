/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#ifdef WINDOWS_ENABLED
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
// Убираем повторное определение _WIN32_WINNT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <windows.h>
#include <shellapi.h>
#include <signal.h>

#ifdef CONNECT_DEFERRED
#undef CONNECT_DEFERRED
#endif
#ifdef ERROR
#undef ERROR
#endif
#ifdef IGNORE
#undef IGNORE
#endif
#endif

#include "crash_handler.h"
#include "core/os/os.h"
#include "core/string/print_string.h"

// Инициализация статических членов
Vector<String> CrashHandler::log_buffer;
int CrashHandler::max_log_lines = 100;
bool CrashHandler::handler_installed = false;
bool CrashHandler::normal_exit = false; // Это фалг, что программа завершилась нормально, а не аварийно
bool CrashHandler::dialog_shown = false;

#ifdef WINDOWS_ENABLED
LONG WINAPI vectored_handler(EXCEPTION_POINTERS *p_exception_info) {
    DWORD code = p_exception_info->ExceptionRecord->ExceptionCode;

    // Игнорируем эту исключения, тк они возникают при входе в программу и не несут полезной информации.
    if (code == EXCEPTION_BREAKPOINT ||
        code == EXCEPTION_SINGLE_STEP ||
        code == EXCEPTION_GUARD_PAGE ||
        code == EXCEPTION_INVALID_HANDLE ||
        code == 0x4001000A ||   // DBG_PRINTEXCEPTION_WIDE_C
        code == 0x40010006) {   // DBG_PRINTEXCEPTION_C
        return EXCEPTION_CONTINUE_SEARCH;
    }

    if (CrashHandler::normal_exit || CrashHandler::dialog_shown) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    CrashHandler::dialog_shown = true;

    // Запись в файл для отладки
    FILE *f = fopen("C:\\temp\\crash_handler_log.txt", "w");
    if (f) {
        fprintf(f, "Vectored handler called. Code: 0x%08lx\n",
                p_exception_info->ExceptionRecord->ExceptionCode);
        fclose(f);
    }

    String signal_str = "Exception 0x" + String::num_int64(p_exception_info->ExceptionRecord->ExceptionCode, 16);
    String error_code = "Code: " + itos(p_exception_info->ExceptionRecord->ExceptionCode);
    String last_log = "";
    for (int i = 0; i < CrashHandler::log_buffer.size(); i++) {
        last_log += CrashHandler::log_buffer[i] + "\n";
    }
    CrashHandler::_show_dialog(signal_str, error_code, last_log);

    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

void CrashHandler::setup() {
    if (handler_installed) return;
#ifdef WINDOWS_ENABLED
    AddVectoredExceptionHandler(1, vectored_handler);
    signal(SIGABRT, _crash_handler);
    signal(SIGSEGV, _crash_handler);
    signal(SIGILL, _crash_handler);
    signal(SIGFPE, _crash_handler);
    signal(SIGTERM, _crash_handler);
#endif
    handler_installed = true;
    print_line("[REAL CRASH HANDLER]: Ready to Work!");
}

void CrashHandler::shutdown() {
    normal_exit = true;
}

void CrashHandler::add_log_line(const String &p_line) {
    log_buffer.push_back(p_line);
    while (log_buffer.size() > max_log_lines) {
        log_buffer.remove_at(0);
    }
}

void CrashHandler::set_log_buffer_size(int p_size) {
    max_log_lines = p_size;
}

void CrashHandler::_crash_handler(int p_signal) {
    print_line("-----------------------------");
    print_line("REAL ENGINE IS CRASHED!");
    print_line("SIGNAL: " + itos(p_signal));
    print_line("ERR CODE: 0x" + String::num_int64(p_signal, 16));
    print_line("REAL ENGINE IS CRASHED! WAIT...");
    print_line("-----------------------------");
    /* ----------------------------------- */

    if (normal_exit) {
        exit(0);
    }
    if (dialog_shown) return;
    dialog_shown = true;

    String signal_str = "Signal " + itos(p_signal);
    String error_code = "0x" + String::num_int64(p_signal, 16);
    String last_log = "";
    for (int i = 0; i < log_buffer.size(); i++) {
        last_log += log_buffer[i] + "\n";
    }
    _show_dialog(signal_str, error_code, last_log);
    exit(1);
}

void CrashHandler::_show_dialog(const String &p_signal, const String &p_error_code, const String &p_last_log) {
#ifdef WINDOWS_ENABLED
    String message = "Real Engine - Emergency stop!\n\n";
    message += "Please report your problem to K1sh-M1sh.\n\n";
    message += "Signal: " + p_signal + "\n";
    message += "Error code: " + p_error_code + "\n";
    message += "Last log before crash:\n" + p_last_log;
    message += "Send a report to the developer?\n\n";
    message += "The No button closes this window.";

    int result = MessageBoxA(NULL, message.utf8().get_data(), "Real Engine Crash",
                              MB_ICONERROR | MB_YESNO | MB_SERVICE_NOTIFICATION | MB_TOPMOST);
    if (result == IDYES) {
        _open_email(p_signal, p_error_code, p_last_log);
    }
#endif
}

void CrashHandler::_open_email(const String &p_signal, const String &p_error_code, const String &p_last_log) {
#ifdef WINDOWS_ENABLED
    String subject = "Real Engine Crash Report";
    String body = "Real Engine has crashed!\n\n";
    body += "Signal: " + p_signal + "\n";
    body += "Error code: " + p_error_code + "\n";
    body += "Last log before crash:\n" + p_last_log + "\n\n";

    String mailto = "mailto:help.k1shm1sh@gmail.com?subject=" + subject + "&body=" + body;
    mailto = mailto.replace(" ", "%20");
    mailto = mailto.replace("\n", "%0A");
    ShellExecuteA(NULL, "open", mailto.utf8().get_data(), NULL, NULL, SW_SHOWNORMAL);
#endif
}
