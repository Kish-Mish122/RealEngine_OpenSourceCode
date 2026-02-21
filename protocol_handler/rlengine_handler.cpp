#include <windows.h>
#include <shellapi.h>
#include <string>
#include <fstream>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Получаем командную строку
    std::string cmdline = GetCommandLineA();

    // Ищем протокол
    size_t pos = cmdline.find("rlengine://");
    if (pos == std::string::npos) return 1;

    std::string url = cmdline.substr(pos);

    // Ищем окно Real Engine
    HWND hWnd = FindWindowA(NULL, "Real Engine");

    if (hWnd != NULL) {
        // Уже запущен - разворачиваем
        ShowWindow(hWnd, SW_RESTORE);
        SetForegroundWindow(hWnd);

        // Отправляем URL
        COPYDATASTRUCT cds;
        cds.dwData = 1;
        cds.cbData = url.length() + 1;
        cds.lpData = (void*)url.c_str();
        SendMessageA(hWnd, WM_COPYDATA, 0, (LPARAM)&cds);
    } else {
        // Запускаем новый
        ShellExecuteA(NULL, "open",
            "godot.windows.editor.x86_64.exe",
            url.c_str(), NULL, SW_SHOW);
    }

    return 0;
}
