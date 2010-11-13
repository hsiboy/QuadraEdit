//---------------------------------------------------------------------------
#ifndef compareH
#define compareH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Mask.hpp>
//---------------------------------------------------------------------------
class TFormCompare : public TForm
{
__published:	// IDE-managed Components
    TPanel *Panel1;
    TLabel *Label1;
    TLabel *Label3;
    TRadioGroup *Bank;
    TMaskEdit *PatchNum;
    TUpDown *UpDownPatchNum;
    TButton *Compare;
    TButton *Cancel;
    void __fastcall CompareClick(TObject *Sender);
    void __fastcall CancelClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TFormCompare(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormCompare *FormCompare;
//---------------------------------------------------------------------------
#endif
