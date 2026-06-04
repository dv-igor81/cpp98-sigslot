//---------------------------------------------------------------------------

#ifndef PresenterH
#define PresenterH
//---------------------------------------------------------------------------
#include "IView.h"
#include "IModel.h"

class Presenter {
public:
    Presenter(IHomeView* home, IDataView* data, IResultView* result, IModel* model);

private:
    IHomeView*   m_home;
    IDataView*   m_data;
    IResultView* m_result;
    IModel*      m_model;
    IView*       m_current;

    // --- Обработчики навигации ---
    void handleGoHome();
    void handleGoData();
    void handleGoResult();

    // --- Обработчик данных ---
    void handleDataSubmitted(const SharedData& data);

    // --- Обработчик изменения данных в модели ---
    void handleDataChanged(const SharedData& data);

    // --- Обработчик выхода ---
    void handleExit();

    // --- Переключение формы ---
    void switchTo(IView* target);

    // --- Обновление всех видов ---
    void updateAllViews();
};

#endif
