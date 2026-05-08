// ============================================================================
// AppModel.h Ч ћодель данных дл€ проекта FormNavigator
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// AppModel хранит SharedData и рассылает уведомлени€ через сигнал
// onDataChanged при каждом изменении данных.
//
// ѕ–»Ќ÷»ѕ MVP: ћодель Ч единственный источник истины (Single Source of
// Truth). ¬иды не хран€т состо€ние Ч они только отображают то, что
// им передаЄт презентер из модели.
//
// ============================================================================

#ifndef APP_MODEL_H
#define APP_MODEL_H

#include "IModel.h"

class AppModel : public IModel {
public:
    AppModel();

    // IModel: получить текущие данные
    virtual const SharedData& getData() const;

    // IModel: установить новые данные и эмитить onDataChanged
    virtual void setData(const SharedData& data);

private:
    SharedData m_data;   // ’ранимые данные
};

#endif // APP_MODEL_H
