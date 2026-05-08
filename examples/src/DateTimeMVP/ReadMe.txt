============================================================
  DateTimeMVP — Конвертер времени, паттерн MVP + Сигналы/Слоты
============================================================

Консольное приложение, выполняющее преобразование между
Unix Time (epoch) и форматом даты/времени TDateTime.
Реализовано по паттерну MVP (Model-View-Presenter).
Связь между View и Model осуществляется исключительно через
сигнально-слотовый механизм библиотеки signal_impl.

Особенности:
  • Преобразование epoch2datetime: Unix Time -> Дата/Время
  • Преобразование datetime2epoch: Дата/Время -> Unix Time
  • Настраиваемый часовой пояс (смещение от UTC в часах)
  • Корректный вывод русских букв через console_settings_keeper
  • Валидация вводимых данных (презентер)
  • Посимвольный ввод чисел с защитой от переполнения


------------------------------------------------------------
Архитектура MVP
------------------------------------------------------------

  MVP (Model-View-Presenter) — паттерн, разделяющий приложение
  на три независимых слоя:

  +--------------------------------------------------------------+
  |                                                              |
  |   View (Вид)          Presenter (Презентер)   Model          |
  |   «Глаза и уши»       «Диспетчер»           «Мозг»           |
  |                                                              |
  |   • Показывает         • Связывает View       • Конвертирует |
  |     информацию            и Model               Unix Time    |
  |   • Получает ввод      • Проверяет данные     • Формирует    |
  |   • Излучает сигналы   • Перенаправляет         результат    |
  |     о событиях           данные между           • Излучает   |
  |                          слоями                  сигналы     |
  |                                                   о ходе     |
  |   НЕ знает Model       НЕ общается с            вычислений   |
  |   НЕ знает Presenter     консолью напрямую      НЕ знает     |
  |                         НЕ считает                View       |
  |                                                  НЕ знает    |
  |   Только ConsoleView   Все методы приватны      Presenter    |
  |   работает с           Знает об обоих           Никакого     |
  |   консолью             интерфейсах               printf      |
  |                                                              |
  +--------------------------------------------------------------+

  Ключевые принципы:
  • IView и IModel не знают друг о друге
  • Presenter принимает только IView* и IModel*
  • Все методы и члены Presenter — приватные
  • main() создаёт экземпляры в куче, хранит как указатели
    на базовые интерфейсы
  • НИКТО кроме ConsoleView не общается с консолью


------------------------------------------------------------
Способ ввода чисел
------------------------------------------------------------

  Все числовые значения вводятся посимвольно через функции
  read_int() и read_uint() (определены в ConsoleView.cpp).

  Эти функции обеспечивают:
  • Защиту от переполнения (overflow detection)
  • Поддержку минуса для знаковых чисел (read_int)
  • Обработку Backspace (стирание последней цифры)
  • Звуковой сигнал (beep) при некорректном вводе
  • Ввод завершается нажатием Enter

  Используются функции getch() и putch() из <conio.h>
  для посимвольного чтения и отображения без буферизации.


------------------------------------------------------------
Состав проекта
------------------------------------------------------------

  signal_impl.h          Библиотека сигналов/слотов (C++98, namespace signals)
  signal_impl.cpp        Реализация библиотеки

  DateTimeTypes.h        Общие типы (TDateTime, ConversionMode)

  IView.h                Интерфейс вида — чистые виртуальные методы + сигналы
  IModel.h               Интерфейс модели — чистые виртуальные методы + сигналы

  ConsoleView.h          Объявление ConsoleView + фабрика createConsoleView()
  ConsoleView.cpp        Реализация ConsoleView, функции read_int/read_uint
                         ЕДИНСТВЕННЫЙ модуль, работающий с консолью!
                         (printf, getch, putch)

  DateTimeModel.h        Объявление DateTimeModel + фабрика createDateTimeModel()
  DateTimeModel.cpp      Реализация DateTimeModel (конвертация, форматирование).
                         Никакого printf!

  Presenter.h            Объявление Presenter (все члены приватные)
  Presenter.cpp          Реализация Presenter (соединения, обработчики)
                         Никакого printf!

  main.cpp               Точка входа. Никакого printf!

  console_settings_keeper.h  Модуль корректного вывода русских букв на консоль
  console_settings_keeper.cpp Реализация модуля (IAT hooking, UTF-8/866/1251)

  DateTimeMVP.bpr        Файл проекта Borland C++ Builder 6.0
  DateTimeMVP.bpf        Makefile BCB6
  DateTimeMVP.res        Ресурсы проекта

  ReadMe.txt             Этот файл


------------------------------------------------------------
Граф зависимостей заголовков
------------------------------------------------------------

                     signal_impl.h
                     /           \
                    v             v
                IView.h         IModel.h
                |   \           /   |
                |    \         /    |
                |     v       v     |
                |   Presenter.h     |
                |    /              |
                v   v               v
            ConsoleView.h      DateTimeModel.h

  DateTimeTypes.h  (независим, включается туда, где нужны TDateTime/константы)


Подробные зависимости каждого .cpp-файла:

  signal_impl.cpp           <- signal_impl.h
  ConsoleView.cpp           <- ConsoleView.h, DateTimeTypes.h, <stdio>,
                               <conio.h>, console_settings_keeper.h
  DateTimeModel.cpp         <- DateTimeModel.h, <stdio>, <string>
  Presenter.cpp             <- Presenter.h, DateTimeTypes.h
  main.cpp                  <- IView.h, IModel.h, Presenter.h
  console_settings_keeper.cpp <- console_settings_keeper.h, <windows.h> (Win32 only)

  Обратите внимание: ни DateTimeModel.cpp, ни Presenter.cpp,
  ни main.cpp не включают <stdio>!


------------------------------------------------------------
Граф зависимостей компиляции (порядок сборки)
------------------------------------------------------------

  1. signal_impl.cpp           -> signal_impl.obj
  2. ConsoleView.cpp           -> ConsoleView.obj
  3. DateTimeModel.cpp         -> DateTimeModel.obj
  4. Presenter.cpp             -> Presenter.obj
  5. console_settings_keeper.cpp -> console_settings_keeper.obj
  6. main.cpp                  -> main.obj

  Линковка: c0x32.obj + все .obj -> DateTimeMVP.exe


------------------------------------------------------------
Поток взаимодействия (runtime)
------------------------------------------------------------

  main()
   |
   +-- createConsoleView()    -> IView*  (new ConsoleView)
   +-- createDateTimeModel()  -> IModel* (new DateTimeModel)
   |
   +-- Presenter(view, model)
        |
        +-- connect: view->onTimezoneEntered  -> slot->handleTimezoneEntered
        +-- connect: view->onModeSelected     -> slot->handleModeSelected
        +-- connect: view->onInputEntered     -> slot->handleInputEntered
        +-- connect: model->onResultReady     -> slot->handleResultReady
        +-- connect: model->onError           -> slot->handleModelError
        |
        +-- view->showBanner()          <-- баннер
        +-- view->promptTimezone()      <-- старт диалога
             |
             v пользователь вводит часовой пояс (read_int)
           onTimezoneEntered -----> handleTimezoneEntered()
             |                        +- валидация (-12..+14)
             |                        +- model->setTimezoneOffset()
             |                        +- view->promptMode()
             v пользователь выбирает направление (read_int)
           onModeSelected --------> handleModeSelected()
             |                        +- валидация (1 или 2)
             |                        +- model->setConversionMode()
             |                        +- promptEpochInput() или
             |                           promptDateTimeInput()
             v пользователь вводит данные (read_int/read_uint)
           onInputEntered ----------> handleInputEntered()
             |                        +- model->setEpoch/DateTime()
             |                        +- model->convert()
             v конвертация
           onResultReady -----------> handleResultReady()
                                      +- view->showResult()

           onError -----------------> handleModelError()
                                      +- view->showError()


------------------------------------------------------------
Сборка
------------------------------------------------------------

  Borland C++ Builder 6.0:
    Открыть DateTimeMVP.bpr, нажать F9 (Run).

  Командная строка BCB6:
    make -f DateTimeMVP.bpf

  BC551 (соседняя папка):
    DateTimeMVP_build_bc551.bat

  GCC/Clang (соседняя папка gcc64):
    DateTimeMVP_build_gcc64.bat

  GCC/Clang (Linux/macOS):
    g++ -std=c++98 -o DateTimeMVP \
        signal_impl.cpp ConsoleView.cpp DateTimeModel.cpp \
        Presenter.cpp main.cpp
