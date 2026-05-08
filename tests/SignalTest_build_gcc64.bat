@echo off
rem Скрипт сборки SignalTest_gcc64.exe с помощью GCC
rem Компилятор GCC ожидается в папке, соседней с папкой проекта

setlocal enabledelayedexpansion

rem Определяем папку скрипта (убираем завершающий \)
set SCRIPT_DIR=%~dp0
set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%
set PROG_NAME=SignalTest

rem Поднимаемся на уровень выше
for %%I in ("%SCRIPT_DIR%\..") do set PARENT_DIR=%%~fI

rem Путь к компилятору GCC (например, MinGW)
set GCC_DIR=%PARENT_DIR%\gcc64

rem Проверяем наличие c++.exe
if not exist "%GCC_DIR%\bin\c++64.exe" (
    echo Error: Compiler not found at "%GCC_DIR%\bin\c++64.exe"
    echo.
    pause
    exit /b 1
)

rem Настраиваем окружение для GCC
set PATH=%GCC_DIR%\bin;%PATH%

echo Building %PROG_NAME%_gcc64.exe ...
echo Using compiler from %GCC_DIR%

rem Компилируем и линкуем: -o задаёт имя выходного файла
c++64 -o %PROG_NAME%_gcc64.exe ^
    main.cpp ^
    signal_impl.cpp ^
    console_settings_keeper.cpp

rem Запоминаем результат сборки
set BUILD_RESULT=%errorlevel%

rem Удаляем объектные файлы (всегда, даже при ошибке)
for %%F in (main signal_impl console_settings_keeper) do (
    if exist %%F.o del %%F.o
)

if %BUILD_RESULT% neq 0 (
    echo.
    echo Build failed! Artifacts cleaned up.
    pause
    exit /b 1
)

echo.
echo Build succeeded. Artifacts cleaned up.
echo Only %PROG_NAME%_gcc64.exe remains in the folder.

endlocal
pause
