//---------------------------------------------------------------------------

#ifndef ResultFormH
#define ResultFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "IView.h"
//---------------------------------------------------------------------------
class TResultForm : public TForm, public IResultView
{
__published:    // IDE-managed Components
        TLabel *m_titleLabel;
        TMemo *m_resultMemo;
        TButton *m_goHomeBtn;
        TButton *m_goDataBtn;
        void __fastcall onGoHomeClick(TObject *Sender);
        void __fastcall onGoDataClick(TObject *Sender);
        void __fastcall onFormClose(TObject *Sender, TCloseAction &Action);
private:        // User declarations
        signals::Signal0 m_onGoHome;
        signals::Signal0 m_onGoData;
public:         // User declarations
        __fastcall TResultForm(TComponent* Owner);

        // ---- IView ----
        virtual void showForm();
        virtual void hideForm();
        virtual void updateData(const SharedData& data);

        // ---- IResultView ----
        virtual signals::Signal0& onGoHomeSignal();
        virtual signals::Signal0& onGoDataSignal();
};
//---------------------------------------------------------------------------
extern PACKAGE TResultForm *ResultForm;
//---------------------------------------------------------------------------
#endif
