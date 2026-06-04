// ============================================================================
// AppNavigator.h -- Навигатор приложения (create-on-demand)
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// Навигатор управляет:
//   1. Созданием форм по требованию (create-on-demand)
//   2. Полной сборкой MVP-триады (View + Presenter) при навигации
//   3. Жизненным циклом: Initialize/Destroy для подписки/отписки
//   4. Корректным завершением при закрытии главной формы
//
// navigateToHome() = «домой», т.е. на MainForm.
// MainForm может быть любой формой — определяется тем, куда
// навигируют первым. Создаётся через Application->CreateForm(),
// что автоматически назначает её Application->MainForm.
// Живёт всё время работы приложения. Закрытие MainForm завершает
// приложение.
//
// Остальные формы создаются через new при навигации и освобождаются
// (releaseView) при уходе с экрана.
//
// Таблица форм (s_formTable) устраняет switch-case.
// Для добавления новой формы:
//   1. Добавить значение в enum MainFormType (в .cpp)
//   2. Добавить статическую функцию-фабрику формы и презентера (в .cpp)
//   3. Добавить строку в s_formTable (в .cpp)
//   4. Добавить метод в INavigator
//
// Стратегия навигации: «замена» (не стек).
//
// ============================================================================

#ifndef APPNAVIGATOR_H
#define APPNAVIGATOR_H

#include "INavigator.h"
#include "IView.h"
#include "IPresenter.h"
#include "SharedData.h"

class AppNavigator : public INavigator {
public:
    AppNavigator();
    ~AppNavigator();

    // INavigator
    virtual void navigateToHome(const SharedData& data);
    virtual void navigateToData(const SharedData& data);
    virtual void navigateToResult(const SharedData& data);
    virtual void closeCurrentView(const SharedData& data);

private:
    // Тип формы -- индекс в таблице форм
    enum MainFormType { mftNone = -1, mftHome = 0, mftData = 1, mftResult = 2 };
    static const int FORM_COUNT = 3;

    // MVP-триада: связка вид + презентер
    struct MvpTriad {
        IView*      view;
        IPresenter* presenter;
        MvpTriad() : view(0), presenter(0) {}
    };

    // Кэш форм. Только MainForm кэшируется (живёт вечно).
    // Остальные формы создаются заново при каждом навигации.
    IView*        m_formCache[FORM_COUNT];
    MainFormType  m_mainFormType;

    // Текущая активная триада
    MvpTriad m_current;

    // Универсальная навигация (без switch-case)
    void navigateTo(MainFormType type, const SharedData& data);

    void activate(IView* view, IPresenter* presenter);
};

#endif // APPNAVIGATOR_H
