// ============================================================================
// Presenter.cpp — Реализация презентера
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// ПОЛНЫЙ ЖИЗНЕННЫЙ ЦИКЛ ПРИЛОЖЕНИЯ:
//
//   main() создаёт IView* и IModel*, передаёт их в Presenter.
//   Конструктор презентера:
//     1. Подключает слоты к сигналам вида и модели
//     2. Показывает баннер (через вид)
//     3. Запрашивает коэффициенты (через вид) — старт потока
//
//   +------------------------------------------------------------------+
//   |  View                      Presenter            Model          |
//   |                                                                |
//   |  promptCoefficients() --сигнал--> handleCoefficientsReady()    |
//   |                                 +- view->showCoefficients()     |
//   |                                 +- model->setCoefficients() --> |
//   |                                 +- model->solve() ------------->|
//   |                                                                |
//   |                             <--сигнал-- onResultReady          |
//   |  showLinearResult() <-- handleResultReady()                   |
//   |  showTwoRoots()     <--   (в зависимости от getSolveResult)   |
//   |  showOneRoot()      <--                                        |
//   |  showNoRealRoots()  <--                                        |
//   |  showInfinite...()  <--                                        |
//   |  showNoSolutions()  <--                                        |
//   |                                                                |
//   |                             <--сигнал-- onError                |
//   |  showError() <-- handleError()                                 |
//   +------------------------------------------------------------------+
//
//   ПРАВИЛО: Презентер НИКОГДА не общается с консолью напрямую.
//
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "Presenter.h"

using namespace signals;

// ============================================================================
// Конструктор: установка соединений и запуск потока
// ============================================================================

Presenter::Presenter(IView* view, IModel* model)
    : m_view(view), m_model(model),
      m_slotCoeffs(this, &Presenter::handleCoefficientsReady),
      m_slotResult(this, &Presenter::handleResultReady),
      m_slotError(this, &Presenter::handleError)
{
    // Подключаем слоты к сигналам вида
    m_view->onCoefficientsReady += m_slotCoeffs;

    // Подключаем слоты к сигналам модели
    m_model->onResultReady += m_slotResult;
    m_model->onError += m_slotError;

    // Запускаем поток взаимодействия
    m_view->showBanner();
    m_view->promptCoefficients();
}

Presenter::~Presenter() {}

// ============================================================================
// Обработчики сигналов
// ============================================================================

// Пользователь ввёл коэффициенты.
// Забираем данные из вида, показываем эхо и запускаем решение.
void Presenter::handleCoefficientsReady()
{
    type a = m_view->getA();
    type b = m_view->getB();
    type c = m_view->getC();

    // Эхо ввода — показываем, что получили
    m_view->showCoefficients(a, b, c);

    // Передаём коэффициенты в модель и запускаем решение
    m_model->setCoefficients(a, b, c);
    m_model->solve();
}

// Модель завершила решение — определяем тип результата и показываем.
void Presenter::handleResultReady()
{
    switch (m_model->getSolveResult())
    {
    case SOLVE_LINEAR:
        m_view->showLinearResult(m_model->getRoot1(),
                                 m_model->getVerification1());
        break;

    case SOLVE_TWO_ROOTS:
        m_view->showTwoRoots(m_model->getRoot1(), m_model->getRoot2(),
                             m_model->getVerification1(),
                             m_model->getVerification2());
        break;

    case SOLVE_ONE_ROOT:
        m_view->showOneRoot(m_model->getRoot1(),
                            m_model->getVerification1());
        break;

    case SOLVE_NO_REAL_ROOTS:
        m_view->showNoRealRoots();
        break;

    case SOLVE_INFINITE:
        m_view->showInfiniteSolutions();
        break;

    case SOLVE_NO_SOLUTION:
        m_view->showNoSolutions();
        break;
    }
}

// Модель сообщила об ошибке — показываем через вид.
void Presenter::handleError(const char* message)
{
    m_view->showError(message);
}
