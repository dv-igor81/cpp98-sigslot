// ============================================================================
// AppNavigator.cpp -- Навигатор приложения (create-on-demand)
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <vcl.h>
#include "AppNavigator.h"
#include "HomeForm.h"
#include "DataForm.h"
#include "ResultForm.h"
#include "HomePresenter.h"
#include "DataPresenter.h"
#include "ResultPresenter.h"

// ============================================================================
// Таблица форм -- устраняет switch-case
//
// Каждая строка: фабрика формы + фабрика презентера.
// Индекс в массиве = значение enum MainFormType.
//
// Для добавления новой формы:
//   1. Добавить значение в enum MainFormType (в .h)
//   2. Написать две статические функции-фабрики ниже
//   3. Добавить строку в s_formTable
// ============================================================================

// ============================================================================
// Конструктор
// ============================================================================

AppNavigator::AppNavigator()
    : m_currentType(VT_COUNT), m_backStackCount(0), m_isNavigatingBack(false)
{
    // Инициализация таблицы навигации.
    // Индекс в массиве строго соответствует значению enum ViewType.
    m_navigateTable[VT_HOME]   = &AppNavigator::navigateToHome;
    m_navigateTable[VT_DATA]   = &AppNavigator::navigateToData;
    m_navigateTable[VT_RESULT] = &AppNavigator::navigateToResult;

    for (int i = 0; i < MAX_HISTORY; ++i) {
        m_backStack[i] = VT_COUNT;
    }
}

// ============================================================================
// Деструктор
// ============================================================================

AppNavigator::~AppNavigator()
{
    if (m_current.presenter) {
        m_current.presenter->destroy();
        delete m_current.presenter;
    }
}

// ============================================================================
// СОБСТВЕННЫЙ ЦИКЛ СООБЩЕНИЙ ВМЕСТО Application->Run()
// ============================================================================

void AppNavigator::Run()
{
    // Крутим цикл, пока приложение не завершено
    while (!Application->Terminated)
    {
        Application->HandleMessage(); // VCL обработает сообщения
    }
}

// ============================================================================
// Вспомогательная функция: перевод enum в текст
// ============================================================================
const char* AppNavigator::viewTypeName(ViewType type)
{
    switch(type) {
        case VT_HOME:   return "Home";
        case VT_DATA:   return "Data";
        case VT_RESULT: return "Result";
        case VT_COUNT:  return "COUNT(Invalid)";
        default:        return "Unknown";
    }
}

// ============================================================================
// Поиск экрана в стеке истории
// ============================================================================
int AppNavigator::findInStack(ViewType type) const
{
    for (int i = 0; i < m_backStackCount; ++i) {
        if (m_backStack[i] == type) {
            //==//String msg = "[Nav] findInStack: Found [" + String(viewTypeName(type)) + "] at index " + IntToStr(i);
            //==//OutputDebugString(msg.c_str());
            return i;
        }
    }
    //==//String msg = "[Nav] findInStack: [" + String(viewTypeName(type)) + "] NOT FOUND in stack.";
    //==//OutputDebugString(msg.c_str());
    return -1;
}

// ============================================================================
// Активация триады (ИЕРАРХИЧЕСКАЯ НАВИГАЦИЯ)
// ============================================================================
void AppNavigator::activate(IView* view, IPresenter* presenter, ViewType newType)
{
    if (!view || !presenter) {
        return; // Тут можно сообщить об ошибке
    }

    // Обрабатываем стек только при движении ВПЕРЕД (не по кнопке X)
    if (!m_isNavigatingBack && m_current.view && m_currentType < VT_COUNT) 
    {
        int stackIndex = findInStack(newType);

        if (stackIndex != -1) 
        {
            // Мы идем ВПЕРЕД на экран, который уже есть в стеке (например, с Result на Data).
            // Это означает движение "вверх" по иерархии. 
            // Обрезаем стек до этого экрана, чтобы "нижние" закрытые формы никогда не повторились.
            //==//String msg = "[Nav] activate: Moving UP to [" + String(viewTypeName(newType)) +
            //==//             "]. Trimming stack to " + IntToStr(stackIndex) + " items.";
            //==//OutputDebugString(msg.c_str());
                         
            m_backStackCount = stackIndex;
            // Текущую форму в стек НЕ добавляем, она теперь вершина
        } 
        else 
        {
            // Мы идем ВПЕРЕД на новый экран, которого нет в стеке (например, с Data на Result).
            // Добавляем текущую форму в стек как "точку возврата".
            //==//String msg = "[Nav] activate: Moving DOWN to [" + String(viewTypeName(newType)) +
            //==//             "]. Pushing [" + String(viewTypeName(m_currentType)) + "] to stack.";
            //==//OutputDebugString(msg.c_str());
                         
            if (m_backStackCount < MAX_HISTORY) {
                m_backStack[m_backStackCount++] = m_currentType;
            }
        }
    }
    m_isNavigatingBack = false;

    IView* oldView = m_current.view;
    IPresenter* oldPresenter = m_current.presenter;

    m_current.view = view;
    m_current.presenter = presenter;
    m_currentType = newType;

    presenter->initialize();
    view->showView();

    if (oldPresenter) {
        oldPresenter->destroy();
        delete oldPresenter;
    }
    if (oldView) {
        oldView->releaseView();
    }
}

// ============================================================================
// Навигация (с защитой от перехода на саму себя)
// ============================================================================

void AppNavigator::navigateToHome(const void* dataPtr)
{
    // Защита: если мы уже находимся на этой форме, игнорируем переход
    if (m_currentType == VT_HOME) {
        OutputDebugString("[Nav] navigateToHome: Already on Home. Ignoring transition.");
        return; 
    }

    const SharedData& data = *static_cast<const SharedData*>(dataPtr);
    IHomeView* view = static_cast<IHomeView*>(new THomeForm(Application));
    IPresenter* presenter = new HomePresenter(view, this, data);
    activate(view, presenter, VT_HOME);
}

void AppNavigator::navigateToData(const void* dataPtr)
{
    // Защита: если мы уже находимся на этой форме, игнорируем переход
    if (m_currentType == VT_DATA) {
        OutputDebugString("[Nav] navigateToData: Already on Data. Ignoring transition.");
        return; 
    }

    const SharedData& data = *static_cast<const SharedData*>(dataPtr);
    IDataView* view = static_cast<IDataView*>(new TDataForm(Application));
    IPresenter* presenter = new DataPresenter(view, this, data);
    activate(view, presenter, VT_DATA);
}

void AppNavigator::navigateToResult(const void* dataPtr)
{
    // Защита: если мы уже находимся на этой форме, игнорируем переход
    if (m_currentType == VT_RESULT) {
        OutputDebugString("[Nav] navigateToResult: Already on Result. Ignoring transition.");
        return; 
    }

    const SharedData& data = *static_cast<const SharedData*>(dataPtr);
    IResultView* view = static_cast<IResultView*>(new TResultForm(Application));
    IPresenter* presenter = new ResultPresenter(view, this, data);
    activate(view, presenter, VT_RESULT);
}



// ============================================================================
// closeCurrentView -- обработка закрытия текущего вида
//
// Если закрывается MainForm -- приложение завершается.
// Иначе -- переход на MainForm (которая может быть любой).
// ============================================================================

void AppNavigator::closeCurrentView(const void* dataPtr)
{
    // Если стек пуст — мы на корневой форме, завершаем приложение
    if (m_backStackCount == 0)
    {
        if (m_current.presenter) {
            m_current.presenter->destroy();
            delete m_current.presenter;
            m_current.presenter = 0;
        }
        if (m_current.view) {
            m_current.view->hideView();
            m_current.view = 0;
        }
        Application->Terminate();
    }
    else
    {
        // Идем назад: запрещаем запись в историю и достаем предыдущий тип
        m_isNavigatingBack = true;
        ViewType backTarget = m_backStack[--m_backStackCount];
        
        // Вызов метода навигации по индексу из таблицы
        (this->*m_navigateTable[backTarget])(dataPtr);
    }
}
