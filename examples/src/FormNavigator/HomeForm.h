//---------------------------------------------------------------------------

#ifndef HomeFormH
#define HomeFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "IView.h"
//---------------------------------------------------------------------------
class THomeForm : public TForm, public IHomeView
{
__published:    // IDE-managed Components
        TLabel *m_titleLabel;
        TLabel *m_infoLabel;
        TButton *m_goDataBtn;
        TButton *m_goResultBtn;
        TButton *m_exitBtn;
        void __fastcall onGoDataClick(TObject *Sender);
        void __fastcall onGoResultClick(TObject *Sender);
        void __fastcall onExitClick(TObject *Sender);
private:        // User declarations
        signals::Signal0 m_onGoData;
        signals::Signal0 m_onGoResult;
        signals::Signal0 m_onExit;
public:         // User declarations
        __fastcall THomeForm(TComponent* Owner);

        // ---- IView ----
        virtual void showForm();
        virtual void hideForm();
        virtual void updateData(const SharedData& data);

        // ---- IHomeView ----
        virtual signals::Signal0& onGoDataSignal();
        virtual signals::Signal0& onGoResultSignal();
        virtual signals::Signal0& onExitSignal();
};
//---------------------------------------------------------------------------
extern PACKAGE THomeForm *HomeForm;
//---------------------------------------------------------------------------
#endif
