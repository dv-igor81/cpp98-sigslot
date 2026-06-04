// ============================================================================
// IView.h -- Интерфейсы видов для проекта Navigator
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// Иерархия:
//
//   IView          -- базовый (showView, hideView, releaseView, viewClosedSignal)
//     IHomeView    -- сигналы: onGoData, onGoResult, onExit
//     IDataView    -- сигналы: onGoHome, onGoResult, onDataSubmitted
//     IResultView  -- сигналы: onGoHome, onGoData
//
// releaseView() -- отложенное удаление формы (VCL TForm::Release).
//   Используется при create-on-demand навигации для безопасного
//   удаления формы после завершения цепочки сигналов.
//
// ============================================================================

#ifndef IVIEW_H
#define IVIEW_H

#include "signal_impl.h"
#include "SharedData.h"

// ============================================================================
// IView -- базовый интерфейс вида
// ============================================================================

class IView {
public:
    virtual void showView() = 0;
    virtual void hideView() = 0;

    // Отложенное удаление формы (VCL: TForm::Release).
    // Форма будет удалена после завершения текущей обработки сообщений.
    // Это безопасно при вызове внутри обработчика сигнала.
    virtual void releaseView() = 0;

    // Сигнал: вид закрыт (кнопка X на форме)
    virtual signals::Signal0& viewClosedSignal() = 0;
};

// ============================================================================
// IHomeView -- интерфейс главной формы
// Сигналы: переход к данным, переход к результатам, выход
// ============================================================================

class IHomeView : public IView {
public:
    // Отобразить данные на форме
    virtual void displayData(const SharedData& data) = 0;

    virtual signals::Signal0& onGoDataSignal() = 0;
    virtual signals::Signal0& onGoResultSignal() = 0;
    virtual signals::Signal0& onExitSignal() = 0;
};

// ============================================================================
// IDataView -- интерфейс формы редактирования данных
// Сигналы: переход на главную, переход к результатам, данные введены
// ============================================================================

class IDataView : public IView {
public:
    // Отобразить данные для редактирования
    virtual void displayData(const SharedData& data) = 0;

    virtual signals::Signal0& onGoHomeSignal() = 0;
    virtual signals::Signal0& onGoResultSignal() = 0;
    virtual signals::Signal1<const SharedData&>& onDataSubmittedSignal() = 0;
};

// ============================================================================
// IResultView -- интерфейс формы результатов
// Сигналы: переход на главную, переход к данным
// ============================================================================

class IResultView : public IView {
public:
    // Отобразить результат обработки данных
    virtual void displayData(const SharedData& data) = 0;

    virtual signals::Signal0& onGoHomeSignal() = 0;
    virtual signals::Signal0& onGoDataSignal() = 0;
};

#endif // IVIEW_H
