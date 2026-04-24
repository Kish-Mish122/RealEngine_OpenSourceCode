/* Real Engine - K1sh-M1sh Studio */
/* License - MIT */

#ifdef WINDOWS_ENABLED
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <windows.h>
#include <shellapi.h>
#include <signal.h>
#include <commctrl.h>

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
#include "core/version.h"
#include <shlobj.h>
#include <time.h>
#include <sys/stat.h>

// Инициализация статических членов
Vector<String> CrashHandler::log_buffer;
int CrashHandler::max_log_lines = 100;
bool CrashHandler::handler_installed = false;
bool CrashHandler::normal_exit = false;
bool CrashHandler::dialog_shown = false;
bool CrashHandler::is_editor = false;
String CrashHandler::project_name = "";

#ifdef WINDOWS_ENABLED

// Поиск главного окна текущего процесса
static HWND find_main_window() {
    DWORD process_id = GetCurrentProcessId();
    HWND hwnd = NULL;
    do {
        hwnd = FindWindowEx(NULL, hwnd, NULL, NULL);
        DWORD window_pid = 0;
        GetWindowThreadProcessId(hwnd, &window_pid);
        if (window_pid == process_id) {
            LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
            if (style & WS_OVERLAPPEDWINDOW) {
                return hwnd;
            }
        }
    } while (hwnd != NULL);
    return NULL;
}

// Скрытие главного окна перед показом диалога
static void hide_main_window() {
    HWND hwnd = find_main_window();
    if (hwnd) {
        ShowWindow(hwnd, SW_HIDE);
    }
}

LONG WINAPI vectored_handler(EXCEPTION_POINTERS *p_exception_info) {
    DWORD code = p_exception_info->ExceptionRecord->ExceptionCode;

    if (code == EXCEPTION_BREAKPOINT ||
        code == EXCEPTION_SINGLE_STEP ||
        code == EXCEPTION_GUARD_PAGE ||
        code == EXCEPTION_INVALID_HANDLE ||
        code == 0x4001000A ||
        code == 0x40010006) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    if (CrashHandler::normal_exit || CrashHandler::dialog_shown) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    CrashHandler::dialog_shown = true;

    hide_main_window();

    String signal_str = "Exception 0x" + String::num_int64(code, 16);
    String error_code = "Code: " + itos(code);
    String last_log = "";
    for (int i = 0; i < CrashHandler::log_buffer.size(); i++) {
        last_log += CrashHandler::log_buffer[i] + "\n";
    }

    CrashHandler::_save_crash_logs(signal_str, error_code);
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
    add_log_line("[REAL CRASH HANDLER]: Ready to Work!");
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
    // Real Engine Log:
    print_line("-----------------------------");
    print_line("REAL ENGINE IS CRASHED!");
    print_line("SIGNAL: " + itos(p_signal));
    print_line("ERR CODE: 0x" + String::num_int64(p_signal, 16));
    print_line("REAL ENGINE IS CRASHED! WAIT TO DIALOG...");
    print_line("-----------------------------");

    /* --------------------------------------------------------------- */

    // Real Engine Crash Handler Log:
    add_log_line("-----------------------------");
    add_log_line("REAL ENGINE IS CRASHED!");
    add_log_line("SIGNAL: " + itos(p_signal));
    add_log_line("ERR CODE: 0x" + String::num_int64(p_signal, 16));
    add_log_line("REAL ENGINE IS CRASHED! WAIT TO DIALOG...");
    add_log_line("-----------------------------");

    if (normal_exit) {
        exit(0);
    }
    if (dialog_shown) return;
    dialog_shown = true;

    // Скрываем главное окно
#ifdef WINDOWS_ENABLED
    hide_main_window();
#endif

    String signal_str = "Signal " + itos(p_signal);
    String error_code = "0x" + String::num_int64(p_signal, 16);
    String last_log = "";
    for (int i = 0; i < log_buffer.size(); i++) {
        last_log += log_buffer[i] + "\n";
    }
    _show_dialog(signal_str, error_code, last_log);
    exit(1);

    _save_crash_logs(signal_str, error_code);
}

void CrashHandler::_show_dialog(const String &p_signal, const String &p_error_code, const String &p_last_log) {
#ifdef WINDOWS_ENABLED
    // Инициализация общих элементов управления
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    HICON hIcon = NULL;
    hIcon = (HICON)LoadImageA(GetModuleHandleA(NULL), "crash_handler_logo.ico", IMAGE_ICON, 48, 48, LR_LOADFROMFILE);

    String version = "Real Engine " + String(VERSION_FULL_CONFIG);
    String mainInstruction;
    String content;
    String expandedInfo = "Last log before crash:\n" + p_last_log;

    if (is_editor) {
        // Режим редактора
        mainInstruction = version + " - Crash Handler v" CRASH_HANDLER_VERSION " detected a crash!";
        content = "Review the logs shown below. Your code may have crashed the Real Engine.\n";
        content += "If the error was not your fault, then inform the Real Engine developer about it.\n\n";
    } else {
        // Режим игры
        String proj = project_name.is_empty() ? "Unknown Project" : project_name;
        mainInstruction = "Project \"" + proj + "\" has encountered an error and needs to close.";
        content = "We apologize for the inconvenience.\n";
        content += "Please report this problem to the project's developer.\n\n";
    }

    content += "Signal: " + p_signal + "\n";
    content += "Error code: " + p_error_code + "\n\n";

    TASKDIALOG_BUTTON buttons[] = {
        { 100, L"Close without report a bug" },
        { 101, L"Report a bug and close " },
        { 102, L"Open VK Play News" }
    };

    TASKDIALOGCONFIG config = {0};
    config.cbSize = sizeof(TASKDIALOGCONFIG);
    config.hwndParent = NULL;
    config.dwFlags = TDF_USE_COMMAND_LINKS | TDF_ALLOW_DIALOG_CANCELLATION | TDF_EXPAND_FOOTER_AREA;
    if (hIcon) {
        config.dwFlags |= TDF_USE_HICON_MAIN;
        config.hMainIcon = hIcon;
    } else {
        config.pszMainIcon = TD_ERROR_ICON;
    }
    config.dwCommonButtons = 0;
    config.pButtons = buttons;
    config.cButtons = 3;
    config.nDefaultButton = 100;
    config.pszWindowTitle = L"Crash Handler For Real Engine";

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

    if (SUCCEEDED(hr) && nButtonPressed == 102) {
        _open_vk_play();
    }

    if (hIcon) {
        DestroyIcon(hIcon);
    }

    TerminateProcess(GetCurrentProcess(), 1);
#endif
}

void CrashHandler::_open_email(const String &p_signal, const String &p_error_code, const String &p_last_log) {
#ifdef WINDOWS_ENABLED
    String subject = "Real Engine v." + String(VERSION_FULL_CONFIG) + " Crash Report"; // Тема письма
    String body = "Real Engine v." + String(VERSION_FULL_CONFIG) + "has crashed!\nCrash Handler v." + CRASH_HANDLER_VERSION; // Главное в описании
    body += "Signal: " + p_signal + "\n"; // Описание
    body += "Error code: " + p_error_code + "\n"; // Описание
    body += "Last log before crash:\n" + p_last_log + "\n"; // Описание

    String mailto = "mailto:help.k1shm1sh@gmail.com?subject=" + subject + "&body=" + body; // Ссылка на mailto с Body
    mailto = mailto.replace(" ", "%20");
    mailto = mailto.replace("\n", "%0A");
    ShellExecuteA(NULL, "open", mailto.utf8().get_data(), NULL, NULL, SW_SHOWNORMAL);
#endif
}

void CrashHandler::_open_vk_play() {
#ifdef WINDOWS_ENABLED
    ShellExecuteA(NULL, "open", "https://community.vkplay.ru/community/game/real-engine-46163/", NULL, NULL, SW_SHOWNORMAL);
#endif
}

void CrashHandler::_save_crash_logs(const String &p_signal, const String &p_error_code) {
#ifdef WINDOWS_ENABLED
    PWSTR docs = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &docs))) {
        String base = String::utf16((const char16_t*)docs) + "/RealEngine/CrashLogs";
        CoTaskMemFree(docs);
        CreateDirectoryW((LPCWSTR)base.utf16().get_data(), NULL);

        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char stamp[64];
        strftime(stamp, sizeof(stamp), "%Y-%m-%d_%H-%M-%S", tm_info);
        String fname = base + "/crash_" + stamp + ".log";

        FILE *f = fopen(fname.utf8().get_data(), "w");
        if (f) {
            fprintf(f, "Real Engine v.%s\n", VERSION_FULL_CONFIG);
            fprintf(f, "Crash Handler v.%s\n", CRASH_HANDLER_VERSION);
            fprintf(f, "-------------------------------\n\n");
            fprintf(f, "Version: %s\n", VERSION_FULL_CONFIG);
            fprintf(f, "Signal: %s\n", p_signal.utf8().get_data());
            fprintf(f, "Error code: %s\n", p_error_code.utf8().get_data());
            fprintf(f, "Time: %s\n", stamp);
            fprintf(f, "-------------------------------\n\n");
            fprintf(f, "Last Logs:\n");
            for (int i = 0; i < log_buffer.size(); i++) {
                fprintf(f, "%s\n", log_buffer[i].utf8().get_data());
            }
            fprintf(f, "Mode: %s\n", is_editor ? "Editor" : "Game");
            fprintf(f, "Project: %s\n", project_name.is_empty() ? "N/A" : project_name.utf8().get_data());
            fclose(f);
        }
    }
#endif
}

void CrashHandler::set_is_editor(bool p_is_editor) {
    is_editor = p_is_editor;
}

void CrashHandler::set_project_name(const String &p_name) {
    project_name = p_name;
}
