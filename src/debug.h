//---------------------------------------------------------------------------
#ifndef debugH
#define debugH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>

#include "types.h"
//---------------------------------------------------------------------------
class TFormDebug : public TForm
{
__published:	// IDE-managed Components
    TPanel *PanelDebug;
    TLabel *LabelCounter;
    TMemo *MemoLog;
    void __fastcall FormShow(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TFormDebug(TComponent* Owner);
    __fastcall Counter(TObject *Sender, AnsiString counter);
    __fastcall Log(TObject* Sender, AnsiString text);
    __fastcall LogHex(TObject* Sender, AnsiString msg, UInt8 *buffer, long int length);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormDebug *FormDebug;
//---------------------------------------------------------------------------
#endif
