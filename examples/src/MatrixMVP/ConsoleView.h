// ============================================================================
// ConsoleView.h Ч  онсольна€ реализаци€ интерфейса IView
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// Ёто ≈ƒ»Ќ—“¬≈ЌЌџ… модуль проекта, который общаетс€ с консолью.
// ¬се printf, getch, putch Ч только здесь.
// ѕрезентер и модель ничего не знают о консоли.
//
// ConsoleView наследует IView и реализует все чисто виртуальные методы:
//   Х showBanner()          Ч выводит приветствие
//   Х promptDimensions()    Ч запрашивает размерности у пользовател€
//   Х promptMatrixA/B()     Ч запрашивает элементы матриц
//   Х showResult()          Ч выводит матрицу-результат
//   Х showError()           Ч выводит сообщение об ошибке
//   Х showDimensionMismatch Ч выводит ошибку несовместимых размерностей
//   Х showInvalidDimensions Ч выводит ошибку некорректных размерностей
//   Х showOverflowWarning() Ч выводит предупреждение о переполнении
//
// ============================================================================

#ifndef CONSOLE_VIEW_H
#define CONSOLE_VIEW_H

#include "IView.h"

class ConsoleView : public IView {
public:
    ConsoleView();
    ~ConsoleView();

    // --- IView: баннер ---
    void showBanner();

    // --- IView: ввод данных ---
    void promptDimensions();
    void promptMatrixA();
    void promptMatrixB();

    // --- IView: вывод результатов и ошибок ---
    void showResult(const typeres* data, int rows, int cols);
    void showError(const char* message);
    void showDimensionMismatch(int cA, int rB);
    void showInvalidDimensions();
    void showOverflowWarning();

    // --- IView: геттеры ---
    int getRowsA() const;
    int getColsA() const;
    int getRowsB() const;
    int getColsB() const;
    const type* getMatrixAData() const;
    const type* getMatrixBData() const;

private:
    int m_rA, m_cA, m_rB, m_cB;
    type* m_matA;  // ѕлоский массив (row-major)
    type* m_matB;

    static type* allocMatrix(int rows, int cols);
    static void freeMatrix(type*& m);
};

#endif // CONSOLE_VIEW_H
