//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "types.h"
#include "compare.h"
#include "midi.h"
#include "quadgt.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormCompare *FormCompare;

static bool changed=FALSE;

//---------------------------------------------------------------------------
__fastcall TFormCompare::TFormCompare(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFormCompare::CompareClick(TObject *Sender)
{
  ModalResult=0;
  if (changed == TRUE)
  {
    QuadGT_UndoCompare();
  }
  QuadGT_Compare(FormCompare->Bank->ItemIndex, StrToInt(FormCompare->PatchNum->Text));
  changed=TRUE;
}
//---------------------------------------------------------------------------

void __fastcall TFormCompare::CancelClick(TObject *Sender)
{
  ModalResult=1;    
  if (changed == TRUE)
  {
    QuadGT_UndoCompare();
  }
  changed=FALSE;
}
//---------------------------------------------------------------------------

