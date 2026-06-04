// ============================================================================
// AppNavigator.cpp -- Навигатор приложения
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
// Конструктор -- сохраняет интерфейсные указатели на формы
// ============================================================================

AppNavigator::AppNavigator(THomeForm* home, TDataForm* data,
                           TResultForm* result)
    : m_homeView(static_cast<IHomeView*>(home))
    , m_dataView(static_cast<IDataView*>(data))
    , m_resultView(static_cast<IResultView*>(result))
{
}

// ============================================================================
// Деструктор -- очистка текущей триады
// Вызывается при выходе из WinMain (stack-объект)
// ============================================================================

AppNavigator::~AppNavigator()
{
    if (m_current.presenter) {
        m_current.presenter->destroy();
        delete m_current.presenter;
        m_current.presenter = 0;
    }
    m_current.view = 0;
}

// ============================================================================
// activate -- создать новую триаду, инициализировать, показать
//
// Порядок: сначала показать новую форму, потом скрыть старую --
// это предотвращает мерцание.
//
// ВАЖНО: oldPresenter->destroy() и delete вызываются ПОСЛЕ того,
// как новая триада активирована. Если мы находимся внутри
// обработчика старого презентера (сигнал), то после delete
// метод-обработчик просто возвращается -- это безопасно,
// т.к. после вызова навигатора нет никакого кода.
// ============================================================================

void AppNavigator::activate(IView* view, IPresenter* presenter)
{
    // Сохранить старую триаду для отложенной деактивации
    IView*      oldView      = m_current.view;
    IPresenter* oldPresenter = m_current.presenter;

    // Активировать новую триаду
    m_current.view      = view;
    m_current.presenter = presenter;
    presenter->initialize();    // Подписка на сигналы, отображение данных
    view->showView();           // Показать форму

    // Деактивировать старую триаду
    if (oldPresenter) {
        oldPresenter->destroy();    // Отписка от сигналов
        delete oldPresenter;        // Освобождение памяти
    }
    if (oldView) {
        oldView->hideView();        // Скрыть форму
    }
}

// ============================================================================
// Навигация на HomeForm
// ============================================================================

void AppNavigator::navigateToHome(const SharedData& data)
{
    IHomeView* view = m_homeView;
    HomePresenter* presenter = new HomePresenter(view, this, data);
    activate(static_cast<IView*>(view), presenter);
}

// ============================================================================
// Навигация на DataForm
// ============================================================================

void AppNavigator::navigateToData(const SharedData& data)
{
    IDataView* view = m_dataView;
    DataPresenter* presenter = new DataPresenter(view, this, data);
    activate(static_cast<IView*>(view), presenter);
}

// ============================================================================
// Навигация на ResultForm
// ============================================================================

void AppNavigator::navigateToResult(const SharedData& data)
{
    IResultView* view = m_resultView;
    ResultPresenter* presenter = new ResultPresenter(view, this, data);
    activate(static_cast<IView*>(view), presenter);
}

// ============================================================================
// closeCurrentView -- обработка закрытия текущего вида
//
// Если закрывается HomeForm -- это последняя форма, выходим из приложения.
// Если DataForm/ResultForm -- переходим на HomeForm с текущими данными.
// ============================================================================

void AppNavigator::closeCurrentView(const SharedData& data)
{
    IView* homeViewIf = static_cast<IView*>(m_homeView);

    if (m_current.view == homeViewIf) {
        // Закрыта последняя форма -- завершить приложение
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
    } else {
        // Переход на главную форму (аналог возврата по стеку)
        navigateToHome(data);
    }
}
