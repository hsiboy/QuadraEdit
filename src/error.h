//---------------------------------------------------------------------------
#ifndef errorH
#define errorH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TFormError : public TForm
{
__published:	// IDE-managed Components
    TLabel *LabelError;
    TButton *ButtonOk;
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormHide(TObject *Sender);
    void __fastcall ButtonOkClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TFormError(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormError *FormError;
//---------------------------------------------------------------------------
#endif
