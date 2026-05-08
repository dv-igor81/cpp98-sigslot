//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "IView.h"
#include "IModel.h"
#include "Presenter.h"
#include "HomeForm.h"
#include "DataForm.h"
#include "ResultForm.h"
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        Application->Initialize();
        Application->Title = "FormNavigator Ч MVP Demo";

        // —оздаЄм формы через Application->CreateForm() Ч стандарт BCB6.
        // HomeForm ѕ≈–¬ќ… Ч становитс€ главной формой приложени€.
        Application->CreateForm(__classid(THomeForm), &HomeForm);
        Application->CreateForm(__classid(TDataForm), &DataForm);
        Application->CreateForm(__classid(TResultForm), &ResultForm);

        // —оздаЄм модель
        IModel* model = createAppModel();

        // ѕолучаем специфичные интерфейсы из уже созданных форм
        IHomeView*   home   = static_cast<IHomeView*>(HomeForm);
        IDataView*   data   = static_cast<IDataView*>(DataForm);
        IResultView* result = static_cast<IResultView*>(ResultForm);

        // —оздаЄм презентер Ч он подключает сигналы и управл€ет навигацией
        Presenter presenter(home, data, result, model);

        // «апускаем цикл обработки сообщений
        Application->Run();

        // ќчистка
        delete model;
        // ‘ормы удал€ютс€ VCL автоматически (Owner = Application)
    }
    catch (Exception& exception)
    {
        Application->ShowException(&exception);
    }
    catch (...)
    {
        try
        {
            throw Exception("");
        }
        catch (Exception& exception)
        {
            Application->ShowException(&exception);
        }
    }

    return 0;
}
