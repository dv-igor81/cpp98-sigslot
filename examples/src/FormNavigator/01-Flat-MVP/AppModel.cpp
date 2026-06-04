// ============================================================================
// AppModel.cpp — Реализация модели данных
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "AppModel.h"

AppModel::AppModel()
{
    m_data.clear();
}

const SharedData& AppModel::getData() const
{
    return m_data;
}

void AppModel::setData(const SharedData& data)
{
    m_data = data;
    onDataChanged.emit_(m_data);   // Уведомляем подписчиков (презентер)
}


// ============================================================================
// Фабричная функция
// ============================================================================

IModel* createAppModel()
{
    return new AppModel();
}
