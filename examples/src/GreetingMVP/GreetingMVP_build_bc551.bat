@echo off
rem Скрипт сборки GreetingMVP_bc551.exe
rem Компилятор BC551 ожидается в папке, соседней с папкой проекта

setlocal enabledelayedexpansion

rem Определяем папку скрипта (убираем завершающий \)
set SCRIPT_DIR=%~dp0
set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%
set PROG_NAME=GreetingMVP

rem Поднимаемся на уровень выше
for %%I in ("%SCRIPT_DIR%\..") do set PARENT_DIR=%%~fI

rem Путь к компилятору
set BORLAND=%PARENT_DIR%\BC551

rem Проверяем наличие компилятора
if not exist "%BORLAND%\Bin\bcc32.exe" (
    echo Error: Compiler not found at "%BORLAND%\Bin\bcc32.exe"
    echo.
    pause
    exit /b 1
)

rem Настраиваем окружение
set PATH=%BORLAND%\Bin;%PATH%
set INCLUDE=%BORLAND%\Include
set LIB=%BORLAND%\Lib

echo Building %PROG_NAME%_bc551.exe ...
echo Using compiler from %BORLAND%

bcc32 -e%PROG_NAME%_bc551.exe -I%INCLUDE% -L%LIB% ^
    main.cpp ^
    ConsoleView.cpp ^
    GreetingModel.cpp ^
    GreetingTypes.cpp ^
    Presenter.cpp ^
    signal_impl.cpp ^
    console_settings_keeper.cpp

rem Запоминаем результат сборки
set BUILD_RESULT=%errorlevel%

rem Удаляем объектные файлы (всегда, даже при ошибке)
for %%F in (main ConsoleView GreetingModel GreetingTypes Presenter signal_impl console_settings_keeper) do (
    if exist %%F.obj del %%F.obj
)

rem Удаляем файл отладочной информации (всегда)
if exist %PROG_NAME%_bc551.tds del %PROG_NAME%_bc551.tds

if %BUILD_RESULT% neq 0 (
    echo.
    echo Build failed! Artifacts cleaned up.
    pause
    exit /b 1
)

echo.
echo Build succeeded. Artifacts cleaned up.
echo Only %PROG_NAME%_bc551.exe remains in the folder.

endlocal
pause
