// ============================================================================
// ConsoleView.cpp — Консольная реализация IView
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// ЕДИНСТВЕННЫЙ модуль, работающий с консолью.
// Ни презентер, ни модель, ни main() не содержат ни одного printf/getch.
// Все пользовательские сообщения и весь ввод — только здесь.
//
// ПРИНЦИП РАБОТЫ:
//
//   1. Презентер вызывает promptDimensions() -> мы спрашиваем пользователя
//      через консоль, сохраняем ответ и излучаем сигнал onDimensionsEntered.
//
//   2. Презентер вызывает promptMatrixA/B() -> мы запрашиваем элементы,
//      сохраняем их в памяти и излучаем сигнал onMatrixAReady/onMatrixBReady.
//
//   3. Презентер вызывает showResult() -> мы выводим матрицу на консоль.
//
//   4. Презентер вызывает showError/showDimensionMismatch/и т.д. ->
//      мы выводим соответствующее сообщение.
//
//   Геттеры (getMatrixAData и т.д.) позволяют презентеру забрать
//   введённые данные и передать их в модель.
//
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// ============================================================================
// ПОДКЛЮЧЕНИЕ БИБЛИОТЕК
// ============================================================================
#include <stdio.h>  /* Для функций ввода/вывода (printf, getchar, putchar) */
#include <stdlib.h> /* Для функций работы с динамической памятью (malloc, free) */

#include "ConsoleView.h"
#include "MatrixTypes.h"

#include "console_settings_keeper.h"

static void putch_key(int ch);
static int getch_key(void);
static type read_num(void);
static void write_typeres_aligned(typeres num, int width);

// ============================================================================
// ConsoleView — реализация
// ============================================================================

//using namespace signals;

ConsoleView::ConsoleView()
    : m_rA(0), m_cA(0), m_rB(0), m_cB(0),
      m_matA(NULL), m_matB(NULL)
{
    consoleKeeper.ApplyRussianSettings(SE_ANSI_1251); 
}

ConsoleView::~ConsoleView()
{
    freeMatrix(m_matA);
    freeMatrix(m_matB);
}

// --- Баннер: показывается при запуске приложения ---

void ConsoleView::showBanner()
{
    printf("============================================================\n");
    printf("  УМНОЖЕНИЕ МАТРИЦ — Паттерн MVP + Сигналы/Слоты\n");
    printf("  Тип элемента: %d бит, тип результата: %d бит\n",
           (int)sizeof(type) * 8, (int)sizeof(typeres) * 8);
    printf("============================================================\n");
}

// --- Ввод размерностей ---

void ConsoleView::promptDimensions()
{
    printf("Введите количество строк матрицы A (макс. %d): ", MAX_DIM);
    m_rA = read_num();
    printf("\nВведите количество столбцов матрицы A (макс. %d): ", MAX_DIM);
    m_cA = read_num();
    printf("\nВведите количество строк матрицы B (макс. %d): ", MAX_DIM);
    m_rB = read_num();
    printf("\nВведите количество столбцов матрицы B (макс. %d): ", MAX_DIM);
    m_cB = read_num();
    printf("\n");
    // Уведомляем презентер: пользователь ввёл размерности
    onDimensionsEntered.emit_(m_rA, m_cA, m_rB, m_cB);
}

// --- Ввод элементов матрицы A ---

void ConsoleView::promptMatrixA()
{
    int i, j;
    freeMatrix(m_matA);
    m_matA = allocMatrix(m_rA, m_cA);
    printf("\nВведите элементы матрицы A (%dx%d):\n", m_rA, m_cA);
    for (i = 0; i < m_rA; i++) {
        for (j = 0; j < m_cA; j++) {
            printf("A[%d][%d]: ", i + 1, j + 1);
            m_matA[i * m_cA + j] = read_num();
            printf("\n");
        }
    }
    // Уведомляем презентер: матрица A готова
    onMatrixAReady.emit_();
}

// --- Ввод элементов матрицы B ---

void ConsoleView::promptMatrixB()
{
    int i, j;
    freeMatrix(m_matB);
    m_matB = allocMatrix(m_rB, m_cB);
    printf("\nВведите элементы матрицы B (%dx%d):\n", m_rB, m_cB);
    for (i = 0; i < m_rB; i++) {
        for (j = 0; j < m_cB; j++) {
            printf("B[%d][%d]: ", i + 1, j + 1);
            m_matB[i * m_cB + j] = read_num();
            printf("\n");
        }
    }
    // Уведомляем презентер: матрица B готова
    onMatrixBReady.emit_();
}

// --- Вывод результата ---

void ConsoleView::showResult(const typeres* data, int rows, int cols)
{
    int i, j;
    printf("\nРезультат умножения матриц A (%dx%d) и B (%dx%d):\n",
           m_rA, m_cA, m_rB, m_cB);
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            write_typeres_aligned(data[i * cols + j], 14);
            putch_key(' ');
        }
        putch_key('\r');
        putch_key('\n');
    }
}

// --- Вывод ошибок ---

void ConsoleView::showError(const char* message)
{
    printf("\n%s\n", message);
}

void ConsoleView::showDimensionMismatch(int cA, int rB)
{
    printf("\nОшибка: число столбцов матрицы A (%d) не равно"
           " числу строк матрицы B (%d).\n", cA, rB);
}

void ConsoleView::showInvalidDimensions()
{
    printf("\nОшибка: размерности должны быть от 1 до %d.\n", MAX_DIM);
}

void ConsoleView::showOverflowWarning()
{
    printf("\nВНИМАНИЕ: В процессе вычислений произошло переполнение типа typeres!\n");
    printf("Отображенные значения могут быть математически неверными.\n");
    printf("Попробуйте использовать матрицы с меньшими по модулю элементами.\n");
}

// --- Геттеры: презентер забирает введённые данные ---

int ConsoleView::getRowsA() const { return m_rA; }
int ConsoleView::getColsA() const { return m_cA; }
int ConsoleView::getRowsB() const { return m_rB; }
int ConsoleView::getColsB() const { return m_cB; }
const type* ConsoleView::getMatrixAData() const { return m_matA; }
const type* ConsoleView::getMatrixBData() const { return m_matB; }

// --- Вспомогательные методы управления памятью ---

type* ConsoleView::allocMatrix(int rows, int cols)
{
    type* m = (type*)malloc((size_t)rows * cols * sizeof(type));
    if (!m) { printf("\nОшибка: не хватило памяти!\n"); exit(1); }
    return m;
}

void ConsoleView::freeMatrix(type*& m)
{
    if (m) { free(m); m = NULL; }
}


// Безопасный ввод числа типа type (посимвольно, с защитой от переполнения)
static type read_num(void)
{
    type accumulator = 0;
    type limit;
    int key;
    int pressed_digit;
    int is_blocked;
    int is_signed_type = ((type)-1 < 0);
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
            if (is_signed_type) {
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

// Безопасный вывод typeres (защита от числа-Феникса)
static void write_typeres(typeres x)
{
    typeres temp_value;
    typeres weight = 1;
    int is_overflow = 0;
    int is_neg = 0;
    typeres max = g_TYPERES_MAX_SIGNED;

    computeTypeLimits();

    if ((x < max) && (x & ~max) != 0) is_neg = 1;

    if (is_neg) {
        putch_key('-');
        x = (typeres)(~x + 1);
        is_neg = (x & ~max) != 0;
        if (is_neg) {
            is_overflow = 1;
            x++;
            x = (typeres)(~x + 1);
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

// Вывод typeres с выравниванием по правому краю
static void write_typeres_aligned(typeres num, int width)
{
    int digits = 0;
    typeres temp = num;
    int i;
    int is_neg = 0;
    typeres max = g_TYPERES_MAX_SIGNED;

    computeTypeLimits();

    if (temp == 0) {
        digits = 1;
    } else {
        if ((temp < max) && (temp & ~max) != 0) {
            is_neg = 1;
            temp = ~temp + 1;
            if ((temp & ~max) != 0) {
                temp = temp / -10;
                digits = 1;
            }
        }
        while (temp > 0) { digits++; temp /= 10; }
        digits += is_neg;
    }

    for (i = digits; i < width; i++) putch_key(' ');
    write_typeres(num);
}

#ifdef _WIN32
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

// ============================================================================
// Вспомогательные функции ввода/вывода
// ============================================================================
// Эти функции статические — видны только в данном .cpp файле.
// Они не являются частью интерфейса IView и используются исключительно
// для реализации консольного ввода/вывода.

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
// Фабричная функция
// ============================================================================
// Создаёт ConsoleView в куче и возвращает указатель на базовый интерфейс.
// Вызывающий код (main) не знает о классе ConsoleView — только об IView.

IView* createConsoleView() { return new ConsoleView(); }
