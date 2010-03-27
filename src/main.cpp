//===========================================================================
//
// See:
//
// Basis of this source code:
//   Mini MIDI, Nick Gessler, 1 March 2005
//   http://www.duke.edu/web/isis/gessler/borland/sound.htm
// MIDI API:
//   http://msdn.microsoft.com/en-us/library/dd742875%28VS.85%29.aspx
//===========================================================================

#include <vcl.h>
#pragma hdrstop

#include "types.h"

#include "main.h"
#include "about.h"
#include "midi.h"
#include "error.h"
#include "debug.h"
#include "device.h"
#include "quadgt.h"
#include <stdlib.h>

#define QUAD_PATCH_MIN (0)
#define QUAD_PATCH_MAX (99)

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
   : TForm(Owner)
{
  Left=Screen->Width/2- Width/2;
  Top =Screen->Height/2 - Height/2;
}

//---------------------------------------------------------------------------
// Name       : TMainForm::Init
// Desc.      : Initialises the GUI main form
// Params.    : NONE.
// Returns    : NONE.
//---------------------------------------------------------------------------
void __fastcall TMainForm::Init(void)
{
  FormDebug->Log(MainForm, "Starting...");

  // Set up Quadraverb specific values
  UpDownQuadPatch->Min=0;
  UpDownQuadPatch->Max=QUAD_NUM_PATCH-1;
  EditQuadPatchName->Text="---";

  // Get list of Midi devices
  Midi_Get_Dev_Lists(FormDevice->ComboBoxInDevs,FormDevice->ComboBoxOutDevs,FormDevice->LabelMidiDevError);

  FormDebug->Log(MainForm, "Sizes:");
  FormDebug->Log(MainForm, "UInt8 "+AnsiString(sizeof(UInt8)));
  FormDebug->Log(MainForm, "UInt16 "+AnsiString(sizeof(UInt16)));
  FormDebug->Log(MainForm, "UInt32 "+AnsiString(sizeof(UInt32)));

  FormDebug->Log(MainForm, "WORD "+AnsiString(sizeof(WORD)));
  FormDebug->Log(MainForm, "DWORD "+AnsiString(sizeof(DWORD)));

  Midi_Init();
  QuadGT_Init();

  QuadGT_Display_Update_Patch((UInt8) StrToInt(QuadPatchNum->Text));

}
//---------------------------------------------------------------------------

//===========================================================================
//                                                                  VARIABLES
//===========================================================================

union {
   public:
   unsigned long word;
   unsigned char data[4];
} message;

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------




void __fastcall TMainForm::MenuHelpAboutClick(TObject *Sender)
{
  FormAbout->ShowModal();
}

//---------------------------------------------------------------------------



void __fastcall TMainForm::UpDownQuadPatchClick(TObject *Sender,
      TUDBtnType Button)
{
 QuadPatchNumExit(Sender);
}
//---------------------------------------------------------------------------





void __fastcall TMainForm::TimerMidiCountsTimer(TObject *Sender)
{
  int msg, sysex,other;
  static int count=0;

  Midi_Get_Counts(&msg, &sysex, &other);
  FormDebug->Counter(Sender,AnsiString(count++)+"  Msg Count: "+AnsiString(msg)+"  Sysex Count: "+AnsiString(sysex)+"  Other Count: "+AnsiString(other));

}
//---------------------------------------------------------------------------



void __fastcall TMainForm::ButtonMidiDevOpenClick(TObject *Sender)
{
   unsigned int status;

   FormDevice->ShowModal();

   status=Midi_Out_Open(FormDevice->ComboBoxOutDevs->ItemIndex);
   if (status != 0)
   {
     FormError->ShowError(status,"opening Midi output device");
   }
   else
   {
     status=Midi_In_Open(FormDevice->ComboBoxInDevs->ItemIndex);
     if (status != 0)
     {
       FormError->ShowError(status,"opening Midi input device");
     }
     else
     {
       FormDebug->Log(Sender,"Midi devices succesfully opened");
       FormDevice->ButtonMidiDevOpen->Enabled=false;
       FormDevice->ButtonMidiDevClose->Enabled=true;
     }
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MenuFileExitClick(TObject *Sender)
{
  Midi_In_Close();
  Application->Terminate();
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::QuadParamChange(TObject *Sender)
{
  QuadtGT_Param_Change(Sender);
}



//---------------------------------------------------------------------------
void __fastcall TMainForm::RadioDelayClick(TObject *Sender)
{
  // TBD
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RadioPitchClick(TObject *Sender)
{
  // TBD
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RadioReverbClick(TObject *Sender)
{
  //TBD
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::QuadPatchReadClick(TObject *Sender)
{
  UInt8 patch;
  long int status;

  patch = (UInt8) StrToInt(QuadPatchNum->Text);
  status=Midi_Out_Dump_Req(patch);

  if (status != 0)
  {
     FormError->ShowError(status, "sending SYSEX to Midi output device");
  }

}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ButtonMidiDevCloseClick(TObject *Sender)
{
  unsigned int status;

  status=Midi_In_Close();
  if (status != 0)
  {
    FormError->ShowError(status, "closing Midi input device");
  }

  status=Midi_Out_Close();
  if (status != 0)
  {
    FormError->ShowError(status, "closing Midi output device");
  }

  FormDevice->ButtonMidiDevOpen->Enabled=true;
  FormDevice->ButtonMidiDevClose->Enabled=false;

}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  Midi_In_Close();
  Application->Terminate();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::TimerProcessTimer(TObject *Sender)
{
   Midi_Sysex_Process();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::QuadBankReadClick(TObject *Sender)
{
  UInt8 status;

  status=Midi_Out_Dump_Req(QUADGT_ALL);

  if (status != 0)
  {
     FormError->ShowError(status, "sending SYSEX data request to Midi output device");
  }

    
}
//---------------------------------------------------------------------------
// Name        : QuadProgWriteClick
// Description : Sends a sysex command to set the program/patch number when
//               the user clicks the "Prog Write" button
void __fastcall TMainForm::QuadProgWriteClick(TObject *Sender)
{
  long int status;

  status=Midi_Out_Edit(0, 0, 65535);

  if (status != 0)
  {
     FormError->ShowError(status, "sending SYSEX edit command to Midi output device");
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::QuadMidiWriteClick(TObject *Sender)
{
  long int status;

  status=Midi_Out_Edit(1, 0, 65535);

  if (status != 0)
  {
     FormError->ShowError(status, "sending SYSEX edit command to Midi output device");
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::TestClick(TObject *Sender)
{
  Midi_Test();    
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::BarChange(TObject *Sender)
{
  TTrackBar * bar = (TTrackBar *)Sender;
  bar->Hint=AnsiString(bar->Max - bar->Position);
}
//---------------------------------------------------------------------------





void __fastcall TMainForm::QuadPatchNumKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if (Key == 13)
  {
    QuadPatchNumExit(Sender);
  }
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::QuadPatchNumExit(TObject *Sender)
{
  FormDebug->Log(Sender,"Quad Patch [" + QuadPatchNum->Text + "] selected");
  QuadGT_Display_Update_Patch((UInt8) StrToInt(QuadPatchNum->Text));
}
//---------------------------------------------------------------------------



