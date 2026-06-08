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
// навигируют первым. Создаётся НЕ через Application->CreateForm(),
// а через оператор new.
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



class AppNavigator : public INavigator {
public:
    AppNavigator();
    ~AppNavigator();

    void Run();

    // INavigator
    virtual void navigateToHome(const void* dataPtr);
    virtual void navigateToData(const void* dataPtr);
    virtual void navigateToResult(const void* dataPtr);
    virtual void closeCurrentView(const void* dataPtr);

private:
    // Типы форм для истории навигации (Индексы от 0 до N-1)
    enum ViewType {
        VT_HOME = 0,   // 0
        VT_DATA,       // 1
        VT_RESULT,     // 2
        VT_COUNT       // 3 - Количество доступных триад (для размера массива)
    };

    // MVP-триада: связка вид + презентер
    struct MvpTriad {
        IView*      view;
        IPresenter* presenter;
        MvpTriad() : view(0), presenter(0) {}
    };

    // Текущая активная триада
    MvpTriad m_current;
    ViewType m_currentType;    

    // Стек навигации (на базе обычного массива)
    static const int MAX_HISTORY = 16;
    ViewType m_backStack[MAX_HISTORY];
    int m_backStackCount;

    bool m_isNavigatingBack; // Флаг защиты от зацикливания истории

    // Тип указателя на метод навигации класса AppNavigator
    typedef void (AppNavigator::*NavigateFunc)(const void*);
    // Таблица методов навигации (массив указателей на функции-члены)
    NavigateFunc m_navigateTable[VT_COUNT];

    int AppNavigator::findInStack(ViewType type) const;
    void activate(IView* view, IPresenter* presenter, ViewType newType);
    const char* viewTypeName(ViewType type);    
};

#endif // APPNAVIGATOR_H
