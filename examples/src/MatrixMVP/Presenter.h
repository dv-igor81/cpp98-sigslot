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
//   Он НЕ хранит данные матриц — это дело вида и модели.
//
//   ЕГО ЗАДАЧА — координация:
//     1. Подключиться к сигналам вида и модели
//     2. При получении сигнала от вида — забрать данные, передать в модель
//     3. При получении сигнала от модели — забрать результат, передать в вид
//     4. Проверить корректность данных (валидация — это логика презентера)
//
// ПОЧЕМУ ВСЕ МЕТОДЫ ПРИВАТНЫЕ?
//
//   Внешний код (main) создаёт презентер и больше с ним не взаимодействует.
//   Вся работа происходит внутри: сигналы -> слоты -> вызовы методов вида/модели.
//   Презентер — это «чёрный ящик»: подал на вход два указателя — и он
//   сам запускает весь процесс в конструкторе.
//
// КАК РАБОТАЮТ СЛОТЫ?
//
//   Слот — это приёмник сигнала. У каждого обработчика есть свой слот:
//     m_slotDimensions  <- подключён к view->onDimensionsEntered
//     m_slotMatrixA     <- подключён к view->onMatrixAReady
//     m_slotMatrixB     <- подключён к view->onMatrixBReady
//     m_slotResult      <- подключён к model->onResultReady
//     m_slotOverflow    <- подключён к model->onOverflowDetected
//     m_slotError       <- подключён к model->onError
//
//   Слоты — стековые объекты-члены презентера. Они живут пока жив
//   презентер и автоматически отключаются при разрушении.
//
// ============================================================================

#ifndef PRESENTER_H
#define PRESENTER_H

#include "IView.h"
#include "IModel.h"

class Presenter {
public:
    // Конструктор устанавливает сигнально-слотовые соединения
    // и запускает поток взаимодействия (showBanner + promptDimensions).
    Presenter(IView* view, IModel* model);
    ~Presenter();

private:
    IView*  m_view;
    IModel* m_model;

    // --- Обработчики сигналов вида ---
    // Вызываются, когда пользователь завершил ввод
    void handleDimensionsEntered(int rA, int cA, int rB, int cB);
    void handleMatrixAReady();
    void handleMatrixBReady();

    // --- Обработчики сигналов модели ---
    // Вызываются, когда модель завершила операцию
    void handleResultReady();
    void handleOverflowDetected();
    void handleModelError(const char* message);

    // --- Стековые слоты для подключения методов презентера ---
    // Каждый слот связывает сигнал с методом-обработчиком
    signals::SlotMethodImpl4<Presenter, int, int, int, int> m_slotDimensions;
    signals::SlotMethodImpl<Presenter>  m_slotMatrixA;
    signals::SlotMethodImpl<Presenter>  m_slotMatrixB;
    signals::SlotMethodImpl<Presenter>  m_slotResult;
    signals::SlotMethodImpl<Presenter>  m_slotOverflow;
    signals::SlotMethodImpl1<Presenter, const char*> m_slotError;

    // Запрет копирования
    Presenter(const Presenter&);
    Presenter& operator=(const Presenter&);
};

#endif // PRESENTER_H
