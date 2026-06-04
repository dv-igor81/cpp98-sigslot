// ============================================================================
// ResultPresenter.cpp -- Презентер формы результатов
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "ResultPresenter.h"

// ============================================================================
// Конструктор -- сохраняет зависимости и данные
// ============================================================================

ResultPresenter::ResultPresenter(IResultView* view, INavigator* navigator,
                                 const SharedData& data)
    : m_view(view)
    , m_navigator(navigator)
    , m_data(data)
{
}

// ============================================================================
// Initialize -- подписка на сигналы вида + отображение данных
// ============================================================================

void ResultPresenter::initialize()
{
    m_view->onGoHomeSignal().connect(this, &ResultPresenter::handleGoHome);
    m_view->onGoDataSignal().connect(this, &ResultPresenter::handleGoData);
    m_view->viewClosedSignal().connect(this, &ResultPresenter::handleViewClosed);

    // Отображаем результат обработки данных
    m_view->displayData(m_data);
}

// ============================================================================
// Destroy -- отписка от сигналов вида
// ============================================================================

void ResultPresenter::destroy()
{
    m_view->onGoHomeSignal().disconnect(this);
    m_view->onGoDataSignal().disconnect(this);
    m_view->viewClosedSignal().disconnect(this);
}

// ============================================================================
// Обработчики навигации
// ВАЖНО: после вызова навигатора данный презентер может быть удалён.
// Поэтому после m_navigator->...() нет никакого кода.
// ============================================================================

void ResultPresenter::handleGoHome()
{
    m_navigator->navigateToHome(m_data);
}

void ResultPresenter::handleGoData()
{
    m_navigator->navigateToData(m_data);
}

void ResultPresenter::handleViewClosed()
{
    m_navigator->closeCurrentView(m_data);
}
