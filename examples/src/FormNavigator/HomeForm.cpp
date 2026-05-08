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
}
//---------------------------------------------------------------------------
// IView
//---------------------------------------------------------------------------

void THomeForm::showForm()
{
    Show();
}

void THomeForm::hideForm()
{
    Hide();
}

void THomeForm::updateData(const SharedData& data)
{
    AnsiString info;
    info.cat_sprintf("Текст: \"%s\"\r\n\r\n", data.text);
    info.cat_sprintf("Счётчик: %d", data.count);
    m_infoLabel->Caption = info;
}

//---------------------------------------------------------------------------
// IHomeView: геттеры сигналов
//---------------------------------------------------------------------------

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
