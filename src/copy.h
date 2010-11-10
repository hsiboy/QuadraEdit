//---------------------------------------------------------------------------
#ifndef copyH
#define copyH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Mask.hpp>
//---------------------------------------------------------------------------
class TFormCopy : public TForm
{
__published:	// IDE-managed Components
    TPanel *Panel1;
    TLabel *LabelQuadPatch;
    TMaskEdit *PatchNum1;
    TUpDown *UpDownPatchNum1;
    TLabel *Label1;
    TMaskEdit *PatchNum2;
    TUpDown *UpDownPatchNum2;
    TButton *Swap;
    TButton *Cancel;
    TLabel *Label2;
    TLabel *Label3;
    TRadioGroup *Parameters;
    TRadioGroup *Bank1;
    TRadioGroup *Bank2;
    void __fastcall CancelClick(TObject *Sender);
    void __fastcall SwapClick(TObject *Sender);
    
    void __fastcall PatchNumExit(TObject *Sender);
    void __fastcall PatchNumKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall UpDownPatchNumClick(TObject *Sender,
          TUDBtnType Button);
private:	// User declarations
public:		// User declarations
    __fastcall TFormCopy(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormCopy *FormCopy;
//---------------------------------------------------------------------------
#endif
