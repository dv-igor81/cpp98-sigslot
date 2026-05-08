// ============================================================================
// ConsoleView.h — Консольная реализация интерфейса IView
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// Это ЕДИНСТВЕННЫЙ модуль проекта, который общается с консолью.
// Все printf, getch, putch — только здесь.
// Презентер и модель ничего не знают о консоли.
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

    // --- IView: ввод ---
    void promptCoefficients();

    // --- IView: эхо ввода ---
    void showCoefficients(type a, type b, type c);

    // --- IView: результаты ---
    void showLinearResult(double x, double verification);
    void showTwoRoots(double x1, double x2, double v1, double v2);
    void showOneRoot(double x, double verification);
    void showNoRealRoots();
    void showInfiniteSolutions();
    void showNoSolutions();

    // --- IView: ошибки ---
    void showError(const char* message);

    // --- IView: геттеры ---
    type getA() const;
    type getB() const;
    type getC() const;

private:
    type m_a, m_b, m_c;
};

#endif // CONSOLE_VIEW_H
