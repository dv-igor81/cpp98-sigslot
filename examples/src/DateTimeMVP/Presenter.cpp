// ============================================================================
// Presenter.cpp — Реализация презентера
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// ПОЛНЫЙ ЖИЗНЕННЫЙ ЦИКЛ ПРИЛОЖЕНИЯ:
//
//   main() создаёт IView* и IModel*, передаёт их в Presenter.
//   Конструктор презентера делает 3 вещи:
//     1. Создаёт слоты и подключает их к сигналам вида и модели
//     2. Показывает баннер (через вид)
//     3. Запрашивает часовой пояс (через вид) — старт потока
//     4. Далее всё работает на сигналах и слотах:
//
//   +--------------------------------------------------------------------+
//   |  View                         Presenter             Model          |
//   |                                                                    |
//   |  promptTimezone() --сигнал--> handleTimezoneEntered()              |
//   |                                +- забирает timezone из вида        |
//   |                                +- валидация (диапазон -12..+14)    |
//   |                                +- model->setTimezoneOffset() ----> |
//   |                                +- promptMode() -->                 |
//   |                                                                    |
//   |  promptMode() --сигнал--> handleModeSelected()                     |
//   |                         +- забирает mode из вида                   |
//   |                         +- валидация (1 или 2)                     |
//   |                         +- model->setConversionMode() -----------> |
//   |                         +- promptEpochInput() или                  |
//   |                            promptDateTimeInput() -->               |
//   |                                                                    |
//   |  promptEpochInput() --сигнал--> handleInputEntered()               |
//   |  promptDateTimeInput()          +- забирает данные из вида         |
//   |                                +- model->setEpoch/DateTime() ----->|
//   |                                +- model->convert() --------------->|
//   |                                                                    |
//   |                             <--сигнал-- onResultReady              |
//   |  showResult() <-- handleResultReady()                              |
//   |                         +- забирает результат из модели            |
//   |                                                                    |
//   |                             <--сигнал-- onError                    |
//   |  showError() <-- handleModelError()                                |
//   +--------------------------------------------------------------------+
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
#include "DateTimeTypes.h"

using namespace signals;

// ============================================================================
// Конструктор: установка соединений и запуск потока
// ============================================================================

Presenter::Presenter(IView* view, IModel* model)
    : m_view(view), m_model(model),
      m_slotTimezoneEntered(this, &Presenter::handleTimezoneEntered),
      m_slotModeSelected(this, &Presenter::handleModeSelected),
      m_slotInputEntered(this, &Presenter::handleInputEntered),
      m_slotResultReady(this, &Presenter::handleResultReady),
      m_slotError(this, &Presenter::handleModelError)
{
    // ---------------------------------------------------------------
    // ШАГ 1: Подключаем слоты презентера к сигналам вида.
    // ---------------------------------------------------------------
    m_view->onTimezoneEntered += m_slotTimezoneEntered;
    m_view->onModeSelected   += m_slotModeSelected;
    m_view->onInputEntered   += m_slotInputEntered;

    // ---------------------------------------------------------------
    // ШАГ 2: Подключаем слоты презентера к сигналам модели.
    // ---------------------------------------------------------------
    m_model->onResultReady += m_slotResultReady;
    m_model->onError       += m_slotError;

    // ---------------------------------------------------------------
    // ШАГ 3: Запускаем поток взаимодействия.
    // ---------------------------------------------------------------
    m_view->showBanner();
    m_view->promptTimezone();
}

Presenter::~Presenter() {}

// ============================================================================
// Обработчики сигналов вида
// ============================================================================

// Пользователь ввёл часовой пояс.
// Забираем из вида, проверяем, передаём в модель.
void Presenter::handleTimezoneEntered()
{
    int tz = m_view->getTimezoneOffset();

    // Валидация — логика презентера.
    // Модель не должна проверять «разумность» часового пояса.
    if (tz < -12 || tz > 14)
    {
        m_view->showError("Часовой пояс должен быть в диапазоне от -12 до +14.");
        m_view->promptTimezone();
        return;
    }

    // Часовой пояс корректен — передаём в модель
    m_model->setTimezoneOffset(tz);

    // Запрашиваем направление преобразования
    m_view->promptMode();
}

// Пользователь выбрал направление преобразования.
// Забираем из вида, проверяем, передаём в модель.
void Presenter::handleModeSelected()
{
    int mode = m_view->getSelectedMode();

    // Валидация: допустимы только режимы 1 и 2
    if (mode != MODE_EPOCH_TO_DATETIME && mode != MODE_DATETIME_TO_EPOCH)
    {
        m_view->showError("Пожалуйста, выберите 1 или 2.");
        m_view->promptMode();
        return;
    }

    // Режим корректен — передаём в модель
    m_model->setConversionMode(mode);

    // Запрашиваем входные данные в зависимости от режима
    if (mode == MODE_EPOCH_TO_DATETIME)
    {
        m_view->promptEpochInput();
    }
    else
    {
        m_view->promptDateTimeInput();
    }
}

// Пользователь ввёл значение для преобразования.
// Забираем из вида, передаём в модель, запускаем конвертацию.
void Presenter::handleInputEntered()
{
    int mode = m_view->getSelectedMode();

    if (mode == MODE_EPOCH_TO_DATETIME)
    {
        unsigned int epoch = m_view->getEpochInput();
        m_model->setEpochValue(epoch);
    }
    else if (mode == MODE_DATETIME_TO_EPOCH)
    {
        const TDateTime& dt = m_view->getDateTimeInput();

        // Базовая валидация полей даты/времени
        if (dt.TimeDMonth < 1 || dt.TimeDMonth > 31 ||
            dt.TimeMonth < 1 || dt.TimeMonth > 12 ||
            dt.TimeHor > 23 || dt.TimeMin > 59 || dt.TimeSec > 59)
        {
            m_view->showError("Некорректная дата или время. Проверьте введённые значения.");
            m_view->promptDateTimeInput();
            return;
        }

        m_model->setDateTime(dt);
    }

    // Запускаем конвертацию — модель излучит onResultReady когда закончит
    m_model->convert();
}

// ============================================================================
// Обработчики сигналов модели
// ============================================================================

// Модель завершила преобразование — передаём результат в вид.
void Presenter::handleResultReady()
{
    m_view->showResult(m_model->getResult());
}

// Модель сообщила об ошибке — показываем через вид.
void Presenter::handleModelError(const char* message)
{
    m_view->showError(message);
}
