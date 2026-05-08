// ============================================================================
// IModel.h — Интерфейс модели (Model) для проекта FormNavigator
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// IModel — абстрактный интерфейс модели данных. Модель хранит SharedData
// и уведомляет презентер об изменениях через сигнал onDataChanged.
//
// ПРИНЦИП MVP: Модель ничего не знает о визуальном представлении.
// Она только хранит данные и рассылает уведомления.
// Презентер решает, какие виды нужно обновить.
//
// ============================================================================

#ifndef IMODEL_H
#define IMODEL_H

#include "signal_impl.h"
#include "NavigatorTypes.h"

class IModel {
public:
    virtual ~IModel() {}

    // Получить текущие данные
    virtual const SharedData& getData() const = 0;

    // Установить новые данные (рассылает onDataChanged)
    virtual void setData(const SharedData& data) = 0;

    // Сигнал: данные изменились (рассылается после setData)
    signals::Signal1<const SharedData&> onDataChanged;
};

// Фабричная функция
IModel* createAppModel();

#endif // IMODEL_H
