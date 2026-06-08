// ============================================================================
// INavigator.h -- Интерфейс навигатора
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// Навигатор управляет переключением экранов и жизненным циклом MVP-триад.
// Каждый метод навигации принимает данные, которые будут переданы
// новому экрану через параметры.
//
// closeCurrentView() -- вызывается при закрытии вида (кнопка X).
//   Если текущий вид -- главная форма, приложение завершается.
//   Иначе -- переход на главную форму.
//
// ============================================================================

#ifndef INAVIGATOR_H
#define INAVIGATOR_H

#include "SharedData.h"

class INavigator {
public:
    virtual ~INavigator() {}

    // Навигация с передачей данных новому экрану
    virtual void navigateToHome(const void* dataPtr) = 0;
    virtual void navigateToData(const void* dataPtr) = 0;
    virtual void navigateToResult(const void* dataPtr) = 0;

    // Закрытие текущего вида (кнопка X или Exit)
    virtual void closeCurrentView(const void* dataPtr) = 0;
};

#endif // INAVIGATOR_H
