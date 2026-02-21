@echo off
title Real Engine Uninstaller
echo ========================================
echo    Удаление Real Engine
echo ========================================
echo.

:: Проверяем права администратора
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Ошибка: Запустите от имени администратора!
    pause
    exit /b 1
)

:: Удаляем протокол
echo Удаление протокола...
reg delete HKCR\rlengine /f >nul 2>&1

:: Удаляем файлы
echo Удаление файлов...
rmdir /s /q "%ProgramFiles%\Real Engine" >nul 2>&1

:: Удаляем ярлык
del "%UserProfile%\Desktop\Real Engine.lnk" >nul 2>&1

echo.
echo ========================================
echo    Удаление завершено!
echo ========================================
echo.
pause