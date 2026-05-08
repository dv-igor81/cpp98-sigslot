// ============================================================================
// IView.h Ч »нтерфейсы видов дл€ проекта FormNavigator
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// »ерархи€:
//
//   IView          Ч базовый (showForm, hideForm, updateData)
//     IHomeView    Ч сигналы: onGoData, onGoResult, onExit
//     IDataView    Ч сигналы: onGoHome, onGoResult, onDataSubmitted
//     IResultView  Ч сигналы: onGoHome, onGoData
//
//  аждый конкретный вид реализует “ќЋ№ ќ свой интерфейс Ч нет лишних
// сигналов и методов.
//
// ============================================================================

#ifndef IVIEW_H
#define IVIEW_H

#include "signal_impl.h"
#include "NavigatorTypes.h"

// ============================================================================
// IView Ч базовый интерфейс вида (чистый, без данных)
// ============================================================================

class IView {
public:
    virtual void showForm() = 0;
    virtual void hideForm() = 0;
    virtual void updateData(const SharedData& data) = 0;
};

// ============================================================================
// IHomeView Ч интерфейс главной формы
// —игналы: переход к данным, переход к результатам, выход
// ============================================================================

class IHomeView : public IView {
public:
    virtual signals::Signal0& onGoDataSignal() = 0;
    virtual signals::Signal0& onGoResultSignal() = 0;
    virtual signals::Signal0& onExitSignal() = 0;
};

// ============================================================================
// IDataView Ч интерфейс формы редактировани€ данных
// —игналы: переход на главную, переход к результатам, данные введены
// ============================================================================

class IDataView : public IView {
public:
    virtual signals::Signal0& onGoHomeSignal() = 0;
    virtual signals::Signal0& onGoResultSignal() = 0;
    virtual signals::Signal1<const SharedData&>& onDataSubmittedSignal() = 0;
};

// ============================================================================
// IResultView Ч интерфейс формы результатов
// —игналы: переход на главную, переход к данным
// ============================================================================

class IResultView : public IView {
public:
    virtual signals::Signal0& onGoHomeSignal() = 0;
    virtual signals::Signal0& onGoDataSignal() = 0;
};

#endif // IVIEW_H
