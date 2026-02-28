/**************************************************************************/
/*  crash_handler_windows_dialog.cpp                                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "crash_handler_windows.h"

#include "core/version.h"
#include "core/string/print_string.h"
#include "core/string/ustring.h"
#include "core/object/object.h"

#include <shellapi.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Инициализация статических членов
CrashHandler::CrashDialogData* CrashHandler::dialog_data = nullptr;
CrashHandler* CrashHandler::instance = nullptr;

// Оконная процедура для диалога краша
INT_PTR CALLBACK CrashHandler::dialog_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG: {
            // Центрируем окно
            HWND parent = GetDesktopWindow();
            RECT rc, rcDlg;
            GetWindowRect(parent, &rc);
            GetWindowRect(hwnd, &rcDlg);
            SetWindowPos(hwnd, NULL,
                (rc.right - (rcDlg.right - rcDlg.left)) / 2,
                (rc.bottom - (rcDlg.bottom - rcDlg.top)) / 2,
                0, 0, SWP_NOSIZE | SWP_NOZORDER);

            return TRUE;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDYES: // Сообщить в поддержку
                    open_support_url();
                    SetTimer(hwnd, 1, 2000, nullptr);
                    EnableWindow(GetDlgItem(hwnd, IDYES), FALSE);
                    SetDlgItemTextW(hwnd, IDYES, L"Thanks! Completion...");
                    break;

                case IDNO: // Закрыть
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    _exit(1);
                    break;
            }
            return TRUE;
        }

        case WM_TIMER: {
            if (wParam == 1) {
                KillTimer(hwnd, 1);
                EndDialog(hwnd, 0);
                _exit(1);
            }
            return TRUE;
        }

        case WM_CLOSE:
            EndDialog(hwnd, 0);
            _exit(1);
            return TRUE;
    }
    return FALSE;
}

// Поток для отображения диалога
DWORD WINAPI CrashHandler::dialog_thread_proc(LPVOID lpParameter) {
    // Инициализируем Common Controls
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    // Создаем простой диалог
    HWND hwnd = CreateDialog(
        GetModuleHandle(nullptr),
        MAKEINTRESOURCE(32770), // Стандартный диалог
        nullptr,
        dialog_proc
    );

    if (hwnd && dialog_data) {
        dialog_data->dialog_window = hwnd;

        // Устанавливаем заголовок
        SetWindowTextW(hwnd, L"Real Engine - Critical Error!");

        // Устанавливаем текст сообщения
        String message = "A critical error has occurred.\n\n";
        message += dialog_data->error_message + "\n\n";
        message += "Would you like to inform the developers about this?";

        // Конвертируем String в Wide String
        CharString cs = message.utf8();
        int len = MultiByteToWideChar(CP_UTF8, 0, cs.get_data(), -1, NULL, 0);
        wchar_t* wstr = (wchar_t*)memalloc(len * sizeof(wchar_t));
        MultiByteToWideChar(CP_UTF8, 0, cs.get_data(), -1, wstr, len);

        SetDlgItemTextW(hwnd, 65535, wstr);
        memfree(wstr);

        // Переименовываем кнопки
        SetDlgItemTextW(hwnd, IDYES, L"Report to support");
        SetDlgItemTextW(hwnd, IDNO, L"Close");

        ShowWindow(hwnd, SW_SHOW);

        // Запускаем цикл сообщений
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            if (!IsDialogMessage(hwnd, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    return 0;
}

// Показываем диалог краша
void CrashHandler::show_crash_dialog(const String &p_error_message) {
    if (disabled || IsDebuggerPresent()) {
        return;
    }

    // Создаем структуру данных если нужно
    if (!dialog_data) {
        dialog_data = memnew(CrashDialogData);
    }

    // Сохраняем данные
    dialog_data->error_message = p_error_message;
    dialog_data->stack_trace = "";
    dialog_data->dialog_thread = nullptr;
    dialog_data->dialog_window = nullptr;

    // Создаем поток для диалога
    dialog_data->dialog_thread = CreateThread(
        nullptr,
        0,
        dialog_thread_proc,
        nullptr,
        0,
        nullptr
    );

    // Ждем завершения диалога
    if (dialog_data->dialog_thread) {
        WaitForSingleObject(dialog_data->dialog_thread, INFINITE);
        CloseHandle(dialog_data->dialog_thread);
        dialog_data->dialog_thread = nullptr;
    }
}

// Открываем URL поддержки
void CrashHandler::open_support_url() {
    if (!dialog_data) return;

    String support_url = "https://www.k1shm1sh-realengine.ru/faq.html";

    // Формируем URL с параметрами
    String full_url = support_url +
        "?error=" + dialog_data->error_message.uri_encode() +
        "&version=" + String(VERSION_FULL_CONFIG).uri_encode();

    // Конвертируем в wide string
    CharString cs = full_url.utf8();
    int len = MultiByteToWideChar(CP_UTF8, 0, cs.get_data(), -1, NULL, 0);
    wchar_t* wstr = (wchar_t*)memalloc(len * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, cs.get_data(), -1, wstr, len);

    // Открываем в браузере
    ShellExecuteW(nullptr, L"open", wstr, nullptr, nullptr, SW_SHOW);

    memfree(wstr);
}
