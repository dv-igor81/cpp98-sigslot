// ============================================================================
// AppMain.cpp -- “очка входа приложени€
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// Ќавигатор управл€ет формами и MVP-триадами (create-on-demand).
// ‘ормы создаютс€ навигатором при навигации, а не заранее.
// ѕерва€ форма, в которую происходит навигаци€, становитс€ MainForm.
// Ёто может быть люба€ из трЄх форм (Home, Data, Result).
// ¬ыход из приложени€ происходит через Application->Terminate()
// внутри навигатора, когда закрываетс€ главна€ форма.
//
// ============================================================================

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "AppNavigator.h"

//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        Application->Initialize();
        Application->Title = "Navigator -- MVP Demo";

        // Ќавигатор создаЄт формы по требованию (create-on-demand).
        // ѕерва€ форма, в которую происходит навигаци€, становитс€
        // Application->MainForm (через CreateForm).
        // ƒл€ выбора стартовой формы достаточно изменить вызов ниже:
        //   navigator.navigateToHome(defaultData);   -- HomeForm (по умолчанию)
        //   navigator.navigateToData(defaultData);   -- DataForm
        //   navigator.navigateToResult(defaultData); -- ResultForm

        AppNavigator navigator;

        // —тартуем с HomeForm и данными по умолчанию
        SharedData defaultData;
        navigator.navigateToHome(defaultData);
        //navigator.navigateToData(defaultData);
        //navigator.navigateToResult(defaultData);

        // «апускаем цикл обработки сообщений.
        // ¬ыход произойдЄт по Application->Terminate() внутри навигатора,
        // когда закроетс€ главна€ форма.
        Application->Run();

        // Ќавигатор уничтожаетс€ при выходе из области видимости.
        // ƒеструктор навигатора вызывает destroy() текущего презентера.
        // ‘ормы принадлежат Application -- VCL удалит их сама.
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
