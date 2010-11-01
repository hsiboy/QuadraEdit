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
// Name        : TMainForm::Init
// Description : Initialises the GUI main form
// Parameters  : NONE.
// Returns     : NONE.
//---------------------------------------------------------------------------
void __fastcall TMainForm::Init(void)
{
  FormDebug->Log(MainForm, "Starting...");

  // Set up Quadraverb specific values
  UpDownQuadPatch->Min=0;
  UpDownQuadPatch->Max=QUAD_NUM_PATCH-1;
  EditQuadPatchName->Text="---";

  DisableIO();

  Midi_Init();
  QuadGT_Init();

  QuadGT_Redraw_Patch((UInt8) StrToInt(QuadPatchNum->Text));

  DeviceOpenClick(NULL);

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



void __fastcall TMainForm::UpDownResNumClick(TObject *Sender,
      TUDBtnType Button)
{
 ResNumberExit(Sender);
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



void __fastcall TMainForm::DeviceOpenClick(TObject *Sender)
{
  int device_count;

  // Get list of Midi devices
  device_count = Midi_Get_IO_Dev_List(FormDevice->ComboBoxInDevs);

  if (device_count > 0)
  {
    FormDevice->ShowModal();
  }
  else
  {
    FormError->ShowError("No MIDI IO devices available");
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MenuFileExitClick(TObject *Sender)
{
  DeviceCloseClick(Sender);
  Application->Terminate();
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void __fastcall TMainForm::DeviceCloseClick(TObject *Sender)
{
  unsigned int status;

  status=Midi_IO_Close();
  if (status != 0)
  {
    FormError->ShowErrorCode(status, "closing Midi device");
  }

  DisableIO();
}

void __fastcall TMainForm::EnableIO(void)
{
    QuadPatchRead->Enabled=TRUE;
    QuadPatchWrite->Enabled=TRUE;
    QuadBankRead->Enabled=TRUE;
    QuadBankWrite->Enabled=TRUE;
    Open->Enabled=FALSE;
    Close->Enabled=TRUE;
}

void __fastcall TMainForm::DisableIO(void)
{
    QuadPatchRead->Enabled=FALSE;
    QuadPatchWrite->Enabled=FALSE;
    QuadBankRead->Enabled=FALSE;
    QuadBankWrite->Enabled=FALSE;
    Open->Enabled=TRUE;
    Close->Enabled=FALSE;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  DeviceCloseClick(Sender);
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
     FormError->ShowErrorCode(status, "sending SYSEX data request to Midi output device");
  }

    
}
//---------------------------------------------------------------------------
// Name        : QuadProgWriteClick
// Description : Sends a sysex command to set the program/patch number when
//               the user clicks the "Prog Write" button
void __fastcall TMainForm::QuadProgWriteClick(TObject *Sender)
{
  UInt32 status;
  UInt16 prog=10;  // Harcoded to program 10 for now

  status=Midi_Out_Edit(0, 0, prog);

  if (status != 0)
  {
     FormError->ShowErrorCode(status, "sending SYSEX edit command to Midi output device");
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::QuadMidiWriteClick(TObject *Sender)
{
  long int status;

  status=Midi_Out_Edit(1, 0, 65535);

  if (status != 0)
  {
     FormError->ShowErrorCode(status, "sending SYSEX edit command to Midi output device");
  }
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
  QuadGT_Redraw_Patch((UInt8) StrToInt(QuadPatchNum->Text));
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ResNumberKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if (Key == 13)
  {
    ResNumberExit(Sender);
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ResNumberExit(TObject *Sender)
{
  QuadGT_Redraw_Resonator((UInt8) StrToInt(QuadPatchNum->Text));
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::UpDownQuadPatchClick(TObject *Sender,
      TUDBtnType Button)
{
 QuadPatchNumExit(Sender);
}
//---------------------------------------------------------------------------

