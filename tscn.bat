@echo off
chcp 65001 >nul
title Real Engine - Fix .tres to .tscn
color 0A

setlocal enabledelayedexpansion

echo ============================================
echo   REAL ENGINE - ЗАМЕНА .tres НА .tscn
echo ============================================
echo.

:: Переходим в папку с исходниками Godot
cd /d D:\Real_Engine\source_realengine

echo 1. Создаю бэкап всех изменяемых файлов...
set BACKUP_DIR=backup_tres_%date:~-4,4%%date:~-7,2%%date:~-10,2%_%time:~0,2%%time:~3,2%
mkdir "%BACKUP_DIR%" 2>nul
echo   ✓ Бэкап создан: %BACKUP_DIR%
echo.

echo 2. Заменяю .tres на .tscn во всех .cpp и .h файлах...
echo.

:: Счётчики
set TOTAL_FILES=0

:: Обрабатываем все .cpp и .h файлы
for /r . %%f in (*.cpp *.h) do (
    findstr /m "\.tres" "%%f" >nul 2>nul
    if !errorlevel! equ 0 (
        set /a TOTAL_FILES+=1
        echo   Обрабатываю: %%f
        
        :: Создаём копию в бэкапе
        set "rel_path=%%f"
        set "rel_path=!rel_path:%CD%\=!"
        set "backup_path=%BACKUP_DIR%\!rel_path!"
        for %%p in ("!backup_path!") do mkdir "%%~dpp" 2>nul
        copy "%%f" "!backup_path!" >nul
        
        :: Заменяем .tres на .tscn
        powershell -Command "(Get-Content '%%f') -replace '\.tres', '.tscn' | Set-Content '%%f' -Encoding UTF8"
        
        echo     ✓ Обработан
    )
)

echo.
echo 3. Проверяю остальные файлы...
echo.

:: Обрабатываем .py и другие файлы
for /r . %%f in (*.py *.xml *.cfg) do (
    findstr /m "\.tres" "%%f" >nul 2>nul
    if !errorlevel! equ 0 (
        set /a TOTAL_FILES+=1
        echo   Обрабатываю: %%f
        
        :: Создаём копию в бэкапе
        set "rel_path=%%f"
        set "rel_path=!rel_path:%CD%\=!"
        set "backup_path=%BACKUP_DIR%\!rel_path!"
        for %%p in ("!backup_path!") do mkdir "%%~dpp" 2>nul
        copy "%%f" "!backup_path!" >nul
        
        :: Заменяем .tres на .tscn
        powershell -Command "(Get-Content '%%f') -replace '\.tres', '.tscn' | Set-Content '%%f' -Encoding UTF8"
    )
)

echo.
echo ============================================
echo   РЕЗУЛЬТАТЫ
echo ============================================
echo.
echo   Изменено файлов: %TOTAL_FILES%
echo   Бэкап: %BACKUP_DIR%
echo.

:: Проверка остатков
echo 4. Проверяю остаточные вхождения...
echo.
findstr /s /i "\.tres" *.cpp *.h *.py *.xml *.cfg 2>nul
if !errorlevel! equ 0 (
    echo   ⚠ Найдены остаточные вхождения!
    echo   Список выше.
) else (
    echo   ✓ Все вхождения .tres заменены на .tscn!
)

echo.
echo 5. Особо важные файлы для проверки:
echo    - editor/settings/editor_settings.cpp
echo    - core/io/resource_loader.cpp
echo    - scene/resources/resource_format_text.cpp
echo.

echo Нажмите любую клавишу для выхода...
pause >nul