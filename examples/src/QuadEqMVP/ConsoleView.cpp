// ============================================================================
// ConsoleView.cpp — Консольная реализация IView
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// ЕДИНСТВЕННЫЙ модуль, работающий с консолью.
// Ни презентер, ни модель, ни main() не содержат ни одного printf/getch.
//
// Содержит:
//   • getch_key / putch_key — кроссплатформенный посимвольный ввод/вывод
//   • read_num — безопасный ввод целого числа (с защитой от переполнения)
//   • write_num — безопасный вывод целого числа (защита от числа-Феникс)
//   • write_double — вывод double с 10 значащими цифрами
//   • get_precision — вычисление точности для форматирования double
//
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "ConsoleView.h"
#include "QuadEqTypes.h"

#include "console_settings_keeper.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

using namespace signals;

// ============================================================================
// Кроссплатформенные функции ввода/вывода
// ============================================================================

static void putch_key(int ch)
{
#ifdef _WIN32
    putch(ch);
#else
    putchar(ch);
    fflush(stdout);
#endif
}

static int getch_key(void)
{
#ifdef _WIN32
    return getch();
#else
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    if (ch == 10)  ch = 13;
    if (ch == 127) ch = 8;
    return ch;
#endif
}

// ============================================================================
// Безопасный ввод числа типа type
// ============================================================================
// Посимвольный ввод с защитой от переполнения, блокировкой букв,
// обработкой Backspace и Enter.

static type read_num(void)
{
    type accumulator = 0;
    type limit;
    int key;
    int pressed_digit;
    int is_blocked;
    int is_signed = ((type)-1 < 0);
    int sign = 1;
    int input_started = 0;

    while (1) {
        is_blocked = 0;
        key = getch_key();

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
                    putch_key(key);
                }
            }
        } else if (key == '-' && !input_started) {
            if (is_signed) {
                sign = -1;
                input_started = 1;
                putch_key(key);
            } else {
                is_blocked = 1;
            }
        } else if (key == '\b' && input_started) {
            putch_key('\b'); putch_key(' '); putch_key('\b');
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

        if (is_blocked) putch_key('\a');
    }
    return accumulator;
}

// ============================================================================
// Безопасный вывод целого числа (защита от числа-Феникс)
// ============================================================================
// Обычный printf может вывести INT_MIN с ошибками на старых компиляторах.
// Выводим посимвольно, обрабатывая особый случай INT_MIN (число-Феникс).

static void write_num(type x)
{
    int i;
    type temp_value;
    type weight = 1;
    int is_overflow = 0;
    int is_neg = 0;
    int bits_type = sizeof(type) * 8 - 1;

    type max = 1;
    for (i = 1; i < bits_type; i++) {
        max <<= 1;
        max += 1;
    }

    if ((x < max) && (x & ~max) != 0)
        is_neg = 1;

    if (is_neg) {
        putch_key('-');
        x = (type)(~x + 1);
        is_neg = (x & ~max) != 0;
        if (is_neg) {
            is_overflow = 1;
            x++;
            x = (type)(~x + 1);
        }
    }

    temp_value = x;
    while (temp_value >= 10) { weight *= 10; temp_value /= 10; }

    putch_key('0' + (int)(x / weight));
    x %= weight;
    weight /= 10;
    x += is_overflow;

    while (weight) {
        putch_key('0' + (int)(x / weight));
        x %= weight;
        weight /= 10;
    }
}

// ============================================================================
// Вывод double с 10 значащими цифрами
// ============================================================================

// Вычисляет количество знаков после запятой для формата %.*f,
// чтобы вывести ровно PRECISION значащих цифр.
static int get_precision(double val)
{
    if (val == 0.0) {
        return PRECISION;
    }

    double abs_val = fabs(val);
    int int_part_log = (int)log10(abs_val);
    int precision = PRECISION - int_part_log - 1;

    if (precision < 0) {
        return 0;
    }

    // Проверка на округление через разряд:
    // если при округлении произошёл перенос (9.999 -> 10.0),
    // цифр до запятой стало больше — нужна на одну цифру после запятой меньше.
    double power = pow(10.0, precision);
    double rounded_shifted = floor(abs_val * power + 0.5); 
    double rounded_val = rounded_shifted / power;
    int int_part_log_rounded = (int)log10(rounded_val);

    if (int_part_log_rounded > int_part_log) {
        precision--;
    }

    if (precision < 0) {
        precision = 0;
    }

    return precision;
}

static void write_double(double num)
{
    int precision = get_precision(num);
    printf("%.*f ", precision, num);
}


// ============================================================================
// ConsoleView — реализация
// ============================================================================

ConsoleView::ConsoleView() : m_a(0), m_b(0), m_c(0)
{
    consoleKeeper.ApplyRussianSettings(SE_ANSI_1251); 
}

ConsoleView::~ConsoleView() {}

// --- Баннер ---

void ConsoleView::showBanner()
{
    printf("============================================================\n");
    printf("  РЕШЕНИЕ КВАДРАТНОГО УРАВНЕНИЯ — Паттерн MVP + Сигналы/Слоты\n");
    printf("  Тип коэффициентов: %d бит\n", (int)sizeof(type) * 8);
    printf("============================================================\n");
}

// --- Ввод коэффициентов ---

void ConsoleView::promptCoefficients()
{
    printf("Введите коэффициенты квадратного уравнения (a, b, c):\n");

    printf("a = ");
    m_a = read_num();

    printf(", b = ");
    m_b = read_num();

    printf(", c = ");
    m_c = read_num();

    printf(".\n");

    // Уведомляем презентер: коэффициенты введены
    onCoefficientsReady.emit_();
}

// --- Эхо ввода ---

void ConsoleView::showCoefficients(type a, type b, type c)
{
    printf("Полученные коэффициенты: a = ");
    write_num(a);
    printf(", b = ");
    write_num(b);
    printf(", c = ");
    write_num(c);
    printf(".\n");
}

// --- Результаты ---

void ConsoleView::showLinearResult(double x, double verification)
{
    printf("Уравнение линейное. Ответ: x = ");
    write_double(x);
    printf("\n");
    printf("Проверка: b*x + c = ");
    write_double(verification);
    printf("\n");
}

void ConsoleView::showTwoRoots(double x1, double x2,
                               double v1, double v2)
{
    printf("Два корня: x1 = ");
    write_double(x1);
    printf(", x2 = ");
    write_double(x2);
    printf("\n");
    printf("Проверка X1: a*x^2 + b*x + c = ");
    write_double(v1);
    printf("\n");
    printf("Проверка X2: a*x^2 + b*x + c = ");
    write_double(v2);
    printf("\n");
}

void ConsoleView::showOneRoot(double x, double verification)
{
    printf("Один корень: x = ");
    write_double(x);
    printf("\n");
    printf("Проверка X: a*x^2 + b*x + c = ");
    write_double(verification);
    printf("\n");
}

void ConsoleView::showNoRealRoots()
{
    printf("Действительных корней нет (D < 0).\n");
}

void ConsoleView::showInfiniteSolutions()
{
    printf("Бесконечное количество решений (0 = 0).\n");
}

void ConsoleView::showNoSolutions()
{
    printf("Решений нет (константа не равна нулю).\n");
}

// --- Ошибки ---

void ConsoleView::showError(const char* message)
{
    printf("\n%s\n", message);
}

// --- Геттеры ---

type ConsoleView::getA() const { return m_a; }
type ConsoleView::getB() const { return m_b; }
type ConsoleView::getC() const { return m_c; }


// ============================================================================
// Фабричная функция
// ============================================================================

IView* createConsoleView() { return new ConsoleView(); }
