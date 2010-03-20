//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "types.h"
#include "debug.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormDebug *FormDebug;
//---------------------------------------------------------------------------
__fastcall TFormDebug::TFormDebug(TComponent* Owner)
    : TForm(Owner)
{
  Left= Screen->Width * 2/100;
  Top =Screen->Height * 2/100;
}

__fastcall TFormDebug::Counter(TObject *Sender, AnsiString counter)
{

  LabelCounter->Caption=counter;
  return 0;
}
__fastcall TFormDebug::Log(TObject *Sender, AnsiString text)
{
  MemoLog->Lines->Add(text);
  return 0;
}

__fastcall TFormDebug::LogHex(TObject* Sender, AnsiString msg, UInt8 *buffer, long int length)
{
  long int i;
  AnsiString string;

  string = msg + " : ";
  for (i=0; i<length; i++)
  {
    string += "[0x" + IntToHex(*(buffer+i),2) + "] ";
  }
  MemoLog->Lines->Add(string);

  return 0;


}
//---------------------------------------------------------------------------
void __fastcall TFormDebug::FormShow(TObject *Sender)
{
  Show();
}
//---------------------------------------------------------------------------
