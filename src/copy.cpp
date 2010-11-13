//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "debug.h"
#include "copy.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormCopy *FormCopy;
//---------------------------------------------------------------------------
__fastcall TFormCopy::TFormCopy(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFormCopy::CancelClick(TObject *Sender)
{
  FormDebug->Log(Sender,"Copy Cancelled");
  ModalResult=0;
}
//---------------------------------------------------------------------------
void __fastcall TFormCopy::CopyClick(TObject *Sender)
{
  ModalResult=1;
}
//---------------------------------------------------------------------------

void __fastcall TFormCopy::PatchNumExit(TObject *Sender)
{
  FormDebug->Log(Sender,"Copy patch settings [" + PatchNum1->Text + "] selected");
}

void __fastcall TFormCopy::PatchNumKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if (Key == 13)
  {
    PatchNumExit(Sender);
  }
}

//---------------------------------------------------------------------------
void __fastcall TFormCopy::UpDownPatchNumClick(TObject *Sender,
      TUDBtnType Button)
{
  PatchNumExit(Sender);
}
//---------------------------------------------------------------------------
