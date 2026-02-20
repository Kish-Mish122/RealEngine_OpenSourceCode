@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

:: ПЕРЕХОДИМ В ПРАВИЛЬНУЮ ПАПКУ
cd /d "D:\Real_Engine\source_realengine"

:: ОТЛАДКА
echo ===== ДЕБАГ =====
echo Текущая папка: %CD%
echo Дата и время: %date% %time%
echo =================
echo.

pause
echo.

echo ================================================
echo    ЗАМЕНА project.godot НА project.rlengine
echo         (ИСПРАВЛЕННАЯ ВЕРСИЯ)
echo ================================================
echo.

:: Создаем бэкап
set BACKUP_DIR=backup_%date:~-4,4%%date:~-7,2%%date:~-10,2%_%time:~0,2%%time:~3,2%
mkdir "%BACKUP_DIR%" 2>nul
echo Создан бэкап: %BACKUP_DIR%
echo.

set TOTAL_FILES=0
set TOTAL_CHANGES=0

:: Поиск всех файлов с project.godot
echo Сканирование файлов...
echo.

:: ИСПРАВЛЕНО: используем временный файл вместо прямого вызова PowerShell
for /r . %%f in (*.cpp *.h *.py *.cs) do (
    findstr /m "project\.godot" "%%f" >nul 2>nul
    if !errorlevel! equ 0 (
        echo Найден: %%f
        set "file=%%f"
        
        :: Создаем структуру папок в бэкапе
        set "rel_path=!file:%CD%\=!"
        set "backup_path=%BACKUP_DIR%\!rel_path!"
        for %%p in ("!backup_path!") do mkdir "%%~dpp" 2>nul
        
        :: Копируем оригинал в бэкап
        copy "%%f" "!backup_path!" /Y >nul
        
        :: ИСПРАВЛЕНО: простая замена через PowerShell без сложных скобок
        powershell -Command "(Get-Content '%%f') -replace 'project\.godot', 'project.rlengine' | Set-Content '%%f'"
        
        set /a TOTAL_FILES+=1
    )
)

echo.
echo ================================================
echo    РЕЗУЛЬТАТЫ
echo ================================================
echo.
echo Изменено файлов: %TOTAL_FILES%
echo Бэкап создан: %BACKUP_DIR%
echo.

:: Проверка
echo Проверка остаточных вхождений:
echo.
findstr /s /n "project\.godot" *.cpp *.h *.py *.cs 2>nul > "%TEMP%\remaining.txt"
if exist "%TEMP%\remaining.txt" (
    echo ⚠ Найдены оставшиеся project.godot:
    type "%TEMP%\remaining.txt"
) else (
    echo ✓ Все project.godot заменены на project.rlengine!
)

echo.
pause