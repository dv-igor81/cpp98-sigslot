//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>      // sscanf
#include <string.h>     // strncpy
#include "DataForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TDataForm *DataForm;
//---------------------------------------------------------------------------
__fastcall TDataForm::TDataForm(TComponent* Owner)
        : TForm(Owner)
{
    OnClose = onFormClose;
}
//---------------------------------------------------------------------------
// IView
//---------------------------------------------------------------------------

void TDataForm::showView()
{
    Show();
}

void TDataForm::hideView()
{
    Hide();
}

signals::Signal0& TDataForm::viewClosedSignal()
{
    return m_viewClosed;
}

//---------------------------------------------------------------------------
// IDataView
//---------------------------------------------------------------------------

void TDataForm::displayData(const SharedData& data)
{
    m_textEdit->Text = data.text;

    // IntToStr -- VCL-функция, совместима с BCB6
    // (snprintf отсутствует в BCB6 runtime)
    m_countEdit->Text = IntToStr(data.count);
}

signals::Signal0& TDataForm::onGoHomeSignal()
{
    return m_onGoHome;
}

signals::Signal0& TDataForm::onGoResultSignal()
{
    return m_onGoResult;
}

signals::Signal1<const SharedData&>& TDataForm::onDataSubmittedSignal()
{
    return m_onDataSubmitted;
}

//---------------------------------------------------------------------------
// Обработчик «Применить»
//---------------------------------------------------------------------------

void __fastcall TDataForm::onApplyClick(TObject * /*Sender*/)
{
    SharedData data;

    strncpy(data.text, m_textEdit->Text.c_str(), MAX_TEXT_LEN - 1);
    data.text[MAX_TEXT_LEN - 1] = '\0';

    int count = 1;
    sscanf(m_countEdit->Text.c_str(), "%d", &count);
    if (count < 1) count = 1;
    data.count = count;

    m_onDataSubmitted.emit_(data);
}

//---------------------------------------------------------------------------
// Навигация
//---------------------------------------------------------------------------

void __fastcall TDataForm::onGoHomeClick(TObject * /*Sender*/)
{
    m_onGoHome.emit_();
}

void __fastcall TDataForm::onGoResultClick(TObject * /*Sender*/)
{
    m_onGoResult.emit_();
}

//---------------------------------------------------------------------------
// Закрытие формы (кнопка X)
// Action = caHide -- форма скрывается, а не уничтожается
// Сигнал viewClosed обрабатывается презентером через навигатор
//---------------------------------------------------------------------------

void __fastcall TDataForm::onFormClose(TObject * /*Sender*/,
                                       TCloseAction &Action)
{
    Action = caHide;
    m_viewClosed.emit_();
}
