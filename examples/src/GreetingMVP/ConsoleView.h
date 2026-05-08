// ============================================================================
// ConsoleView.h — Консольная реализация интерфейса IView
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// Это ЕДИНСТВЕННЫЙ модуль проекта, который общается с консолью.
// Все printf, scanf, fgets — только здесь.
// Презентер и модель ничего не знают о консоли.
//
// ДВА СПОСОБА ВВОДА (демонстрация):
//   1. Имя — через scanf(" %99[^\n]", name)
//   2. Фамилия — через fgets(name, sizeof(name), stdin)
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
    void promptName();
    void promptSurname();

    // --- IView: вывод результатов и ошибок ---
    void showGreeting(const char* greeting);
    void showError(const char* message);

    // --- IView: геттеры ---
    const char* getName() const;
    const char* getSurname() const;

private:
    char m_name[MAX_NAME_LEN];
    char m_surname[MAX_NAME_LEN];
};

#endif // CONSOLE_VIEW_H
