@echo off
title Real Engine Installer
echo ========================================
echo    Установка Real Engine
echo ========================================
echo.

:: Проверяем права администратора
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Ошибка: Запустите установщик от имени администратора!
    pause
    exit /b 1
)

:: Копируем файлы
echo Копирование файлов...
if not exist "%ProgramFiles%\Real Engine" mkdir "%ProgramFiles%\Real Engine"
copy /Y "godot.windows.editor.x86_64.exe" "%ProgramFiles%\Real Engine\" >nul
copy /Y "rlengine_handler.exe" "%ProgramFiles%\Real Engine\" >nul

:: Регистрируем протокол
echo Регистрация протокола rlengine://...
reg add HKCR\rlengine /ve /d "URL:Real Engine Protocol" /f >nul
reg add HKCR\rlengine /v "URL Protocol" /d "" /f >nul
reg add HKCR\rlengine\shell\open\command /ve /d "\"%ProgramFiles%\Real Engine\rlengine_handler.exe\" \"%%1\"" /f >nul

:: Создаем ярлык на рабочем столе
echo Создание ярлыков...
powershell -Command "$WS = New-Object -ComObject WScript.Shell; $SC = $WS.CreateShortcut('%UserProfile%\Desktop\Real Engine.lnk'); $SC.TargetPath = '%ProgramFiles%\Real Engine\godot.windows.editor.x86_64.exe'; $SC.Save()" >nul

echo.
echo ========================================
echo    Установка завершена!
echo ========================================
echo.
echo Real Engine установлен в: %ProgramFiles%\Real Engine
echo Протокол rlengine:// зарегистрирован
echo.
pause