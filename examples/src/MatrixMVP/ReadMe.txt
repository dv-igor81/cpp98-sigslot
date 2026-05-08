============================================================
  MatrixMVP — Умножение матриц, паттерн MVP + Сигналы/Слоты
============================================================

Демонстрационное консольное приложение, реализующее умножение
матриц с проверкой переполнения по паттерну MVP (Model-View-Presenter).
Связь между View и Model осуществляется исключительно через
сигнально-слотовый механизм библиотеки signal_impl.


------------------------------------------------------------
Архитектура MVP
------------------------------------------------------------

  MVP (Model-View-Presenter) — паттерн, разделяющий приложение
  на три независимых слоя:

  -----------------------------------------------------------¬
  ¦                                                          ¦
  ¦   View (Вид)          Presenter (Презентер)   Model      ¦
  ¦   «Глаза и уши»       «Диспетчер»           «Мозг»       ¦
  ¦                                                          ¦
  ¦   • Показывает         • Связывает View       • Считает  ¦
  ¦     информацию            и Model               • Хранит ¦
  ¦   • Получает ввод      • Проверяет данные       данные   ¦
  ¦   • Излучает сигналы   • Перенаправляет       • Излучает ¦
  ¦     о событиях           данные между           сигналы  ¦
  ¦                          слоями                  о ходе  ¦
  ¦   НЕ знает Model       НЕ общается с          вычислений ¦
  ¦   НЕ знает Presenter     консолью напрямую     НЕ знает  ¦
  ¦                         НЕ считает               View    ¦
  ¦                                                  НЕ знает¦
  ¦   Только ConsoleView   Все методы приватны      Presenter¦
  ¦   работает с           Знает об обоих           Никакого ¦
  ¦   консолью             интерфейсах               printf  ¦
  ¦                                                          ¦
  L-----------------------------------------------------------

  Ключевые принципы:
  • IView и IModel не знают друг о друге
  • Presenter принимает только IView* и IModel*
  • Все методы и члены Presenter — приватные
  • main() создаёт экземпляры в куче, хранит как указатели
    на базовые интерфейсы
  • НИКТО кроме ConsoleView не общается с консолью


------------------------------------------------------------
Состав проекта
------------------------------------------------------------

  signal_impl.h     Библиотека сигналов/слотов (C++98, namespace signals)
  signal_impl.cpp   Реализация библиотеки

  MatrixTypes.h     Общие типы (type, typeres), константы,
                    COMPILE_TIME_ASSERT, объявления computeTypeLimits()
                    и глобальных пределов g_*
  MatrixTypes.cpp   Реализация computeTypeLimits() и глобальных пределов

  IView.h           Интерфейс вида — чистые виртуальные методы + сигналы
  IModel.h          Интерфейс модели — чистые виртуальные методы + сигналы

  ConsoleView.h     Объявление ConsoleView + фабрика createConsoleView()
  ConsoleView.cpp   Реализация ConsoleView, I/O-хелперы
                    ЕДИНСТВЕННЫЙ модуль, работающий с консолью!
                    (getch_key, read_num, write_typeres,
                    write_typeres_aligned, printf)

  MatrixModel.h     Объявление MatrixModel + фабрика createMatrixModel()
  MatrixModel.cpp   Реализация MatrixModel (умножение, проверка
                    переполнения). Никакого printf!

  Presenter.h       Объявление Presenter (все члены приватные)
  Presenter.cpp     Реализация Presenter (соединения, обработчики)
                    Никакого printf!

  main.cpp          Точка входа. Никакого printf!

  MatrixMVP.bpr     Файл проекта Borland C++ Builder 6.0
  MatrixMVP.bpf     Makefile BCB6

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
          ConsoleView.h      MatrixModel.h

  MatrixTypes.h  (независим, включается туда, где нужны type/typeres/константы)


Подробные зависимости каждого .cpp-файла:

  signal_impl.cpp  < signal_impl.h
  MatrixTypes.cpp  < MatrixTypes.h
  ConsoleView.cpp  < ConsoleView.h, MatrixTypes.h, <stdio>, <stdlib>,
                     <conio> (Win32) / <termios> (POSIX)
  MatrixModel.cpp  < MatrixModel.h, MatrixTypes.h, <stdlib>, <string>
  Presenter.cpp    < Presenter.h, MatrixTypes.h
  main.cpp         < IView.h, IModel.h, Presenter.h

  Обратите внимание: ни MatrixModel.cpp, ни Presenter.cpp,
  ни main.cpp не включают <stdio>!


------------------------------------------------------------
Граф зависимостей компиляции (порядок сборки)
------------------------------------------------------------

  1. signal_impl.cpp  > signal_impl.obj
  2. MatrixTypes.cpp  > MatrixTypes.obj
  3. ConsoleView.cpp  > ConsoleView.obj
  4. MatrixModel.cpp  > MatrixModel.obj
  5. Presenter.cpp    > Presenter.obj
  6. main.cpp         > main.obj

  Линковка: c0x32.obj + все .obj > MatrixMVP.exe


------------------------------------------------------------
Поток взаимодействия (runtime)
------------------------------------------------------------

  main()
   ¦
   +- createConsoleView()   > IView*  (new ConsoleView)
   +- createMatrixModel()   > IModel* (new MatrixModel)
   ¦
   L- Presenter(view, model)
        ¦
        +- connect: view>onDimensionsEntered  > slot>handleDimensionsEntered
        +- connect: view>onMatrixAReady       > slot>handleMatrixAReady
        +- connect: view>onMatrixBReady       > slot>handleMatrixBReady
        +- connect: model>onResultReady       > slot>handleResultReady
        +- connect: model>onOverflowDetected  > slot>handleOverflowDetected
        +- connect: model>onError             > slot>handleModelError
        ¦
        +- view>showBanner()           <-- приветствие
        L- view>promptDimensions()     <-- старт диалога
             ¦
             Ў пользователь вводит размерности
           onDimensionsEntered ----> handleDimensionsEntered()
             ¦                        +- валидация
             ¦                        +- view>showInvalidDimensions() [ошибка]
             ¦                        +- view>showDimensionMismatch() [ошибка]
             ¦                        +- model>setDimensions()
             ¦                        L- view>promptMatrixA()
             Ў пользователь вводит матрицу A
           onMatrixAReady ------> handleMatrixAReady()
             ¦                        +- model>loadMatrixA()
             ¦                        L- view>promptMatrixB()
             Ў пользователь вводит матрицу B
           onMatrixBReady ------> handleMatrixBReady()
             ¦                        +- model>loadMatrixB()
             ¦                        L- model>multiply()
             Ў вычисление завершено
           onResultReady ------> handleResultReady()
             ¦                        L- view>showResult()
             ¦
           onOverflowDetected -> handleOverflowDetected()
                                      L- view>showOverflowWarning()
             ¦
           onError -------------> handleModelError()
                                      L- view>showError()


------------------------------------------------------------
Типы данных
------------------------------------------------------------

  По умолчанию:
    type     = short   (16 бит, элемент матрицы)
    typeres  = int     (32 бита, результат умножения)

  Можно переопределить при компиляции:
    -DELEM_TYPE=char -DRESULT_TYPE=short

  Ограничения (проверяются на этапе компиляции):
    sizeof(typeres) == 2 * sizeof(type)
    Оба типа имеют одинаковую знаковость (оба signed или оба unsigned)


------------------------------------------------------------
Сборка
------------------------------------------------------------

  Borland C++ Builder 6.0:
    Открыть MatrixMVP.bpr, нажать F9 (Run).

  Командная строка BCB6:
    make -f MatrixMVP.bpf

  GCC/Clang (Linux/macOS):
    g++ -std=c++98 -o MatrixMVP \
        signal_impl.cpp MatrixTypes.cpp ConsoleView.cpp \
        MatrixModel.cpp Presenter.cpp main.cpp


------------------------------------------------------------
Особенности реализации
------------------------------------------------------------

  • Посимвольный ввод чисел с защитой от переполнения (read_num).
  • Обнаружение переполнения при умножении (флаг m_overflow).
  • Защита от «числа-Феникса» (INT_MIN) при выводе результата
    (write_typeres, write_typeres_aligned).
  • Реентерабельность сигналов (EmitGuard, m_emitDepth).
  • Отложенное удаление слотов (isDead, sweepDeadNodes).
  • Слоты, подключённые во время emit_, не вызываются в текущем
    цикле (snapshot_tail).
  • Строгое разделение: только ConsoleView работает с консолью.
  • Модель сообщает об ошибках через сигнал onError (не через printf).
============================================================
