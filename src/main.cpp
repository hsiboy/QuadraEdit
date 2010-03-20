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
#include <mmsystem.h>                       // MultiMedia System headers

#define QUAD_PATCH_MIN (0)
#define QUAD_PATCH_MAX (9)

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

void __fastcall TMainForm::Init(void)
{
  FormDebug->Log(MainForm, "Starting...");

  UpDownQuadPatch->Min=QUAD_PATCH_MIN;
  UpDownQuadPatch->Max=QUAD_PATCH_MAX;
  EditQuadPatchName->Text="---";

  Midi_Get_Dev_Lists(ComboBoxInDevs,ComboBoxOutDevs,LabelMidiDevError);

  RadioConfigClick(MainForm);


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

int instrument;
int note;

void __fastcall TMainForm::ButtonSilenceClick(TObject *Sender)
{
      note = StrToInt(MainForm->EditSilenceNote->Text);
      message.data[0] = 0x80;                                     // note off
      message.data[1] = note;                                       // note #
      message.data[2] = 100;                   // once set, no need to repeat
      message.data[3] = 0;                     // once set, no need to repeat
      Midi_Out_ShortMsg(message.word);
}

//--------------------------------------------------------- Instrument Button
void __fastcall TMainForm::ButtonInstrumentClick(TObject *Sender)
{
   instrument = StrToInt(MainForm->EditInstrument->Text);
   message.data[0] = 0xC0;                               // choose instrument
   message.data[1] = instrument;
   Midi_Out_ShortMsg(message.word);
}

void __fastcall TMainForm::ButtonPlayClick(TObject *Sender)
{
     note = StrToInt(MainForm->EditPlayNote->Text);
      message.data[0] = 0x90;                                      // note on
      message.data[1] = note;                                       // note #
      message.data[2] = 100;                   // once set, no need to repeat
      message.data[3] = 0;                     // once set, no need to repeat
      Midi_Out_ShortMsg(message.word);

}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------




void __fastcall TMainForm::MenuHelpAboutClick(TObject *Sender)
{
  // Display the about form
  FormAbout->FormShow(Sender);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::QuadPatchNumChange(TObject *Sender)
{
   // TBD: Check it is numeric and in range

   PanelQuad->Caption = "Quad Patch " + QuadPatchNum->Text + " selected";

  // TBD: User changed the patch number, update display
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::UpDownQuadPatchClick(TObject *Sender,
      TUDBtnType Button)
{
 // Nothing to do - handled automatically
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

   status=Midi_Out_Open(ComboBoxOutDevs->ItemIndex);
   if (status != MMSYSERR_NOERROR)
   {
     FormError->LabelError->Caption="Error ["+AnsiString(status)+"] opening Midi output device";
     FormError->FormShow(Sender);
   }
   else
   {
     status=Midi_In_Open(ComboBoxInDevs->ItemIndex);
     if (status != MMSYSERR_NOERROR)
     {
       FormError->LabelError->Caption="Error ["+AnsiString(status)+"] opening Midi input device";
       FormError->FormShow(Sender);
     }
     else
     {
       FormDebug->Log(Sender,"Midi devices succesfully opened");
       ButtonMidiDevOpen->Enabled=false;
       ButtonMidiDevClose->Enabled=true;
     }
   }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MenuFileExitClick(TObject *Sender)
{
  Application->Terminate();
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::RadioConfigClick(TObject *Sender)
{
  // Handler for the Config radio buttons
 
  // TBD: How to set it up so enabling/disabling a panel causes it's children to also be enabled/disabled

  // 0: EQ-Pitch-Delay-Reverb;
  if (Config0->Checked == TRUE)
  {
    FormDebug->Log(Sender,"Config 0");
    PanelQuadEq->Visible=true;
    PanelQuadPitch->Visible=true;
    PanelQuadDelay->Visible=true;
    PanelQuadReverb->Visible=true;

    DelayMultiTap->Enabled=false;
  }

  // 1: Leslie-Delay-Reverb;
  else if (Config1->Checked == TRUE)
  {
    FormDebug->Log(Sender,"Config 1");
    PanelQuadEq->Visible=false;
    PanelQuadPitch->Visible=false;
    PanelQuadDelay->Visible=true;
    PanelQuadReverb->Visible=true;

    DelayMultiTap->Enabled=false;
  }

  // 2: Graphic Eq-Delay
  else if (Config2->Checked == TRUE)
  {
    PanelQuadEq->Visible=true;
    PanelQuadPitch->Visible=false;
    PanelQuadDelay->Visible=true;
    PanelQuadReverb->Visible=false;

    DelayMultiTap->Enabled=false;
  }

  // 3: 5 Band Eq-Pitch-Delay
  else if (Config3->Checked == TRUE)
  {
    PanelQuadEq->Visible=true;
    PanelQuadPitch->Visible=true;
    PanelQuadDelay->Visible=true;
    PanelQuadReverb->Visible=false;

    DelayMultiTap->Enabled=true;
  }

  // 4: 3 Band Eq-Reverb
  else if (Config4->Checked == TRUE)
  {
    PanelQuadEq->Visible=true;
    PanelQuadPitch->Visible=false;
    PanelQuadDelay->Visible=false;
    PanelQuadReverb->Visible=true;

    DelayMultiTap->Enabled=false;
  }

  // 5: Ring Mod-Delay-Reverb
  else if (Config5->Checked == TRUE)
  {
    PanelQuadEq->Visible=false;
    PanelQuadPitch->Visible=false;
    PanelQuadDelay->Visible=true;
    PanelQuadReverb->Visible=true;

    DelayMultiTap->Enabled=false;
  }

  // 6: Resonator-Delay-Reverb
  else if (Config6->Checked == TRUE)
  {
    PanelQuadEq->Enabled=false;
    PanelQuadPitch->Enabled=false;
    PanelQuadDelay->Enabled=true;
    PanelQuadReverb->Enabled=true;

    DelayMultiTap->Enabled=false;
  }

  // 7: Sampling
  else if (Config7->Checked == TRUE)
  {
    PanelQuadEq->Enabled=false;
    PanelQuadPitch->Enabled=false;
    PanelQuadDelay->Enabled=false;
    PanelQuadReverb->Enabled=false;

    DelayMultiTap->Enabled=false;
  }

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

  if (status != MMSYSERR_NOERROR)
  {
     FormError->LabelError->Caption="Error ["+AnsiString(status)+"] sending SYSEX to Midi output device";
     FormError->FormShow(Sender);
  }

}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ButtonMidiDevCloseClick(TObject *Sender)
{
  unsigned int status;

  status=Midi_In_Close();
  if (status != MMSYSERR_NOERROR)
  {
    FormError->LabelError->Caption="Error ["+AnsiString(status)+"] closing Midi input device";
    FormError->FormShow(Sender);
  }

  status=Midi_Out_Close();
  if (status != MMSYSERR_NOERROR)
  {
    FormError->LabelError->Caption="Error ["+AnsiString(status)+"] closing Midi output device";
    FormError->FormShow(Sender);
  }

  ButtonMidiDevOpen->Enabled=true;
  ButtonMidiDevClose->Enabled=false;

}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  Application->Terminate();
}
//---------------------------------------------------------------------------

