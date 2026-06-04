// ============================================================================
// ResultPresenter.h -- Презентер формы результатов
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// Управляет отображением результата обработки данных и навигацией
// с ResultForm. Бизнес-логика (повтор текста N раз) инкапсулирована здесь.
//
// ============================================================================

#ifndef RESULTPRESENTER_H
#define RESULTPRESENTER_H

#include "IPresenter.h"
#include "IView.h"
#include "INavigator.h"
#include "SharedData.h"

class ResultPresenter : public IPresenter {
public:
    ResultPresenter(IResultView* view, INavigator* navigator,
                    const SharedData& data);

    // IPresenter
    virtual void initialize();
    virtual void destroy();

private:
    IResultView* m_view;
    INavigator*  m_navigator;
    SharedData   m_data;

    // Обработчики сигналов вида
    void handleGoHome();
    void handleGoData();
    void handleViewClosed();
};

#endif // RESULTPRESENTER_H
