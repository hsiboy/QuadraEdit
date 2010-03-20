//---------------------------------------------------------------------------
#ifndef aboutH
#define aboutH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TFormAbout : public TForm
{
__published:	// IDE-managed Components
    TPanel *PanelAbout;
    TLabel *LabelAbout1;
    TLabel *LabelAbout2;
    TButton *ButtonAboutOk;
    TImage *Image1;
    void __fastcall ButtonAboutOkClick(TObject *Sender);
    void __fastcall PanelAboutExit(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormHide(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TFormAbout(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormAbout *FormAbout;
//---------------------------------------------------------------------------
#endif
