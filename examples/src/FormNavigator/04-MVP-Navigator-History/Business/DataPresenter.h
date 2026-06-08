// ============================================================================
// DataPresenter.h -- Презентер формы редактирования данных
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// Управляет редактированием SharedData и навигацией с DataForm.
// При «Применить» данные сохраняются локально (m_data),
// а при навигации передаются через навигатор следующему экрану.
//
// ============================================================================

#ifndef DATAPRESENTER_H
#define DATAPRESENTER_H

#include "IPresenter.h"
#include "IView.h"
#include "INavigator.h"
#include "SharedData.h"

class DataPresenter : public IPresenter {
public:
    DataPresenter(IDataView* view, INavigator* navigator, const SharedData& data);

    // IPresenter
    virtual void initialize();
    virtual void destroy();

private:
    IDataView*  m_view;
    INavigator* m_navigator;
    SharedData  m_data;

    // Обработчики сигналов вида
    void handleGoHome();
    void handleGoResult();
    void handleDataSubmitted(const SharedData& data);
    void handleViewClosed();
};

#endif // DATAPRESENTER_H
