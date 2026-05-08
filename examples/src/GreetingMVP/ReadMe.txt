============================================================
  GreetingMVP — Приветствие пользователя, паттерн MVP + Сигналы/Слоты
============================================================

Демонстрационное консольное приложение, реализующее приветствие
пользователя по паттерну MVP (Model-View-Presenter).
Связь между View и Model осуществляется исключительно через
сигнально-слотовый механизм библиотеки signal_impl.

Особенности:
  • Два способа ввода: scanf (имя) и fgets (фамилия)
  • Фамилия необязательна — можно нажать Enter для пропуска
  • Корректный вывод русских букв через console_settings_keeper


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
  |   • Показывает         • Связывает View       • Формирует    |
  |     информацию            и Model               приветствие  |
  |   • Получает ввод      • Проверяет данные     • Хранит       |
  |   • Излучает сигналы   • Перенаправляет         данные       |
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
Два способа ввода
------------------------------------------------------------

  1. Имя — через scanf(" %99[^\n]", name):
     • Пробел перед %99[^\n] «съедает» \n от предыдущего ввода
     • Читает до перевода строки, но не включая его
     • Возвращает 1 при успехе
     • Пример: if (scanf(" %99[^\n]", name) == 1) { ... }

  2. Фамилия — через fgets(name, sizeof(name), stdin):
     • Захватывает символ \n (нажатие Enter)
     • Нужно вручную обрезать \n через strcspn
     • Если сразу нажать Enter — строка = "\n" -> фамилия = ""
     • Пример:
         if (fgets(name, sizeof(name), stdin) != NULL) {
             int pos = strcspn(name, "\n");
             name[pos] = '\0';
         }

  Фамилия необязательна! Если пользователь нажал Enter
  не введя ничего — фамилия считается пустой, и приветствие
  формируется только по имени.


------------------------------------------------------------
Состав проекта
------------------------------------------------------------

  signal_impl.h          Библиотека сигналов/слотов (C++98, namespace signals)
  signal_impl.cpp        Реализация библиотеки

  GreetingTypes.h        Общие типы (UserData: имя, фамилия)
  GreetingTypes.cpp      Реализация UserData

  IView.h                Интерфейс вида — чистые виртуальные методы + сигналы
  IModel.h               Интерфейс модели — чистые виртуальные методы + сигналы

  ConsoleView.h          Объявление ConsoleView + фабрика createConsoleView()
  ConsoleView.cpp        Реализация ConsoleView, I/O-хелперы
                         ЕДИНСТВЕННЫЙ модуль, работающий с консолью!
                         (scanf, fgets, printf)

  GreetingModel.h        Объявление GreetingModel + фабрика createGreetingModel()
  GreetingModel.cpp      Реализация GreetingModel (формирование приветствия).
                         Никакого printf!

  Presenter.h            Объявление Presenter (все члены приватные)
  Presenter.cpp          Реализация Presenter (соединения, обработчики)
                         Никакого printf!

  main.cpp               Точка входа. Никакого printf!

  console_settings_keeper.h  Модуль корректного вывода русских букв на консоль
  console_settings_keeper.cpp Реализация модуля (IAT hooking, UTF-8/866/1251)

  GreetingMVP.bpr        Файл проекта Borland C++ Builder 6.0
  GreetingMVP.bpf        Makefile BCB6
  GreetingMVP.res        Ресурсы проекта

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
            ConsoleView.h      GreetingModel.h

  GreetingTypes.h  (независим, включается туда, где нужны UserData/константы)


Подробные зависимости каждого .cpp-файла:

  signal_impl.cpp           <- signal_impl.h
  GreetingTypes.cpp         <- GreetingTypes.h
  ConsoleView.cpp           <- ConsoleView.h, GreetingTypes.h, <stdio>,
                               console_settings_keeper.h
  GreetingModel.cpp         <- GreetingModel.h, <stdio>, <string>
  Presenter.cpp             <- Presenter.h, GreetingTypes.h
  main.cpp                  <- IView.h, IModel.h, Presenter.h
  console_settings_keeper.cpp <- console_settings_keeper.h, <windows.h> (Win32 only)

  Обратите внимание: ни GreetingModel.cpp, ни Presenter.cpp,
  ни main.cpp не включают <stdio>!


------------------------------------------------------------
Граф зависимостей компиляции (порядок сборки)
------------------------------------------------------------

  1. signal_impl.cpp           -> signal_impl.obj
  2. GreetingTypes.cpp         -> GreetingTypes.obj
  3. ConsoleView.cpp           -> ConsoleView.obj
  4. GreetingModel.cpp         -> GreetingModel.obj
  5. Presenter.cpp             -> Presenter.obj
  6. console_settings_keeper.cpp -> console_settings_keeper.obj
  7. main.cpp                  -> main.obj

  Линковка: c0x32.obj + все .obj -> GreetingMVP.exe


------------------------------------------------------------
Поток взаимодействия (runtime)
------------------------------------------------------------

  main()
   |
   +-- createConsoleView()   -> IView*  (new ConsoleView)
   +-- createGreetingModel() -> IModel* (new GreetingModel)
   |
   +-- Presenter(view, model)
        |
        +-- connect: view->onNameEntered     -> slot->handleNameEntered
        +-- connect: view->onSurnameEntered  -> slot->handleSurnameEntered
        +-- connect: model->onGreetingReady  -> slot->handleGreetingReady
        +-- connect: model->onError          -> slot->handleModelError
        |
        +-- view->showBanner()          <-- приветствие
        +-- view->promptName()          <-- старт диалога
             |
             v пользователь вводит имя (scanf)
           onNameEntered -----> handleNameEntered()
             |                    +-- валидация (не пустое?)
             |                    +-- model->setName()
             |                    +-- view->promptSurname()
             v пользователь вводит/пропускает фамилию (fgets)
           onSurnameEntered --> handleSurnameEntered()
             |                    +-- model->setSurname()
             |                    +-- model->buildGreeting()
             v формирование приветствия
           onGreetingReady ----> handleGreetingReady()
                                  +-- view->showGreeting()

           onError -------------> handleModelError()
                                  +-- view->showError()


------------------------------------------------------------
Сборка
------------------------------------------------------------

  Borland C++ Builder 6.0:
    Открыть GreetingMVP.bpr, нажать F9 (Run).

  Командная строка BCB6:
    make -f GreetingMVP.bpf

  BC551 (соседняя папка):
    GreetingMVP_build_bc551.bat

  GCC/Clang (соседняя папка gcc64):
    GreetingMVP_build_gcc64.bat

  GCC/Clang (Linux/macOS):
    g++ -std=c++98 -o GreetingMVP \
        signal_impl.cpp GreetingTypes.cpp ConsoleView.cpp \
        GreetingModel.cpp Presenter.cpp main.cpp
