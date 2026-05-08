// ============================================================================
// ConsoleView.cpp — Консольная реализация IView
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// ЕДИНСТВЕННЫЙ модуль, работающий с консолью.
// Ни презентер, ни модель, ни main() не содержат ни одного printf/scanf.
// Все пользовательские сообщения и весь ввод — только здесь.
//
// ПРИНЦИП РАБОТЫ:
//
//   1. Презентер вызывает promptTimezone() -> спрашиваем смещение UTC,
//      сохраняем ответ и излучаем onTimezoneEntered.
//
//   2. Презентер вызывает promptMode() -> спрашиваем направление
//      преобразования (1 или 2), сохраняем и излучаем onModeSelected.
//
//   3. Презентер вызывает promptEpochInput() или promptDateTimeInput()
//      -> запрашиваем значение, сохраняем и излучаем onInputEntered.
//
//   4. Презентер вызывает showResult() -> выводим результат.
//
// СПОСОБ ВВОДА:
//
//   Все числовые значения вводятся посимвольно через функции
//   read_int() и read_uint(), которые обеспечивают:
//     • Защиту от переполнения (overflow detection)
//     • Поддержку минуса для знаковых чисел
//     • Обработку Backspace (стирание последней цифры)
//     • Звуковой сигнал при некорректном вводе
//     • Ввод завершается нажатием Enter
//
//   Эти функции используют getch() для посимвольного чтения
//   (без ожидания Enter и без эха) и putch() для отображения
//   введённых символов.
//
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// ============================================================================
// ПОДКЛЮЧЕНИЕ БИБЛИОТЕК
// ============================================================================
#include <stdio.h>     /* printf, fflush */
#include <string.h>    /* memset */
#include <conio.h>     /* getch, putch */

#include "ConsoleView.h"
#include "DateTimeTypes.h"

#include "console_settings_keeper.h"

// ============================================================================
// ФУНКЦИИ БЕЗОПАСНОГО ВВОДА ЧИСЕЛ
// ============================================================================
//
// Посимвольный ввод с защитой от переполнения.
// Позволяют вводить только цифры (и минус для знаковых).
// Backspace стирает последнюю цифру.
// Enter подтверждает ввод.
// Любой другой символ — звуковой сигнал (beep).
//
// ============================================================================

// ----------------------------------------------------------------------------
// Безопасный ввод знакового числа типа int
// ----------------------------------------------------------------------------
static int read_int(void)
{
    int accumulator = 0;
    int limit;
    int key;
    int pressed_digit;
    int is_blocked;
    int sign = 1;
    int input_started = 0;

    while (1) {
        is_blocked = 0;
        key = getch();

        // Специальные клавиши (F1-F12, стрелки и т.д.) — пропускаем
        if (key == 0 || key == 0xE0) {
            getch(); // дочитываем scan code
            putch('\a');
            continue;
        }

        if (key >= '0' && key <= '9') {
            pressed_digit = key - '0';
            if (input_started && accumulator == 0) {
                if (sign == 1) is_blocked = 1;
                else if (sign == -1 && pressed_digit == 0) is_blocked = 1;
            }
            if (!is_blocked) {
                limit = accumulator * 10 + pressed_digit * sign;
                if (accumulator != (limit / 10)) {
                    is_blocked = 1;
                } else {
                    accumulator = limit;
                    input_started = 1;
                    putch(key);
                }
            }
        } else if (key == '-' && !input_started) {
            sign = -1;
            input_started = 1;
            putch(key);
        } else if (key == '\b' && input_started) {
            putch('\b'); putch(' '); putch('\b');
            if (accumulator == 0) {
                sign = 1;
                input_started = 0;
            } else {
                accumulator /= 10;
                if (accumulator == 0 && sign == 1)
                    input_started = 0;
            }
        } else if (key == '\r') {
            if (accumulator == 0 && (!input_started || sign == -1))
                is_blocked = 1;
            else
                break;
        } else {
            is_blocked = 1;
        }

        if (is_blocked) putch('\a');
    }
    return accumulator;
}

// ----------------------------------------------------------------------------
// Безопасный ввод беззнакового числа типа unsigned int
// ----------------------------------------------------------------------------
static unsigned int read_uint(void)
{
    unsigned int accumulator = 0;
    unsigned int limit;
    int key;
    int pressed_digit;
    int is_blocked;
    int input_started = 0;

    while (1) {
        is_blocked = 0;
        key = getch();

        // Специальные клавиши (F1-F12, стрелки и т.д.) — пропускаем
        if (key == 0 || key == 0xE0) {
            getch(); // дочитываем scan code
            putch('\a');
            continue;
        }

        if (key >= '0' && key <= '9') {
            pressed_digit = key - '0';
            if (input_started && accumulator == 0) {
                is_blocked = 1;
            }
            if (!is_blocked) {
                limit = accumulator * 10U + (unsigned int)pressed_digit;
                if (accumulator != (limit / 10U)) {
                    is_blocked = 1;
                } else {
                    accumulator = limit;
                    input_started = 1;
                    putch(key);
                }
            }
        } else if (key == '\b' && input_started) {
            putch('\b'); putch(' '); putch('\b');
            accumulator /= 10U;
            if (accumulator == 0)
                input_started = 0;
        } else if (key == '\r') {
            if (accumulator == 0 && !input_started)
                is_blocked = 1;
            else
                break;
        } else {
            is_blocked = 1;
        }

        if (is_blocked) putch('\a');
    }
    return accumulator;
}

// ============================================================================
// ConsoleView — реализация
// ============================================================================

ConsoleView::ConsoleView()
    : m_timezoneOffset(0), m_selectedMode(0), m_epochInput(0)
{
    memset(&m_dateTimeInput, 0, sizeof(m_dateTimeInput));

    // Применяем настройки консоли для корректного отображения русских букв
    consoleKeeper.ApplyRussianSettings(SE_ANSI_1251);
}

ConsoleView::~ConsoleView() {}

// --- Баннер: показывается при запуске приложения ---

void ConsoleView::showBanner()
{
    printf("============================================================\n");
    printf("  КОНВЕРТЕР ВРЕМЕНИ — Паттерн MVP + Сигналы/Слоты\n");
    printf("  Преобразование Unix Time <-> Дата/Время (TDateTime)\n");
    printf("============================================================\n\n");
}

// --- Ввод часового пояса ---

void ConsoleView::promptTimezone()
{
    printf("Введите часовой пояс (смещение в часах от UTC,\n");
    printf("  например 3 для Москвы, -5 для Нью-Йорка): ");
    fflush(stdout);

    m_timezoneOffset = read_int();
    printf("\n");

    onTimezoneEntered.emit_();
}

// --- Ввод направления преобразования ---

void ConsoleView::promptMode()
{
    printf("\nВыберите направление преобразования:\n");
    printf("  1. Unix Time -> Дата/Время  (epoch2datetime)\n");
    printf("  2. Дата/Время -> Unix Time  (datetime2epoch)\n");
    printf("Ваш выбор (1 или 2): ");
    fflush(stdout);

    m_selectedMode = read_int();
    printf("\n");

    onModeSelected.emit_();
}

// --- Ввод значения Unix Time ---

void ConsoleView::promptEpochInput()
{
    printf("\nВведите значение Unix Time (секунды с 01.01.1970 00:00:00 UTC): ");
    fflush(stdout);

    m_epochInput = read_uint();
    printf("\n");

    onInputEntered.emit_();
}

// --- Ввод даты и времени (по полям) ---

void ConsoleView::promptDateTimeInput()
{
    int val;

    printf("\nВведите дату и время:\n");

    printf("  День месяца (1-31): ");
    fflush(stdout);
    val = read_int();
    printf("\n");
    m_dateTimeInput.TimeDMonth = (unsigned char)val;

    printf("  Месяц (1-12): ");
    fflush(stdout);
    val = read_int();
    printf("\n");
    m_dateTimeInput.TimeMonth = (unsigned char)val;

    printf("  Год (0-99, где 0=2000, 99=2099): ");
    fflush(stdout);
    val = read_int();
    printf("\n");
    m_dateTimeInput.TimeYear = (unsigned char)val;

    printf("  Часы (0-23): ");
    fflush(stdout);
    val = read_int();
    printf("\n");
    m_dateTimeInput.TimeHor = (unsigned char)val;

    printf("  Минуты (0-59): ");
    fflush(stdout);
    val = read_int();
    printf("\n");
    m_dateTimeInput.TimeMin = (unsigned char)val;

    printf("  Секунды (0-59): ");
    fflush(stdout);
    val = read_int();
    printf("\n");
    m_dateTimeInput.TimeSec = (unsigned char)val;

    // День недели не запрашиваем — он вычисляется моделью
    m_dateTimeInput.TimeWeek = 0;

    onInputEntered.emit_();
}

// --- Вывод результата ---

void ConsoleView::showResult(const char* text)
{
    // Определяем знак часового пояса для отображения
    int tz = m_timezoneOffset;
    const char* tzSign = (tz >= 0) ? "+" : "";
    int tzAbs = (tz >= 0) ? tz : -tz;

    printf("\n============================================================\n");
    printf("  Результат конвертации:\n");
    printf("  %s\n", text);
    printf("  Часовой пояс: UTC%s%d", tzSign, tz);
    if (tzAbs == 3) {
        printf(" (Москва)");
    } else if (tzAbs == 0) {
        printf(" (Лондон)");
    }
    printf("\n");
    printf("============================================================\n\n");
}

// --- Вывод ошибки ---

void ConsoleView::showError(const char* message)
{
    printf("\nОшибка: %s\n", message);
}

// --- Геттеры: презентер забирает введённые данные ---

int ConsoleView::getTimezoneOffset() const { return m_timezoneOffset; }
int ConsoleView::getSelectedMode() const { return m_selectedMode; }
unsigned int ConsoleView::getEpochInput() const { return m_epochInput; }
const TDateTime& ConsoleView::getDateTimeInput() const { return m_dateTimeInput; }


// ============================================================================
// Фабричная функция
// ============================================================================
// Создаёт ConsoleView в куче и возвращает указатель на базовый интерфейс.
// Вызывающий код (main) не знает о классе ConsoleView — только об IView.

IView* createConsoleView() { return new ConsoleView(); }
