# cpp98-sigslot

**Сигналы и слоты для C++98** — без Qt, без Boost, без RTTI, без исключений.

Лёгкая библиотека событийно-ориентированного программирования,
написанная на «чистом» C++98 и совместимая с **Borland C++ Builder 6.0**,
**Borland C++ 5.5.1**, а также современными GCC/Clang.

Включает готовые примеры приложений на базе **паттерна MVP** и модуль
**русификации консольного ввода-вывода** Windows.

---

## Ключевые слова

`сигналы и слоты` `signals and slots` `events` `observer pattern` `C++98`
`BCB6` `Borland C++` `MVP` `Model-View-Presenter` `кракозябры`
`русификация консоли` `IAT hooking` `console I/O` `кроссплатформенный`

---

## Содержание

- [Зачем ещё одна библиотека сигналов/слотов?](#зачем-ещё-одна-библиотека-сигналовслотов)
- [Быстрый старт](#быстрый-старт)
- [Архитектура библиотеки](#архитектура-библиотеки)
- [Примеры использования (MVP-приложения)](#примеры-использования-mvp-приложения)
- [Русификация консоли Windows](#русификация-консоли-windows)
- [Сборка](#сборка)
- [Структура репозитория](#структура-репозитория)

---

## Зачем ещё одна библиотека сигналов/слотов?

Существует множество реализаций сигналов/слотов: Qt Signals/Slots, Boost.Signals2,
nanosignalslot, KDBindings… Но каждая из них имеет хотя бы одну из проблем:

| Библиотека | C++98 | BCB6 | Без RTTI | Без исключений | Без зависимостей |
|---|:---:|:---:|:---:|:---:|:---:|
| Qt Signals/Slots | ✗ | ✗ | ✗ | ✗ | ✗ (moc) |
| Boost.Signals2 | ✗ | ✗ | ✗ | ✗ | ✗ (Boost) |
| nanosignalslot | ✗ | ✗ | ✓ | ✗ | ✓ |
| **cpp98-sigslot** | **✓** | **✓** | **✓** | **✓** | **✓** |

Если вы поддерживаете легаси-проект на Borland C++ Builder или вам нужна
максимально portable реализация для старых компиляторов — эта библиотека
написана специально для вас.

### Что умеет

- **Signal0 … Signal8** — сигналы с 0–8 аргументами произвольных типов
- Подключение **свободных функций**, **методов объектов** и **const-методов**
- **Отключение** слотов четырьмя способами: по объекту, по функции, по слоту, через билет
- **Реентерабельность** — безопасный вызов `emit_()` из другого `emit_()`
- **Самоудаление** слота во время emit (`delete this`) — не крашит
- **Отключение** и **очистка** подписчиков прямо во время emit — tombstones
- **FIFO-порядок** вызова слотов
- **Двусторонняя связь** — слот помнит все свои сигналы, сигнал — все слоты
- **Стековые билеты** — `SlotMethodImpl<T>` как член объекта, без `new`

---

## Быстрый старт

### Минимальный пример

```cpp
#include "signal_impl.h"
#include <stdio.h>

void onButtonClick() {
    printf("Кнопка нажата!\n");
}

int main() {
    signals::Signal0 buttonClicked;
    buttonClicked.connect(onButtonClick);  // подключаем слот
    buttonClicked.emit_();                 // → "Кнопка нажата!"
    return 0;
}
```

### Метод объекта

```cpp
class Calculator {
public:
    mutable int result;
    Calculator() : result(0) {}
    void add(int value) const { result += value; }
};

// Подключение:
signals::Signal1<int> valueChanged;
Calculator calc;
valueChanged.connect(&calc, &Calculator::add);
valueChanged.emit_(42);  // calc.result == 42
```

### Стековый билет (без new/delete)

```cpp
class MyHandler {
public:
    signals::SlotMethodImpl<MyHandler> m_slot;
    MyHandler() : m_slot(this, &MyHandler::onEvent) {}
    void onEvent() { /* ... */ }
};

// Подключение:
signals::Signal0 eventOccurred;
MyHandler handler;
eventOccurred += handler.m_slot;   // operator+= вместо connect()
eventOccurred.emit_();              // вызывает handler.onEvent()
// При разрушении handler билет автоматически отписывается от сигнала
```

### Сигнал с аргументами

```cpp
signals::Signal3<int, float, const char*> dataReady;
dataReady.connect(someFunction);            // свободная функция
dataReady.connect(&obj, &Obj::onData);      // метод объекта
dataReady.connect(&obj, &Obj::onDataConst); // const-метод
dataReady.emit_(42, 3.14f, "hello");        // все слоты вызваны
```

---

## Архитектура библиотеки

### Иерархия классов

```
SlotBaseCore (абстрактная база — реф-счёт, владение, флаг разрушения, список сигналов)
├── SlotBase (abstract, void())
│   ├── SlotFunctionPtr        — обёртка свободной функции
│   ├── SlotMethodImpl<T>      — обёртка метода объекта
│   └── SlotConstMethodImpl<T> — обёртка const-метода
└── SlotBaseN<T1,...,TN> (abstract, void(T1,...,TN))
    ├── SlotFunctionPtrN<T1,...,TN>
    ├── SlotMethodImplN<Receiver,T1,...,TN>
    └── SlotConstMethodImplN<Receiver,T1,...,TN>

SignalBase (связный список Node, счётчик глубины emit, сборщик мусора)
├── Signal0 : private SignalBase     (0 аргументов)
└── SignalN : private SignalBase     (1–8 аргументов, макро-генерация)
```

### Ключевые механизмы

| Механизм | Описание |
|---|---|
| **Tombstones** | Узлы, удалённые во время `emit_()`, помечаются как «мёртвые» и удаляются позже сборщиком |
| **EmitGuard** | RAII-обёртка: инкремент `m_emitDepth` при входе, декремент + сборка мусора при выходе |
| **Двусторонняя связь** | Слот хранит список своих сигналов (`m_signalLinks`), сигнал — список слотов (`Node`) |
| **Флаг разрушения** | `m_isDestructing` в `SlotBaseCore` защищает от Use-After-Free при `delete this` в слоте |

### Публичный API Signal0 / SignalN

```cpp
// Подключение
sig.connect(func);                  // свободная функция
sig.connect(slot);                  // слот-обёртка
sig.connect(obj, &Class::method);   // метод объекта
sig.connect(obj, &Class::method);   // const-метод (автоопределение)

sig += slot;   // операторная форма connect()
sig += func;   // операторная форма для свободной функции

// Отключение
sig.disconnect(obj);     // все слоты объекта
sig.disconnect(func);    // свободная функция
sig.disconnect(slot);    // конкретный слот
sig -= slot;             // операторная форма

// Испускание сигнала
sig.emit_();             // явный вызов
sig();                   // оператор () — синоним emit_()

// Прочее
sig.clear();             // отключить все слоты
```

---

## Примеры использования (MVP-приложения)

Все примеры построены по паттерну **MVP (Model-View-Presenter)** —
связь между View и Model осуществляется **исключительно через сигналы и слоты**.

### Принципы MVP в примерах

```
┌────────────────────────────────────────────────────────────────────┐
│                                                                    │
│   View (Вид)            Presenter (Презентер)     Model            │
│   «Глаза и уши»         «Диспетчер»               «Мозг»           │
│                                                                    │
│   • Показывает          • Связывает View          • Считает        │
│     информацию            и Model                   / Хранит       │
│   • Получает ввод       • Проверяет данные        • Формирует      │
│                                                                    │
│   • Излучает            • Перенаправляет          • Излучает       │
│     сигналы о             данные между              сигналы о      │
│     событиях              слоями                    ходе           │
│                                                     вычислений     │
│                                                                    │
│   НЕ знает Model        НЕ общается с             НЕ знает         │
│   НЕ знает                консолью напрямую         View           │
│     Presenter           НЕ считает                НЕ знает         │
│                                                     Presenter      │
│                                                                    │
│   Только                Все методы приватны       Никакого         │
│     ConsoleView         Знает об обоих              printf         │
│     работает с            интерфейсах                              │
│     консолью                                                       │
│                                                                    │
└────────────────────────────────────────────────────────────────────┘
```

**Строгие правила:**
- `IView` и `IModel` не знают друг о друге
- Presenter принимает только `IView*` и `IModel*`
- Все методы и члены Presenter — приватные
- `main()` создаёт экземпляры в куче через фабричные функции,
  хранит как указатели на базовые интерфейсы
- **НИКТО** кроме `ConsoleView` не общается с консолью

---

### MatrixMVP

```
examples/src/MatrixMVP/
```

Умножение матриц с **проверкой переполнения**. Ввод размерностей и элементов
двух матриц, перемножение, вывод результата. Модель обнаруживает переполнение
типа результата и сообщает об этом через отдельный сигнал `onOverflowDetected`.

Особенности:
- Настраиваемые типы: `type` (элемент матрицы, по умолчанию `short`) и
  `typeres` (результат умножения, по умолчанию `int`),
  можно переопределить при компиляции: `-DELEM_TYPE=char -DRESULT_TYPE=short`
- Ограничение `sizeof(typeres) == 2 * sizeof(type)` проверяется на этапе компиляции
  (`COMPILE_TIME_ASSERT`)
- Посимвольный ввод чисел с защитой от переполнения (`read_num`)
- Безопасный вывод результата с защитой от «числа-Феникс» (`write_typeres_aligned`)
- Русификация консоли

**Сигналы:** `onDimensionsEntered(rA, cA, rB, cB)`, `onMatrixAReady`, `onMatrixBReady`, `onResultReady`, `onOverflowDetected`, `onError`

---

### QuadEqMVP

```
examples/src/QuadEqMVP/
```

Решение квадратного уравнения **ax² + bx + c = 0** с верификацией
(подстановка корней обратно в уравнение). Шесть вариантов результата:
линейное, два корня, один корень, нет действительных корней,
бесконечно много решений, нет решений.

Особенности:
- Посимвольный ввод чисел с защитой от переполнения (`read_num`)
- Безопасный вывод целых чисел с защитой от «числа-Феникс» (`write_num`)
- Вывод `double` с 10 значащими цифрами и авто-точностью (`write_double`)
- Настраиваемый тип коэффициентов: `-DELEM_TYPE=short`
- Русификация консоли

**Сигналы:** `onCoefficientsReady`, `onResultReady`, `onError`

---

### GreetingMVP

```
examples/src/GreetingMVP/
```

Простейший пример: спрашивает имя и фамилию, выводит приветствие.
Демонстрирует два способа ввода строк (`scanf` vs `fgets`),
необязательный ввод (фамилию можно пропустить), русификацию консоли.

**Сигналы:** `onNameEntered`, `onSurnameEntered`, `onGreetingReady`, `onError`

---

### DateTimeMVP

```
examples/src/DateTimeMVP/
```

Конвертер **Unix Time ↔ TDateTime** с настраиваемым часовым поясом.
Функции `epoch2datetime` и `datetime2epoch` — расчёт года, месяца и дня
без массивов (экономия памяти), 16-битная арифметика для дней.

Особенности:
- Посимвольный ввод чисел: `read_int()` (знаковый) и `read_uint()` (беззнаковый)
- Защита от переполнения, Backspace, звуковой сигнал при ошибке
- Часовой пояс: смещение от UTC в часах (−12 … +14)
- Русские названия дней недели и месяцев
- Русификация консоли

**Сигналы:** `onTimezoneEntered`, `onModeSelected`, `onInputEntered`, `onResultReady`, `onError`

---

### FormNavigator

```
examples/src/FormNavigator/
```

**VCL GUI**-приложение: навигация между тремя формами с общей моделью.
Единственный пример с **графическим интерфейсом** — демонстрирует, что
сигналы/слоты работают не только в консоли, но и в полноценном Windows-приложении.

Три формы:
1. **HomeForm** (Главная) — показывает текущие данные
2. **DataForm** (Редактор) — позволяет редактировать данные
3. **ResultForm** (Результаты) — показывает результат обработки

Особенности:
- **Иерархия интерфейсов**: `IView` → `IHomeView`, `IDataView`, `IResultView`
- Каждая форма реализует только свой интерфейс — нет лишних сигналов
- Навигация между формами через сигналы (`onGoHome`, `onGoData`, `onGoResult`)
- Общие данные (`SharedData`: текст + счётчик) хранятся в модели
- Кнопка закрытия (X) на DataForm/ResultForm → возврат на HomeForm
- Компоненты на формах создаются в визуальном дизайнере BCB6 (файлы .dfm)
- Только BCB6 (требует VCL)

**Сигналы:** `onGoHome`, `onGoData`, `onGoResult`, `onExit`, `onDataSubmitted(SharedData)`, `onDataChanged`

---

## Русификация консоли Windows

Если вы когда-нибудь запускали консольное приложение на Windows и видели вместо
русских букв **кракозябры** — `╧ЁштхЄ ╠шЁ` вместо `Привет Мир` — модуль
`console_settings_keeper` решает эту проблему раз и навсегда.

### Проблема

Windows использует для консоли кодировку CP866 (OEM), а исходные файлы
обычно сохраняются в UTF-8 или Windows-1251 (ANSI). Несовпадение кодировок
приводит к нечитаемому выводу. Стандартные решения (`SetConsoleOutputCP(65001)`,
`system("chcp 65001")`) работают нестабильно на старых Windows.

### Решение

`console_settings_keeper` перехватывает функции Windows API для записи
в консоль (`WriteFile`, `WriteConsoleA`, `WriteConsoleW`) через технику
**IAT hooking** (Import Address Table) и автоматически перекодирует
UTF-8 / Windows-1251 → CP866 на лету. Аналогичный перехват для чтения
(`ReadFile`, `ReadConsoleA`, `ReadConsoleW`) обеспечивает корректный ввод.

### Использование

```cpp
#include "console_settings_keeper.h"

int main() {
    // Один вызов — и русские буквы работают:
    consoleKeeper.ApplyRussianSettings(SE_UTF8_NO_BOM);

    printf("Привет, Мир!\n");  // → читаемый текст, не кракозябры
    return 0;
}
```

### Поддерживаемые исходные кодировки

| Константа | Кодировка | Комментарий |
|---|---|---|
| `SE_UTF8_NO_BOM` | UTF-8 без BOM | Рекомендуемый вариант для современных исходников |
| `SE_UTF8_BOM` | UTF-8 с BOM | Если файл начинается с EF BB BF |
| `SE_ANSI_1251` | Windows-1251 | Для легаси-файлов в ANSI-кодировке |
| `SE_NONE` | Без перекодировки | Прямой вывод, как есть |

### Как это работает

1. При создании `ConsoleSettingsKeeper` определяет адреса функций
   `WriteFile`, `WriteConsoleA/W`, `ReadFile`, `ReadConsoleA/W` в памяти
2. `ApplyRussianSettings()` подменяет указатели в IAT (Import Address Table)
   на собственные функции-перехватчики
3. Перехватчики получают данные от приложения, перекодируют их
   UTF-8/1251 → CP866 и передают оригинальным API-функциям
4. При разрушении `ConsoleSettingsKeeper` (выход из `main`) оригинальные
   адреса восстанавливаются, и деструктор ждёт нажатия клавиши
   (чтобы окно консоли не закрылось раньше времени)

---

## Сборка

### Borland C++ Builder 6.0

Открыть `.bpr`-файл проекта, нажать **F9** (Run).

Или из командной строки:
```
make -f <Project>.bpf
```

### Borland C++ 5.5.1 (BC551)

Каждый проект содержит батник `_build_bc551.bat`.

### GCC / Clang (Windows, MinGW)

Каждый проект содержит батник `_build_gcc64.bat`.

### GCC / Clang (Linux / macOS)

```bash
# Библиотека + тест
g++ -std=c++98 -o signal_test \
    tests/signal_impl.cpp tests/main.cpp

# Пример MatrixMVP
g++ -std=c++98 -o MatrixMVP \
    examples/src/MatrixMVP/signal_impl.cpp \
    examples/src/MatrixMVP/MatrixTypes.cpp \
    examples/src/MatrixMVP/ConsoleView.cpp \
    examples/src/MatrixMVP/MatrixModel.cpp \
    examples/src/MatrixMVP/Presenter.cpp \
    examples/src/MatrixMVP/main.cpp

# Пример QuadEqMVP
g++ -std=c++98 -o QuadEqMVP \
    examples/src/QuadEqMVP/signal_impl.cpp \
    examples/src/QuadEqMVP/ConsoleView.cpp \
    examples/src/QuadEqMVP/QuadEqModel.cpp \
    examples/src/QuadEqMVP/Presenter.cpp \
    examples/src/QuadEqMVP/main.cpp -lm
```

> **Примечание:** модуль `console_settings_keeper` использует Win32 API
> (`<windows.h>`, IAT hooking) и работает только на Windows.
> На Linux/macOS он не нужен — там консоль изначально в UTF-8.
> Пример `FormNavigator` требует VCL и собирается только в BCB6 IDE.

---

## Структура репозитория

```
cpp98-sigslot/
│
├── signal_impl.h               Библиотека сигналов/слотов (header)
├── signal_impl.cpp             Реализация библиотеки
├── console_settings_keeper.h   Русификация консоли (header)
├── console_settings_keeper.cpp Реализация (Win32, IAT hooking)
│
├── tests/                      Тесты библиотеки
│   ├── main.cpp                100+ тестов сигналов/слотов
│   ├── signal_impl.h / .cpp
│   ├── console_settings_keeper.h / .cpp
│   ├── SignalTestBCB6.bpr / .bpf / .res
│   ├── SignalTest_build_bc551.bat
│   └── SignalTest_build_gcc64.bat
│
└── examples/
    │
    ├── bin/                    Скомпилированные exe (в .gitignore)
    │
    └── src/
        │
        ├── MatrixMVP/          Умножение матриц + проверка переполнения
        │   ├── signal_impl.h / .cpp
        │   ├── IView.h / IModel.h
        │   ├── ConsoleView.h / .cpp
        │   ├── MatrixModel.h / .cpp
        │   ├── MatrixTypes.h / .cpp
        │   ├── Presenter.h / .cpp
        │   ├── console_settings_keeper.h / .cpp
        │   ├── main.cpp
        │   ├── MatrixMVP.bpr / .bpf / .res
        │   └── ReadMe.txt
        │
        ├── QuadEqMVP/          Квадратное уравнение с верификацией
        │   ├── signal_impl.h / .cpp
        │   ├── IView.h / IModel.h
        │   ├── ConsoleView.h / .cpp
        │   ├── QuadEqModel.h / .cpp
        │   ├── QuadEqTypes.h
        │   ├── Presenter.h / .cpp
        │   ├── console_settings_keeper.h / .cpp
        │   ├── main.cpp
        │   ├── QuadEqMVP.bpr / .bpf / .res
        │   └── ReadMe.txt
        │
        ├── GreetingMVP/        Приветствие пользователя
        │   ├── signal_impl.h / .cpp
        │   ├── IView.h / IModel.h
        │   ├── ConsoleView.h / .cpp
        │   ├── GreetingModel.h / .cpp
        │   ├── GreetingTypes.h / .cpp
        │   ├── Presenter.h / .cpp
        │   ├── console_settings_keeper.h / .cpp
        │   ├── main.cpp
        │   ├── GreetingMVP.bpr / .bpf / .res
        │   └── ReadMe.txt
        │
        ├── DateTimeMVP/        Конвертер Unix Time ↔ TDateTime
        │   ├── signal_impl.h / .cpp
        │   ├── IView.h / IModel.h
        │   ├── ConsoleView.h / .cpp
        │   ├── DateTimeModel.h / .cpp
        │   ├── DateTimeTypes.h
        │   ├── Presenter.h / .cpp
        │   ├── console_settings_keeper.h / .cpp
        │   ├── main.cpp
        │   ├── DateTimeMVP.bpr / .bpf / .res
        │   └── ReadMe.txt
        │
        └── FormNavigator/      Навигация между VCL-формами (GUI)
            ├── signal_impl.h / .cpp
            ├── IView.h / IModel.h
            ├── HomeForm.h / .cpp / .dfm
            ├── DataForm.h / .cpp / .dfm
            ├── ResultForm.h / .cpp / .dfm
            ├── AppModel.h / .cpp
            ├── NavigatorTypes.h / .cpp
            ├── Presenter.h / .cpp
            ├── FormNavigator.cpp     (WinMain)
            ├── FormNavigator.bpr / .res
            ├── app.ico / default.ico
            └── ReadMe.txt
```

---

## Совместимость

| Компилятор | Статус |
|---|---|
| Borland C++ Builder 6.0 (bcc32 5.x) | ✅ Основная целевая платформа |
| Borland C++ 5.5.1 (bc551) | ✅ Проверено |
| GCC 10+ (g++, `-std=c++98`) | ✅ Проверено |
| Clang 10+ | ✅ Проверено |
| MSVC | ⚠ Не тестировалось, но должно работать |
| C++11 и выше | ✅ Обратная совместимость |

---

## Лицензия

MIT
