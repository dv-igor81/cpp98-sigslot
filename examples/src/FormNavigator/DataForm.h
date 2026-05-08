//---------------------------------------------------------------------------

#ifndef DataFormH
#define DataFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "IView.h"
//---------------------------------------------------------------------------
class TDataForm : public TForm, public IDataView
{
__published:    // IDE-managed Components
        TLabel *m_titleLabel;
        TLabel *m_textLabel;
        TEdit *m_textEdit;
        TLabel *m_countLabel;
        TEdit *m_countEdit;
        TButton *m_applyBtn;
        TButton *m_goHomeBtn;
        TButton *m_goResultBtn;
        void __fastcall onApplyClick(TObject *Sender);
        void __fastcall onGoHomeClick(TObject *Sender);
        void __fastcall onGoResultClick(TObject *Sender);
        void __fastcall onFormClose(TObject *Sender, TCloseAction &Action);
private:        // User declarations
        signals::Signal0 m_onGoHome;
        signals::Signal0 m_onGoResult;
        signals::Signal1<const SharedData&> m_onDataSubmitted;
public:         // User declarations
        __fastcall TDataForm(TComponent* Owner);

        // ---- IView ----
        virtual void showForm();
        virtual void hideForm();
        virtual void updateData(const SharedData& data);

        // ---- IDataView ----
        virtual signals::Signal0& onGoHomeSignal();
        virtual signals::Signal0& onGoResultSignal();
        virtual signals::Signal1<const SharedData&>& onDataSubmittedSignal();
};
//---------------------------------------------------------------------------
extern PACKAGE TDataForm *DataForm;
//---------------------------------------------------------------------------
#endif
