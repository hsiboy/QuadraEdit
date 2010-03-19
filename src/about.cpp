//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "about.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormAbout *FormAbout;
//---------------------------------------------------------------------------
__fastcall TFormAbout::TFormAbout(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TFormAbout::ButtonAboutOkClick(TObject *Sender)
{
    PanelAboutExit(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TFormAbout::PanelAboutExit(TObject *Sender)
{
  FormHide(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TFormAbout::FormShow(TObject *Sender)
{
  // Do it
  LabelAbout1->Caption = "QuadraEdit V1.0";
  FormAbout->Show();
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void __fastcall TFormAbout::FormHide(TObject *Sender)
{
  FormAbout->Hide();
}
//---------------------------------------------------------------------------
