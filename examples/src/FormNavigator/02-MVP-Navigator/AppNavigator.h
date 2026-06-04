// ============================================================================
// AppNavigator.h -- Навигатор приложения
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// Навигатор управляет:
//   1. Полной сборкой MVP-триады (View + Presenter) при навигации
//   2. Жизненным циклом: Initialize/Destroy для подписки/отписки
//   3. Корректным завершением при закрытии последней формы (HomeForm)
//
// Формы создаются один раз и живут всё время работы приложения.
// Презентеры создаются при навигации и уничтожаются при уходе с экрана.
//
// Стратегия навигации: «замена» (не стек).
// При навигации: показать новую форму -> скрыть старую форму
// (порядок важен для избежания мерцания).
//
// ============================================================================

#ifndef APPNAVIGATOR_H
#define APPNAVIGATOR_H

#include "INavigator.h"
#include "IView.h"
#include "IPresenter.h"
#include "SharedData.h"

class THomeForm;
class TDataForm;
class TResultForm;

class AppNavigator : public INavigator {
public:
    AppNavigator(THomeForm* home, TDataForm* data, TResultForm* result);
    ~AppNavigator();

    // INavigator
    virtual void navigateToHome(const SharedData& data);
    virtual void navigateToData(const SharedData& data);
    virtual void navigateToResult(const SharedData& data);
    virtual void closeCurrentView(const SharedData& data);

private:
    // MVP-триада: связка вид + презентер
    struct MvpTriad {
        IView*      view;
        IPresenter* presenter;

        MvpTriad() : view(0), presenter(0) {}
    };

    // Интерфейсные указатели на три формы (каждая реализует свой I*View)
    IHomeView*   m_homeView;
    IDataView*   m_dataView;
    IResultView* m_resultView;

    // Текущая активная триада
    MvpTriad m_current;

    // Внутренние методы
    void activate(IView* view, IPresenter* presenter);
};

#endif // APPNAVIGATOR_H
