//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Presenter.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// ============================================================================
// Конструктор — подключение сигналов через специфичные интерфейсы
// ============================================================================

Presenter::Presenter(IHomeView* home, IDataView* data, IResultView* result, IModel* model)
    : m_home(home)
    , m_data(data)
    , m_result(result)
    , m_model(model)
    , m_current(0)
{
    // --- HomeForm: onGoData, onGoResult, onExit ---
    m_home->onGoDataSignal().connect(this, &Presenter::handleGoData);
    m_home->onGoResultSignal().connect(this, &Presenter::handleGoResult);
    m_home->onExitSignal().connect(this, &Presenter::handleExit);

    // --- DataForm: onGoHome, onGoResult, onDataSubmitted ---
    m_data->onGoHomeSignal().connect(this, &Presenter::handleGoHome);
    m_data->onGoResultSignal().connect(this, &Presenter::handleGoResult);
    m_data->onDataSubmittedSignal().connect(this, &Presenter::handleDataSubmitted);

    // --- ResultForm: onGoHome, onGoData ---
    m_result->onGoHomeSignal().connect(this, &Presenter::handleGoHome);
    m_result->onGoDataSignal().connect(this, &Presenter::handleGoData);

    // --- Сигнал модели ---
    m_model->onDataChanged.connect(this, &Presenter::handleDataChanged);

    // --- Начальное состояние: показать HomeForm ---
    switchTo(m_home);
}

// ============================================================================
// Навигация
// ============================================================================

void Presenter::handleGoHome()
{
    switchTo(m_home);
}

void Presenter::handleGoData()
{
    switchTo(m_data);
}

void Presenter::handleGoResult()
{
    switchTo(m_result);
}

// ============================================================================
// Данные — передать введённые данные в модель
// ============================================================================

void Presenter::handleDataSubmitted(const SharedData& data)
{
    m_model->setData(data);
}

// ============================================================================
// Данные модели изменились — обновить все виды
// ============================================================================

void Presenter::handleDataChanged(const SharedData& data)
{
    m_home->updateData(data);
    m_data->updateData(data);
    m_result->updateData(data);
}

// ============================================================================
// Выход из приложения
// ============================================================================

void Presenter::handleExit()
{
    Application->Terminate();
}

// ============================================================================
// Переключение формы (сначала показать новую, потом скрыть старую)
// ============================================================================

void Presenter::switchTo(IView* target)
{
    if (m_current == target) return;

    target->showForm();
    if (m_current) m_current->hideForm();

    m_current = target;
}

// ============================================================================
// Обновление всех видов данными из модели
// ============================================================================

void Presenter::updateAllViews()
{
    const SharedData& data = m_model->getData();
    m_home->updateData(data);
    m_data->updateData(data);
    m_result->updateData(data);
}
