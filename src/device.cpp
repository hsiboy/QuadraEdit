//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "types.h"

#include "device.h"
#include "debug.h"
#include "error.h"
#include "main.h"
#include "midi.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormDevice *FormDevice;
//---------------------------------------------------------------------------
__fastcall TFormDevice::TFormDevice(TComponent* Owner)
    : TForm(Owner)
{
  //TBD: Get rid of this field?
  LabelMidiDevError->Caption = "";
}
//---------------------------------------------------------------------------
void __fastcall TFormDevice::ButtonMidiDevOpenClick(TObject *Sender)
{
   unsigned int status;

   ModalResult=1;

   status=Midi_IO_Open(ComboBoxInDevs->Items->Strings[ComboBoxInDevs->ItemIndex]);
   if (status != 0)
   {
     FormError->ShowErrorCode(status,"opening Midi device");
     MainForm->DisableIO();
   }
   else
   {
     FormDebug->Log(Sender,"Midi devices succesfully opened");
     MainForm->EnableIO();
   }
}
//---------------------------------------------------------------------------

void __fastcall TFormDevice::ButtonMidiDevCancelClick(TObject *Sender)
{
  ModalResult=1;
}
//---------------------------------------------------------------------------

