//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "types.h"
#include "error.h"
#include "debug.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormError *FormError;
//---------------------------------------------------------------------------
__fastcall TFormError::TFormError(TComponent* Owner)
    : TForm(Owner)
{
   FormDebug->Log(NULL,"FormError create");
}
//---------------------------------------------------------------------------
void __fastcall TFormError::FormHide(TObject *Sender)
{
 ModalResult=1;
}
//---------------------------------------------------------------------------
void __fastcall TFormError::ButtonOkClick(TObject *Sender)
{
  FormHide(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TFormError::ShowError(UInt32 code, AnsiString string)
{
  LabelError->Caption="Error ["+AnsiString(code)+"] "+string;
  ShowModal();
}
