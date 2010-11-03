//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "debug.h"
#include "swap.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormSwap *FormSwap;
//---------------------------------------------------------------------------
__fastcall TFormSwap::TFormSwap(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFormSwap::CancelClick(TObject *Sender)
{
  ModalResult=0;
}
//---------------------------------------------------------------------------
void __fastcall TFormSwap::SwapClick(TObject *Sender)
{
  ModalResult=1;
}
//---------------------------------------------------------------------------

void __fastcall TFormSwap::PatchNumExit(TObject *Sender)
{
  FormDebug->Log(Sender,"Swap patch [" + PatchNum1->Text + "] selected");
}

void __fastcall TFormSwap::PatchNumKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if (Key == 13)
  {
    PatchNumExit(Sender);
  }
}

//---------------------------------------------------------------------------
void __fastcall TFormSwap::UpDownPatchNumClick(TObject *Sender,
      TUDBtnType Button)
{
  PatchNumExit(Sender);
}
//---------------------------------------------------------------------------
