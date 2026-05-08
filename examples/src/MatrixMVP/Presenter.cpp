// ============================================================================
// Presenter.cpp — Реализация презентера
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// ПОЛНЫЙ ЖИЗНЕННЫЙ ЦИКЛ ПРИЛОЖЕНИЯ:
//
//   main() создаёт IView* и IModel*, передаёт их в Presenter.
//   Конструктор презентера делает 4 вещи:
//     1. Создаёт слоты и подключает их к сигналам вида и модели
//     2. Показывает баннер (через вид)
//     3. Запрашивает размерности (через вид) — это старт потока
//     4. Далее всё работает на сигналах и слотах:
//
//   +-----------------------------------------------------------------+
//   |  View                         Presenter           Model       |
//   |                                                               |
//   |  promptDimensions() --сигнал--> handleDimensionsEntered()     |
//   |                               +- валидация                    |
//   |                               +- showInvalidDimensions() -->  |
//   |                               +- showDimensionMismatch() -->  |
//   |                               +- setDimensions() ------------->|
//   |                               +- promptMatrixA() -->          |
//   |                                                               |
//   |  promptMatrixA() --сигнал--> handleMatrixAReady()             |
//   |                               +- loadMatrixA() -------------->|
//   |                               +- promptMatrixB() -->          |
//   |                                                               |
//   |  promptMatrixB() --сигнал--> handleMatrixBReady()             |
//   |                               +- loadMatrixB() -------------->|
//   |                               +- multiply() ----------------->|
//   |                                                               |
//   |                              <--сигнал-- onResultReady         |
//   |  showResult() <-- handleResultReady()                         |
//   |                                                               |
//   |                              <--сигнал-- onOverflowDetected    |
//   |  showOverflowWarning() <-- handleOverflowDetected()           |
//   |                                                               |
//   |                              <--сигнал-- onError               |
//   |  showError() <-- handleModelError()                           |
//   +-----------------------------------------------------------------+
//
//   ПРАВИЛО: Презентер НИКОГДА не общается с консолью напрямую.
//   Весь вывод — только через методы вида.
//   Весь ввод — только через геттеры вида.
//   Все вычисления — только через методы модели.
//
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "Presenter.h"
#include "MatrixTypes.h"

using namespace signals;

// ============================================================================
// Конструктор: установка соединений и запуск потока
// ============================================================================

Presenter::Presenter(IView* view, IModel* model)
    : m_view(view), m_model(model),
      m_slotDimensions(this, &Presenter::handleDimensionsEntered),
      m_slotMatrixA(this, &Presenter::handleMatrixAReady),
      m_slotMatrixB(this, &Presenter::handleMatrixBReady),
      m_slotResult(this, &Presenter::handleResultReady),
      m_slotOverflow(this, &Presenter::handleOverflowDetected),
      m_slotError(this, &Presenter::handleModelError)
{
    // ---------------------------------------------------------------
    // ШАГ 1: Подключаем слоты презентера к сигналам вида.
    //
    // Когда вид излучает сигнал — вызывается наш обработчик.
    // Это «проводка», которая соединяет пользовательские события
    // с логикой презентера.
    // ---------------------------------------------------------------
    m_view->onDimensionsEntered += m_slotDimensions;
    m_view->onMatrixAReady += m_slotMatrixA;
    m_view->onMatrixBReady += m_slotMatrixB;

    // ---------------------------------------------------------------
    // ШАГ 2: Подключаем слоты презентера к сигналам модели.
    //
    // Когда модель завершает операцию — вызывается наш обработчик.
    // ---------------------------------------------------------------
    m_model->onResultReady += m_slotResult;
    m_model->onOverflowDetected += m_slotOverflow;
    m_model->onError += m_slotError;

    // ---------------------------------------------------------------
    // ШАГ 3: Запускаем поток взаимодействия.
    //
    // Сначала показываем баннер, затем запрашиваем размерности.
    // Дальше всё пойдёт по сигнально-слотовой цепочке.
    // ---------------------------------------------------------------
    m_view->showBanner();
    m_view->promptDimensions();
}

Presenter::~Presenter() {}

// ============================================================================
// Обработчики сигналов вида
// ============================================================================

// Пользователь ввёл размерности матриц.
// Проверяем корректность и передаём в модель.
void Presenter::handleDimensionsEntered(int rA, int cA, int rB, int cB)
{
    // Валидация — это логика презентера.
    // Модель не должна проверять «разумность» размерностей —
    // это не её дело. Модель — вычислитель, а не валидатор.

    // Проверка диапазона
    if (rA <= 0 || cA <= 0 || rB <= 0 || cB <= 0 ||
        rA > MAX_DIM || cA > MAX_DIM || rB > MAX_DIM || cB > MAX_DIM)
    {
        m_view->showInvalidDimensions();
        return;
    }

    // Проверка совместимости: cA должно быть равно rB
    if (cA != rB)
    {
        m_view->showDimensionMismatch(cA, rB);
        return;
    }

    // Размерности корректны — передаём в модель
    m_model->setDimensions(rA, cA, rB, cB);

    // Запрашиваем элементы матрицы A
    m_view->promptMatrixA();
}

// Пользователь ввёл матрицу A.
// Забираем данные из вида и передаём в модель.
void Presenter::handleMatrixAReady()
{
    m_model->loadMatrixA(m_view->getMatrixAData(),
                         m_view->getRowsA(), m_view->getColsA());

    // Теперь запрашиваем матрицу B
    m_view->promptMatrixB();
}

// Пользователь ввёл матрицу B.
// Забираем данные из вида и запускаем вычисление.
void Presenter::handleMatrixBReady()
{
    m_model->loadMatrixB(m_view->getMatrixBData(),
                         m_view->getRowsB(), m_view->getColsB());

    // Запускаем умножение — модель излучит onResultReady когда закончит
    m_model->multiply();
}

// ============================================================================
// Обработчики сигналов модели
// ============================================================================

// Модель завершила умножение — передаём результат в вид.
void Presenter::handleResultReady()
{
    m_view->showResult(m_model->getResultData(),
                       m_model->getResultRows(),
                       m_model->getResultCols());
}

// Модель обнаружила переполнение — показываем предупреждение.
void Presenter::handleOverflowDetected()
{
    m_view->showOverflowWarning();
}

// Модель сообщила об ошибке — показываем через вид.
void Presenter::handleModelError(const char* message)
{
    m_view->showError(message);
}
