//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "error.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormError *FormError;
//---------------------------------------------------------------------------
__fastcall TFormError::TFormError(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFormError::FormShow(TObject *Sender)
{
 Show();
}
//---------------------------------------------------------------------------
void __fastcall TFormError::FormHide(TObject *Sender)
{
 Hide();
}
//---------------------------------------------------------------------------
void __fastcall TFormError::ButtonOkClick(TObject *Sender)
{
  FormHide(Sender);
}
//---------------------------------------------------------------------------
