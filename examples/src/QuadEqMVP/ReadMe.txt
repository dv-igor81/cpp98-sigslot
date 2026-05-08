============================================================
  QuadEqMVP — Решение квадратного уравнения, MVP + Сигналы/Слоты
============================================================

Консольное приложение, решающее квадратное уравнение ax^2 + bx + c = 0
по паттерну MVP (Model-View-Presenter). Связь между View и Model
осуществляется через сигнально-слотовый механизм библиотеки signal_impl.


------------------------------------------------------------
Архитектура MVP
------------------------------------------------------------

  MVP (Model-View-Presenter) — паттерн, разделяющий приложение
  на три независимых слоя:

  +----------------------------------------------------------+
  |                                                          |
  |   View (Вид)          Presenter (Презентер)    Model     |
  |   «Глаза и уши»       «Диспетчер»             «Мозг»     |
  |                                                          |
  |   • Показывает         • Связывает View       • Решает   |
  |     информацию            и Model               уравнение|
  |   • Получает ввод      • Перенаправляет       • Хранит   |
  |   • Излучает сигналы     данные между          результат |
  |     о событиях            слоями              •  Излучает|
  |                                                 сигналы  |
  |   НЕ знает Model       НЕ общается с            о ходе   |
  |   НЕ знает Presenter     консолью напрямую     вычислений|
  |                        НЕ считает               НЕ знает |
  |   Только ConsoleView   Все методы приватны       View    |
  |   работает с           Знает об обоих           НЕ знает |
  |   консолью             интерфейсах              Presenter|
  |                                                          |
  +----------------------------------------------------------+


------------------------------------------------------------
Состав проекта
------------------------------------------------------------

  signal_impl.h     Библиотека сигналов/слотов (C++98, namespace signals)
  signal_impl.cpp   Реализация библиотеки

  QuadEqTypes.h     Общие типы (type = int), константа PRECISION.
                    Header-only: не требует отдельного .cpp

  IView.h           Интерфейс вида — чистые виртуальные методы + сигналы.
                    Фабричная функция createConsoleView().

  IModel.h          Интерфейс модели — чистые виртуальные методы + сигналы.
                    Enum SolveResult (6 вариантов результата).
                    Фабричная функция createQuadEqModel().

  ConsoleView.h     Объявление ConsoleView
  ConsoleView.cpp   Реализация ConsoleView.
                    ЕДИНСТВЕННЫЙ модуль, работающий с консолью!
                    Содержит: read_num, write_num, write_double,
                    get_precision, getch_key, putch_key.

  QuadEqModel.h     Объявление QuadEqModel
  QuadEqModel.cpp   Реализация QuadEqModel (решение уравнения,
                    вычисление дискриминанта и корней, верификация).
                    Никакого printf!

  Presenter.h       Объявление Presenter (все члены приватные)
  Presenter.cpp     Реализация Presenter (соединения, обработчики).
                    Никакого printf!

  main.cpp          Точка входа. Никакого printf!

  QuadEqMVP.bpr     Файл проекта Borland C++ Builder 6.0
  QuadEqMVP.bpf     Makefile BCB6

  ReadMe.txt        Этот файл


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
          ConsoleView.h      QuadEqModel.h

  QuadEqTypes.h  (независим, включается туда, где нужны type/PRECISION)


Подробные зависимости каждого .cpp-файла:

  signal_impl.cpp  <- signal_impl.h
  ConsoleView.cpp  <- ConsoleView.h, QuadEqTypes.h, <stdio>, <stdlib>,
                     <math>, <conio> (Win32) / <termios> (POSIX)
  QuadEqModel.cpp  <- QuadEqModel.h, <math>
  Presenter.cpp    <- Presenter.h
  main.cpp         <- IView.h, IModel.h, Presenter.h


------------------------------------------------------------
Граф зависимостей компиляции (порядок сборки)
------------------------------------------------------------

  1. signal_impl.cpp  -> signal_impl.obj
  2. ConsoleView.cpp  -> ConsoleView.obj
  3. QuadEqModel.cpp  -> QuadEqModel.obj
  4. Presenter.cpp    -> Presenter.obj
  5. main.cpp         -> main.obj

  Линковка: c0x32.obj + все .obj -> QuadEqMVP.exe


------------------------------------------------------------
Поток взаимодействия (runtime)
------------------------------------------------------------

  main()
   |
   +- createConsoleView()   -> IView*     (new ConsoleView)
   +- createQuadEqModel()   -> IModel*    (new QuadEqModel)
   |
   +- Presenter(view, model)
        |
        +- connect: view->onCoefficientsReady -> slot->handleCoefficientsReady
        +- connect: model->onResultReady      -> slot->handleResultReady
        +- connect: model->onError            -> slot->handleError
        |
        +- view->showBanner()              <--- приветствие
        +- view->promptCoefficients()      <--- старт диалога
             |
             v пользователь вводит a, b, c
           onCoefficientsReady ---> handleCoefficientsReady()
             |                       +- view->showCoefficients(a,b,c)
             |                       +- model->setCoefficients(a,b,c)
             |                       +- model->solve()
             v модель решила уравнение
           onResultReady ---> handleResultReady()
             |                  +- SOLVE_LINEAR:       view->showLinearResult()
             |                  +- SOLVE_TWO_ROOTS:    view->showTwoRoots()
             |                  +- SOLVE_ONE_ROOT:     view->showOneRoot()
             |                  +- SOLVE_NO_REAL_ROOTS: view->showNoRealRoots()
             |                  +- SOLVE_INFINITE:     view->showInfiniteSolutions()
             |                  +- SOLVE_NO_SOLUTION:  view->showNoSolutions()
             |
           onError -----------> handleError()
                                   +- view->showError()


------------------------------------------------------------
Типы данных
------------------------------------------------------------

  По умолчанию:
    type = int   (32 бита, коэффициенты уравнения)

  Можно переопределить при компиляции:
    -DELEM_TYPE=short

  Результаты вычислений (корни, дискриминант, верификация) — double.


------------------------------------------------------------
Сборка
------------------------------------------------------------

  Borland C++ Builder 6.0:
    Открыть QuadEqMVP.bpr, нажать F9 (Run).

  Командная строка BCB6:
    make -f QuadEqMVP.bpf

  GCC/Clang (Linux/macOS):
    g++ -std=c++98 -o QuadEqMVP \
        signal_impl.cpp ConsoleView.cpp \
        QuadEqModel.cpp Presenter.cpp main.cpp -lm


------------------------------------------------------------
Особенности реализации
------------------------------------------------------------

  • Посимвольный ввод чисел с защитой от переполнения (read_num).
  • Безопасный вывод целых чисел с защитой от числа-Феникс (write_num).
  • Вывод double с 10 значащими цифрами (write_double + get_precision).
  • Обнаружение переноса разряда при округлении (9.999 -> 10.0).
  • Верификация: подстановка корней обратно в уравнение.
  • Реентерабельность сигналов (EmitGuard, m_emitDepth).
  • Строгое разделение: только ConsoleView работает с консолью.
============================================================
