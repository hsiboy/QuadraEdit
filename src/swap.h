//---------------------------------------------------------------------------
#ifndef swapH
#define swapH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Mask.hpp>
//---------------------------------------------------------------------------
class TFormSwap : public TForm
{
__published:	// IDE-managed Components
    TPanel *Panel1;
    TRadioGroup *Bank1;
    TLabel *LabelQuadPatch;
    TMaskEdit *PatchNum1;
    TUpDown *UpDownPatchNum1;
    TRadioGroup *Bank2;
    TLabel *Label1;
    TMaskEdit *PatchNum2;
    TUpDown *UpDownPatchNum2;
    TButton *Swap;
    TButton *Cancel;
    void __fastcall CancelClick(TObject *Sender);
    void __fastcall SwapClick(TObject *Sender);
    
    void __fastcall PatchNumExit(TObject *Sender);
    void __fastcall PatchNumKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall UpDownPatchNumClick(TObject *Sender,
          TUDBtnType Button);
private:	// User declarations
public:		// User declarations
    __fastcall TFormSwap(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormSwap *FormSwap;
//---------------------------------------------------------------------------
#endif
