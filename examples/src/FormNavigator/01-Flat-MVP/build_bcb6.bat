@echo off
setlocal

REM === Build script for FormNavigator (BCB6 command-line) ===
REM
REM Требования:
REM   - Borland C++ Builder 6.0 (предоставляет bcc32, ilink32, brcc32, VCL)
REM   - Свободный BC551 НЕ подходит — нет VCL!
REM
REM Иконка приложения:
REM   - Если есть app.ico — используется она (пользовательская)
REM   - Иначе — default.ico из каталога проекта (входит в поставку)
REM   - default.ico должна быть в формате BMP, НЕ PNG
REM     (BRCC32 не поддерживает PNG-сжатые иконки!)
REM
REM Использование:
REM   build_bcb6.bat
REM   build_bcb6.bat "C:\Program Files\Borland\CBuilder6"
REM
REM =======================================================================

if "%1"=="" (
    set "BCB=C:\Program Files\Borland\CBuilder6"
) else (
    set "BCB=%~1"
)

REM Конвертируем путь в короткий формат 8.3 — убираем пробелы
for %%I in ("%BCB%") do set "BCB=%%~sI"

set "PATH=%BCB%\Bin;%PATH%"
set "INC=-I%BCB%\Include -I%BCB%\Include\Vcl"
set "LIB=-L%BCB%\Lib\Obj -L%BCB%\Lib"

echo === Building FormNavigator (BCB6) ===
echo BCB=%BCB%

REM -----------------------------------------------------------------------
REM Иконка приложения: своя или стандартная из проекта
REM -----------------------------------------------------------------------

set "APP_ICON="
if exist "app.ico" (
    set "APP_ICON=app.ico"
    echo Icon: app.ico - custom
) else if exist "default.ico" (
    set "APP_ICON=default.ico"
    echo Icon: default.ico - project default
) else (
    echo Icon: none found - building without icon
)

REM -----------------------------------------------------------------------
REM Step 1: Компиляция ресурсов (.dfm + иконка -> Forms.res)
REM   Ресурсы форм называются по имени класса (THomeForm и т.д.),
REM   т.к. VCL ищет ресурс по имени класса при загрузке формы.
REM   MAINICON — стандартное имя иконки приложения для VCL.
REM -----------------------------------------------------------------------

echo.
echo [1/3] Compiling resources...

echo THomeForm RCDATA "HomeForm.dfm"      > Forms.rc
echo TDataForm RCDATA "DataForm.dfm"     >> Forms.rc
echo TResultForm RCDATA "ResultForm.dfm" >> Forms.rc

if not "%APP_ICON%"=="" (
    echo MAINICON ICON "%APP_ICON%"       >> Forms.rc
)

brcc32 Forms.rc
if errorlevel 1 goto fail

REM -----------------------------------------------------------------------
REM Step 2: Компиляция .cpp -> .obj
REM   -tW       — Windows GUI (а не консоль)
REM   -c        — только компиляция, без линковки
REM   -Vx -Ve   — совместимость VCL
REM   -w-8027   — подавление предупреждений в макросах сигналов
REM   -w-8026   — подавление предупреждений о неиспользуемых параметрах
REM -----------------------------------------------------------------------

echo.
echo [2/3] Compiling .cpp sources...

bcc32 -tW -c -Vx -Ve -w-8027 -w-8026 %INC% signal_impl.cpp
if errorlevel 1 goto fail
bcc32 -tW -c -Vx -Ve -w-8027 -w-8026 %INC% NavigatorTypes.cpp
if errorlevel 1 goto fail
bcc32 -tW -c -Vx -Ve -w-8027 -w-8026 %INC% HomeForm.cpp
if errorlevel 1 goto fail
bcc32 -tW -c -Vx -Ve -w-8027 -w-8026 %INC% DataForm.cpp
if errorlevel 1 goto fail
bcc32 -tW -c -Vx -Ve -w-8027 -w-8026 %INC% ResultForm.cpp
if errorlevel 1 goto fail
bcc32 -tW -c -Vx -Ve -w-8027 -w-8026 %INC% AppModel.cpp
if errorlevel 1 goto fail
bcc32 -tW -c -Vx -Ve -w-8027 -w-8026 %INC% Presenter.cpp
if errorlevel 1 goto fail
bcc32 -tW -c -Vx -Ve -w-8027 -w-8026 %INC% FormNavigator.cpp
if errorlevel 1 goto fail

REM -----------------------------------------------------------------------
REM Step 3: Линковка -> FormNavigator.exe
REM   c0w32.obj   — startup для Windows GUI (не c0x32.obj!)
REM   -aa         — Windows GUI application
REM   -Tpe        — PE executable
REM   cp32mt.lib  — C++ runtime (static, не требует DLL на другом ПК!)
REM   vcl.lib     — Visual Component Library
REM   rtl.lib     — Runtime Library
REM   import32.lib — Win32 API imports
REM
REM   Формат ilink32:
REM     ilink32 [options] objfiles, exe, mapfile, libfiles, deffile, resfiles
REM -----------------------------------------------------------------------

echo.
echo [3/3] Linking FormNavigator.exe...

ilink32 %LIB% -D"" -aa -Tpe -x -Gn c0w32.obj FormNavigator.obj signal_impl.obj NavigatorTypes.obj HomeForm.obj DataForm.obj ResultForm.obj AppModel.obj Presenter.obj, FormNavigator.exe,, import32.lib cp32mt.lib vcl.lib rtl.lib,, Forms.res
if errorlevel 1 goto fail

echo.
echo === Build successful: FormNavigator.exe ===
goto cleanup

:fail
echo.
echo === Build FAILED ===

:cleanup
del *.obj *.tds *.rc 2>nul
del Forms.res 2>nul
REM Не удаляем FormNavigator.res — может содержать манифест и другие ресурсы
endlocal
