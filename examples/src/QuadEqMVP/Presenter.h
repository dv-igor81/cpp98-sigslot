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
//
// ПОЧЕМУ ВСЕ МЕТОДЫ ПРИВАТНЫЕ?
//
//   Внешний код (main) создаёт презентер и больше с ним не взаимодействует.
//   Вся работа происходит через сигналы и слоты.
//   Презентер — «чёрный ящик»: подал два указателя — он сам всё сделает.
//
// ============================================================================

#ifndef PRESENTER_H
#define PRESENTER_H

#include "IView.h"
#include "IModel.h"

class Presenter {
public:
    Presenter(IView* view, IModel* model);
    ~Presenter();

private:
    IView*  m_view;
    IModel* m_model;

    // --- Обработчики сигналов ---
    void handleCoefficientsReady();
    void handleResultReady();
    void handleError(const char* message);

    // --- Стековые слоты ---
    signals::SlotMethodImpl<Presenter>  m_slotCoeffs;
    signals::SlotMethodImpl<Presenter>  m_slotResult;
    signals::SlotMethodImpl1<Presenter, const char*> m_slotError;

    Presenter(const Presenter&);
    Presenter& operator=(const Presenter&);
};

#endif // PRESENTER_H
