//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ResultForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
__fastcall TResultForm::TResultForm(TComponent* Owner)
        : TForm(Owner)
{
    OnClose = onFormClose;
}
//---------------------------------------------------------------------------
// IView
//---------------------------------------------------------------------------

void TResultForm::showView()
{
    Show();
}

void TResultForm::hideView()
{
    Hide();
}

void TResultForm::releaseView()
{
    Release();
}

signals::Signal0& TResultForm::viewClosedSignal()
{
    return m_viewClosed;
}

//---------------------------------------------------------------------------
// IResultView
//---------------------------------------------------------------------------

void TResultForm::displayData(const SharedData& data)
{
    AnsiString result;

    if (data.text[0] == '\0') {
        result = "(Текст не задан — нечего повторять)";
    } else {
        for (int i = 0; i < data.count; i++) {
            if (i > 0) result += "\r\n";
            result += data.text;
        }
    }

    m_resultMemo->Text = result;
}

signals::Signal0& TResultForm::onGoHomeSignal()
{
    return m_onGoHome;
}

signals::Signal0& TResultForm::onGoDataSignal()
{
    return m_onGoData;
}

//---------------------------------------------------------------------------
// Навигация
//---------------------------------------------------------------------------

void __fastcall TResultForm::onGoHomeClick(TObject * /*Sender*/)
{
    m_onGoHome.emit_();
}

void __fastcall TResultForm::onGoDataClick(TObject * /*Sender*/)
{
    m_onGoData.emit_();
}

//---------------------------------------------------------------------------
// Закрытие формы (кнопка X)
// Action = caHide -- форма скрывается, а не уничтожается
// Навигатор решит: скрыть (MainForm) или освободить (releaseView)
// Сигнал viewClosed обрабатывается презентером через навигатор
//---------------------------------------------------------------------------

void __fastcall TResultForm::onFormClose(TObject * /*Sender*/,
                                         TCloseAction &Action)
{
    Action = caHide;
    m_viewClosed.emit_();
}
