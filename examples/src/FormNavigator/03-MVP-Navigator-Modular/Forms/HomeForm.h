//---------------------------------------------------------------------------

#ifndef HomeFormH
#define HomeFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "IView.h"
#include <ComCtrls.hpp>
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
        void __fastcall onFormClose(TObject *Sender, TCloseAction &Action);
private:        // User declarations
        signals::Signal0 m_viewClosed;
        signals::Signal0 m_onGoData;
        signals::Signal0 m_onGoResult;
        signals::Signal0 m_onExit;
public:         // User declarations
        __fastcall THomeForm(TComponent* Owner);

        // ---- IView ----
        virtual void showView();
        virtual void hideView();
        virtual void releaseView();
        virtual signals::Signal0& viewClosedSignal();

        // ---- IHomeView ----
        virtual void displayData(const SharedData& data);
        virtual signals::Signal0& onGoDataSignal();
        virtual signals::Signal0& onGoResultSignal();
        virtual signals::Signal0& onExitSignal();
};
//---------------------------------------------------------------------------
#endif
