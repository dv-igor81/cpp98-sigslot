// ============================================================================
// DataPresenter.cpp -- Презентер формы редактирования данных
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "DataPresenter.h"

// ============================================================================
// Конструктор -- сохраняет зависимости и данные
// ============================================================================

DataPresenter::DataPresenter(IDataView* view, INavigator* navigator,
                             const SharedData& data)
    : m_view(view)
    , m_navigator(navigator)
    , m_data(data)
{
}

// ============================================================================
// Initialize -- подписка на сигналы вида + отображение данных
// ============================================================================

void DataPresenter::initialize()
{
    m_view->onGoHomeSignal().connect(this, &DataPresenter::handleGoHome);
    m_view->onGoResultSignal().connect(this, &DataPresenter::handleGoResult);
    m_view->onDataSubmittedSignal().connect(this, &DataPresenter::handleDataSubmitted);
    m_view->viewClosedSignal().connect(this, &DataPresenter::handleViewClosed);

    // Отображаем данные для редактирования
    m_view->displayData(m_data);
}

// ============================================================================
// Destroy -- отписка от сигналов вида
// ============================================================================

void DataPresenter::destroy()
{
    m_view->onGoHomeSignal().disconnect(this);
    m_view->onGoResultSignal().disconnect(this);
    m_view->onDataSubmittedSignal().disconnect(this);
    m_view->viewClosedSignal().disconnect(this);
}

// ============================================================================
// Обработчики навигации
// ВАЖНО: после вызова навигатора данный презентер может быть удалён.
// Поэтому после m_navigator->...() нет никакого кода.
// ============================================================================

void DataPresenter::handleGoHome()
{
    m_navigator->navigateToHome(&m_data);
}

void DataPresenter::handleGoResult()
{
    m_navigator->navigateToResult(&m_data);
}

// ============================================================================
// Обработчик «Применить» -- валидация и сохранение данных локально
// Навигация не происходит -- пользователь нажмёт кнопку сам.
// ============================================================================

void DataPresenter::handleDataSubmitted(const SharedData& data)
{
    m_data = data;

    // Валидация: счётчик не может быть меньше 1
    if (m_data.count < 1) m_data.count = 1;

    // Обновляем вид отредактированными (валидированными) данными
    m_view->displayData(m_data);
}

void DataPresenter::handleViewClosed()
{
    m_navigator->closeCurrentView(&m_data);
}
