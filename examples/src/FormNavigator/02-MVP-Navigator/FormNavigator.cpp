// ============================================================================
// FormNavigator.cpp -- Точка входа приложения
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// Навигатор управляет формами и MVP-триадами.
// Выход из приложения происходит через Application->Terminate()
// внутри навигатора, когда закрывается последняя форма (HomeForm).
//
// ============================================================================

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "INavigator.h"
#include "AppNavigator.h"
#include "HomeForm.h"
#include "DataForm.h"
#include "ResultForm.h"
//---------------------------------------------------------------------------
USEFORM("DataForm.cpp", DataForm);
USEFORM("HomeForm.cpp", HomeForm);
USEFORM("ResultForm.cpp", ResultForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        Application->Initialize();
        Application->Title = "FormNavigator -- MVP Navigator Demo";

        // Создаём формы через Application->CreateForm() -- стандарт BCB6.
        // Формы живут всё время работы приложения (show/hide).
        // HomeForm создана ПЕРВОЙ -- становится главной формой приложения.
        Application->CreateForm(__classid(THomeForm), &HomeForm);
        Application->CreateForm(__classid(TDataForm), &DataForm);
        Application->CreateForm(__classid(TResultForm), &ResultForm);

        AppNavigator navigator(HomeForm, DataForm, ResultForm);

        // Стартуем с HomeForm и данными по умолчанию
        SharedData defaultData;
        navigator.navigateToHome(defaultData);

        // Запускаем цикл обработки сообщений.
        // Выход произойдёт по Application->Terminate() внутри навигатора,
        // когда закроется последняя форма.
        Application->Run();

        // Навигатор уничтожается при выходе из области видимости.
        // Деструктор навигатора вызывает destroy() текущего презентера.
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
