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

    // --- IView: ввод данных ---
    void promptTimezone();
    void promptMode();
    void promptEpochInput();
    void promptDateTimeInput();

    // --- IView: вывод результатов и ошибок ---
    void showResult(const char* text);
    void showError(const char* message);

    // --- IView: геттеры ---
    int getTimezoneOffset() const;
    int getSelectedMode() const;
    unsigned int getEpochInput() const;
    const TDateTime& getDateTimeInput() const;

private:
    int      m_timezoneOffset;
    int      m_selectedMode;
    unsigned int m_epochInput;
    TDateTime m_dateTimeInput;
};

#endif // CONSOLE_VIEW_H
