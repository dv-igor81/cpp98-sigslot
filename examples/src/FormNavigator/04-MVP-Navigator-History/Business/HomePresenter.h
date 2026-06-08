// ============================================================================
// HomePresenter.h -- Презентер главной формы
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// Управляет отображением данных и навигацией с HomeForm.
//
// ВАЖНО: Методы-обработчики не должны содержать кода после вызова
//        методов навигатора, т.к. навигатор может удалить данный
//        презентер (delete this). Это безопасно для void-методов,
//        которые просто возвращаются после вызова навигатора.
//
// ============================================================================

#ifndef HOMEPRESENTER_H
#define HOMEPRESENTER_H

#include "IPresenter.h"
#include "IView.h"
#include "INavigator.h"
#include "SharedData.h"

class HomePresenter : public IPresenter {
public:
    HomePresenter(IHomeView* view, INavigator* navigator, const SharedData& data);

    // IPresenter
    virtual void initialize();
    virtual void destroy();

private:
    IHomeView*  m_view;
    INavigator* m_navigator;
    SharedData  m_data;

    // Обработчики сигналов вида
    void handleGoData();
    void handleGoResult();
    void handleExit();
    void handleViewClosed();
};

#endif // HOMEPRESENTER_H
