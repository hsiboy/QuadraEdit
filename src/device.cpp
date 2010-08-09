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
}
//---------------------------------------------------------------------------
void __fastcall TFormDevice::ButtonMidiDevOpenClick(TObject *Sender)
{
   unsigned int status;

   ModalResult=1;

   //status=Midi_Out_Open(FormDevice->ComboBoxOutDevs->ItemIndex);
   //if (status != 0)
   //{
   //  FormError->ShowError(status,"opening Midi output device");
   //  MainForm->DisableIO();
   //}
   //else
   //{
   //  status=Midi_In_Open(FormDevice->ComboBoxInDevs->ItemIndex);
   //  if (status != 0)
   //  {
   //    FormError->ShowError(status,"opening Midi input device");
   //    MainForm->DisableIO();
   //  }
   //  else
   //  {
   //    FormDebug->Log(Sender,"Midi devices succesfully opened");
   //    MainForm->EnableIO();
   //  }
   //}

   status=Midi_IO_Open(FormDevice->ComboBoxOutDevs->Items->Strings[FormDevice->ComboBoxOutDevs->ItemIndex]);
   if (status != 0)
   {
     FormError->ShowError(status,"opening Midi device");
     MainForm->DisableIO();
   }
}
//---------------------------------------------------------------------------

void __fastcall TFormDevice::ButtonMidiDevCancelClick(TObject *Sender)
{
  ModalResult=1;
}
//---------------------------------------------------------------------------

