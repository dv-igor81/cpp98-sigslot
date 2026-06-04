//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "HomeForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
THomeForm *HomeForm;
//---------------------------------------------------------------------------
__fastcall THomeForm::THomeForm(TComponent* Owner)
        : TForm(Owner)
{
    OnClose = onFormClose;
}
//---------------------------------------------------------------------------
// IView
//---------------------------------------------------------------------------

void THomeForm::showView()
{
    Show();
}

void THomeForm::hideView()
{
    Hide();
}

signals::Signal0& THomeForm::viewClosedSignal()
{
    return m_viewClosed;
}

//---------------------------------------------------------------------------
// IHomeView
//---------------------------------------------------------------------------

void THomeForm::displayData(const SharedData& data)
{
    AnsiString info;
    info.cat_sprintf("Текст: \"%s\"\r\n\r\n", data.text);
    info.cat_sprintf("Счётчик: %d", data.count);
    m_infoLabel->Caption = info;
}

signals::Signal0& THomeForm::onGoDataSignal()
{
    return m_onGoData;
}

signals::Signal0& THomeForm::onGoResultSignal()
{
    return m_onGoResult;
}

signals::Signal0& THomeForm::onExitSignal()
{
    return m_onExit;
}

//---------------------------------------------------------------------------
// Обработчики кнопок
//---------------------------------------------------------------------------

void __fastcall THomeForm::onGoDataClick(TObject * /*Sender*/)
{
    m_onGoData.emit_();
}

void __fastcall THomeForm::onGoResultClick(TObject * /*Sender*/)
{
    m_onGoResult.emit_();
}

void __fastcall THomeForm::onExitClick(TObject * /*Sender*/)
{
    m_onExit.emit_();
}

//---------------------------------------------------------------------------
// Закрытие формы (кнопка X)
// Action = caHide -- форма скрывается, а не уничтожается
// Сигнал viewClosed обрабатывается презентером через навигатор
//---------------------------------------------------------------------------

void __fastcall THomeForm::onFormClose(TObject * /*Sender*/,
                                       TCloseAction &Action)
{
    Action = caHide;
    m_viewClosed.emit_();
}
