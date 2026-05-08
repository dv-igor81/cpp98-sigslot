// ============================================================================
// Presenter.h — Связывает вид и модель через сигналы/слоты
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// ЧТО ТАКОЕ PRESENTER В MVP?
//
//   Presenter (Презентер) — это «диспетчер» или «регулировщик».
//   Он единственный знает и о View, и о Model, и связывает их.
//
//   Презентер НЕ общается с пользователем напрямую — никакого printf.
//   Он НЕ выполняет вычисления — это дело модели.
//   Он НЕ хранит данные — это дело вида и модели.
//
//   ЕГО ЗАДАЧА — координация:
//     1. Подключиться к сигналам вида и модели
//     2. При получении сигнала от вида — забрать данные, передать в модель
//     3. При получении сигнала от модели — забрать результат, передать в вид
//     4. Проверить корректность данных (валидация — логика презентера)
//
// ============================================================================

#ifndef PRESENTER_H
#define PRESENTER_H

#include "IView.h"
#include "IModel.h"

class Presenter {
public:
    // Конструктор устанавливает сигнально-слотовые соединения
    // и запускает поток взаимодействия (showBanner + promptName).
    Presenter(IView* view, IModel* model);
    ~Presenter();

private:
    IView*  m_view;
    IModel* m_model;

    // --- Обработчики сигналов вида ---
    // Вызываются, когда пользователь завершил ввод
    void handleNameEntered();
    void handleSurnameEntered();

    // --- Обработчики сигналов модели ---
    // Вызываются, когда модель завершила операцию
    void handleGreetingReady();
    void handleModelError(const char* message);

    // --- Стековые слоты для подключения методов презентера ---
    signals::SlotMethodImpl<Presenter>  m_slotNameEntered;
    signals::SlotMethodImpl<Presenter>  m_slotSurnameEntered;
    signals::SlotMethodImpl<Presenter>  m_slotGreetingReady;
    signals::SlotMethodImpl1<Presenter, const char*> m_slotError;

    // Запрет копирования
    Presenter(const Presenter&);
    Presenter& operator=(const Presenter&);
};

#endif // PRESENTER_H
