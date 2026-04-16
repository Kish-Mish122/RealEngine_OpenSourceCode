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

#include <commctrl.h>

#include "crash_handler.h"
#include "core/os/os.h"
#include "core/string/print_string.h"

#include "core/version.h"

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
    FILE *f = fopen("C:\\Windows\\temp\\crash_handler_log.txt", "w");
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

    TerminateProcess(GetCurrentProcess(), 1);

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
    print_line("[REAL CRASH HANDLER]: Shutdown...");
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

    if (normal_exit) {
        exit(0);
    }
    if (dialog_shown) return;
    dialog_shown = true;

    // Собираем логи, ограничивая количество строк (например, последние 200)
    String last_log = "";
    int max_lines = 200;
    int start = 0;
    if (log_buffer.size() > max_lines) {
        start = log_buffer.size() - max_lines;
        last_log = "[... " + itos(log_buffer.size() - max_lines) + " lines omitted]\n";
    }
    for (int i = start; i < log_buffer.size(); i++) {
        last_log += log_buffer[i] + "\n";
    }

    // Для отладки: записываем логи в файл
    FILE *f = fopen("C:\\Windows\\temp\\crash_log_dump.txt", "w");
    if (f) {
        fprintf(f, "=== CRASH LOG DUMP ===\n%s", last_log.utf8().get_data());
        fclose(f);
    }

    String signal_str = "Signal " + itos(p_signal);
    String error_code = "0x" + String::num_int64(p_signal, 16);
    _show_dialog(signal_str, error_code, last_log);
    exit(1);
}

void CrashHandler::_show_dialog(const String &p_signal, const String &p_error_code, const String &p_last_log) {
#ifdef WINDOWS_ENABLED
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // Загружаем иконку из файла (путь относительно EXE)
    HICON hIcon = NULL;
    // Пытаемся загрузить иконку из файла "crash_logo.ico" в папке с движком
    // Используем максимальный размер 48x48 для диалога
    hIcon = (HICON)LoadImageA(GetModuleHandleA(NULL), "crash_handler_logo.ico", IMAGE_ICON, 48, 48, LR_LOADFROMFILE);
    if (!hIcon) {
        // Если не загрузилось, пробуем другой путь (например, из папки с ресурсами)
        // Здесь можно указать абсолютный путь или встроенный ресурс
        // Если иконки нет, оставляем NULL
    }

    String version = "Real Engine " + String(VERSION_FULL_CONFIG);
    String mainInstruction = version + " - Stopped working!";
    String content = "It's a pity, but the Real Engine crashed!\n";
    content += "Signal: " + p_signal + "\n";
    content += "Error code: " + p_error_code + "\n\n";
    content += "Click 'Send report' to open your email client with pre-filled details.\n";
    content += "Click 'Close' to exit the engine.";
    String expandedInfo = "Last log before crash:\n" + p_last_log;

    TASKDIALOG_BUTTON buttons[] = {
        { 100, L"Close without sending" },
        { 101, L"Send a bug and close" }
    };

    TASKDIALOGCONFIG config = {0};
    config.cbSize = sizeof(TASKDIALOGCONFIG);
    config.hwndParent = NULL;
    config.dwFlags = TDF_USE_COMMAND_LINKS | TDF_ALLOW_DIALOG_CANCELLATION | TDF_EXPAND_FOOTER_AREA;
    if (hIcon) {
        config.dwFlags |= TDF_USE_HICON_MAIN;  // используем HICON вместо системной иконки
        config.hMainIcon = hIcon;
    } else {
        config.pszMainIcon = TD_ERROR_ICON;    // запасной вариант
    }
    config.dwCommonButtons = 0;
    config.pButtons = buttons;
    config.cButtons = 2;
    config.nDefaultButton = 100;
    config.pszWindowTitle = L"Real Engine Crash";

    Char16String mainInstStr = mainInstruction.utf16();
    Char16String contentStr = content.utf16();
    Char16String expandedStr = expandedInfo.utf16();

    config.pszMainInstruction = reinterpret_cast<PCWSTR>(mainInstStr.get_data());
    config.pszContent = reinterpret_cast<PCWSTR>(contentStr.get_data());
    config.pszExpandedInformation = reinterpret_cast<PCWSTR>(expandedStr.get_data());

    int nButtonPressed = 0;
    HRESULT hr = TaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);

    if (SUCCEEDED(hr) && nButtonPressed == 101) {
        _open_email(p_signal, p_error_code, p_last_log);
    }

    // Освобождаем иконку, если она была загружена
    if (hIcon) {
        DestroyIcon(hIcon);
    }

    TerminateProcess(GetCurrentProcess(), 1);
#endif
}

void CrashHandler::_open_email(const String &p_signal, const String &p_error_code, const String &p_last_log) {
#ifdef WINDOWS_ENABLED
    String subject = "Real Engine v." + String(VERSION_FULL_CONFIG) + " - Bug Report.";
    String body = "The Real Engine was shut down for my reason or yours!\n\n";
    body += "Signal: " + p_signal + "\n";
    body += "Error code: " + p_error_code + "\n";
    body += "Last log before crash:\n\n" + p_last_log + "\n\n";

    String mailto = "mailto:help.k1shm1sh@gmail.com?subject=" + subject + "&body=" + body;
    mailto = mailto.replace(" ", "%20");
    mailto = mailto.replace("\n", "%0A");
    ShellExecuteA(NULL, "open", mailto.utf8().get_data(), NULL, NULL, SW_SHOWNORMAL);
#endif
}
