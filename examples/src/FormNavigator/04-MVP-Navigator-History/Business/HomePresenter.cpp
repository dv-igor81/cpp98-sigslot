// ============================================================================
// HomePresenter.cpp -- Презентер главной формы
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "HomePresenter.h"

// ============================================================================
// Конструктор -- сохраняет зависимости и данные
// ============================================================================

HomePresenter::HomePresenter(IHomeView* view, INavigator* navigator,
                             const SharedData& data)
    : m_view(view)
    , m_navigator(navigator)
    , m_data(data)
{
}

// ============================================================================
// Initialize -- подписка на сигналы вида + отображение данных
// ============================================================================

void HomePresenter::initialize()
{
    m_view->onGoDataSignal().connect(this, &HomePresenter::handleGoData);
    m_view->onGoResultSignal().connect(this, &HomePresenter::handleGoResult);
    m_view->onExitSignal().connect(this, &HomePresenter::handleExit);
    m_view->viewClosedSignal().connect(this, &HomePresenter::handleViewClosed);

    // Отображаем данные при активации экрана
    m_view->displayData(m_data);
}

// ============================================================================
// Destroy -- отписка от сигналов вида
// Критически важно для предотвращения утечек памяти!
// ============================================================================

void HomePresenter::destroy()
{
    m_view->onGoDataSignal().disconnect(this);
    m_view->onGoResultSignal().disconnect(this);
    m_view->onExitSignal().disconnect(this);
    m_view->viewClosedSignal().disconnect(this);
}

// ============================================================================
// Обработчики навигации
// ВАЖНО: после вызова навигатора данный презентер может быть удалён.
// Поэтому после m_navigator->...() нет никакого кода.
// ============================================================================

void HomePresenter::handleGoData()
{
    m_navigator->navigateToData(&m_data);
}

void HomePresenter::handleGoResult()
{
    m_navigator->navigateToResult(&m_data);
}

void HomePresenter::handleExit()
{
    m_navigator->closeCurrentView(&m_data);
}

void HomePresenter::handleViewClosed()
{
    m_navigator->closeCurrentView(&m_data);
}
