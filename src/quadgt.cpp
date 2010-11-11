#include <vcl.h>
#include <mmsystem.h>                       // MultiMedia System headers

#include <stdlib.h>
#include <stdio.h>

#pragma hdrstop

#include "types.h"
#include "midi.h"
#include "error.h"
#include "main.h"
#include "debug.h"
#include "display.h"

#include "quadgt.h"

#define GEQ_OFFSET (14)
#define RES_TUNE_OFFSET (24)

#define QUAD_NUM_BANK (2)

static tQuadGT_Prog QuadtGT_Bank[QUAD_NUM_BANK][QUAD_NUM_PATCH];

static int bank=0;

void __fastcall TMainForm::QuadBankClick(TObject *Sender)
{
  UInt8 prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  bank=QuadBank->ItemIndex;

  QuadGT_Redraw_Patch(bank, prog);
}
// 
//---------------------------------------------------------------------------
// Name        : save_patch
// Description : DEBUG: Save a patch to a an ASCII hex disk file for analysis
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void save_patch(UInt8 *data, UInt32 length, char *fname)
{
  FILE *handle;
  UInt32 i;
  handle= fopen(fname,"w");

  for (i=0; i<length; i++)
  {
    fprintf(handle,"%2.2X ",*(data+i));
  }
  fclose(handle);
}

//---------------------------------------------------------------------------
//Decode two bytes of QuadreverbGT binary data to an internal format 16bit UInt
UInt16 QuadGT_Decode_16Bit(UInt8 *data)
{
  //FormDebug->Log(NULL, "Decode "+AnsiString(*data)+", "+AnsiString(*(data+1)));
  return( (UInt16) *(data + 1)  + ((UInt16) *(data)<<8));
}

UInt16 QuadGT_Decode_16Bit_Split(UInt8 *msb, UInt8 *lsb)
{
  //FormDebug->Log(NULL, "Decode "+AnsiString(*data)+", "+AnsiString(*(data+1)));
  return( (UInt16) *msb  + ((UInt16) *(lsb)<<8));
}

// Encode an internal format 16 bit UInt to two bytes of QuadGT format data (8 bits each)
void QuadGT_Encode_16Bit(const UInt16 word, UInt8 *data)
{
  *data= (UInt8)(word>>8); 
  *(data+1)=(UInt8)(word & 0xff);
  //FormDebug->Log(NULL, "Encoded "+AnsiString(*data)+", "+AnsiString(*(data+1)));
}

void QuadGT_Encode_16Bit_Split(const UInt16 word, UInt8 *msb, UInt8 *lsb)
{
  *lsb= (UInt8)(word>>8); 
  *msb=(UInt8)(word & 0xff);
  //FormDebug->Log(NULL, "Encoded "+AnsiString(*data)+", "+AnsiString(*(data+1)));
}

// 
//---------------------------------------------------------------------------
// Name        : QuadGT_Decode_From_Sysex
// Description : Decode Midi SysEx data in 7bits to Quadraverb data in 8bits
// Parameter 1 : Pointer to sysex data
// Parameter 2 : Length of sysex data in bytes
// Parameter 1 : Pointer to QuadtGt fromat data (returned)
// Parameter 2 : Length of QuadGt format data in bytes
// Returns     : Number of output bytes.  Zero on error.
//---------------------------------------------------------------------------
UInt32 QuadGT_Decode_From_Sysex(UInt8 *in, UInt32 length, UInt8* out, UInt32 out_len)
{
  UInt8 oc;
  UInt32 i,j;
  UInt8 shift;

  // DEBUG: Write the patch in sysex format to a disk file
  save_patch(in,length,"patch_in_sysex.txt");

  j=0;
  oc = 0;
  for (i=0; i<length; i++) {
     if (shift = i % 8) {
        oc = (oc << shift) + (in[i] >> (7-shift));
        out[j++]=oc;
     }
     oc = in[i];

     // Protect for overflow on output data
     if (j >= out_len) return(0);
  }
  
  return(j);
}

// 
//---------------------------------------------------------------------------
// Name        : QuadGT_Encode_To_Sysex
// Description : Encode 8bit quadraverb data into low 7 bits for SysEx
// Parameter 1 : Pointer to QuadGt format data
// Parameter 2 : Length of QuadGt format data in bytes
// Parameter 1 : Pointer to sysex fromat data (returned)
// Parameter 2 : Length of sysex format data in bytes
// Returns     : Number of output bytes.  Zero on error.
//---------------------------------------------------------------------------
UInt32 QuadGT_Encode_To_Sysex(UInt8 *in, UInt32 length, UInt8 * out, UInt32 out_len)
{
  UInt8 lc,cc;
  UInt32 i,j;
  UInt8 shift;

  i=0;
  j=0;
  lc=0;
  cc=in[0];
  while (i<=length)
  {
    shift = j % 8;

    if (shift == 0)
    {
      out[j]= cc>>1;
    }
    else
    {
      out[j]= (cc >> (shift+1)) | ((lc<< (7-shift))&0x7F);
    }
    lc=cc;
    if (shift != 7) cc=in[++i];
    j++;

    // Protect for overflow on output data
    if (j > out_len) {
      FormDebug->Log(NULL,"Encode to SYSEX: OVERFLOW "+AnsiString(i)+" "+AnsiString(j));
      return(0);
    }
  }

  
  // DEBUG: Write the patch in sysex format to a disk file
  save_patch(out,j,"patch_out_sysex.txt");
  FormDebug->Log(NULL,"Encode to SYSEX: OK "+IntToHex(out[out_len-2],2)+" "+IntToHex(out[out_len-1],2));

  return(j);
}


//---------------------------------------------------------------------------
// Name        : QuadGT_Init
// Description : Initialise the QuadtGT specific data at program start.
// Parameters  : NONE.
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Init(void)
{
  UInt8 bank, patch;
  char patch_name[NAME_LENGTH+1];
  TStringList *ModTgtReverb_A=new(TStringList);

  for (bank=0; bank<QUAD_NUM_BANK; bank++)
  {
    for (patch=0; patch<QUAD_NUM_PATCH; patch++)
    {
      // Set all parameters to 0
      memset(&QuadtGT_Bank[bank][patch], 0x00, sizeof(tQuadGT_Prog));

      // Create a program name
      sprintf(patch_name,"Patch-%2.2d",patch);
      strcpy(QuadtGT_Bank[bank][patch].name,patch_name);
    }
  }

  ModTgtReverb_A->Add("In Mix");
  ModTgtReverb_A->Add("Predelay");
  ModTgtReverb_A->Add("Pre Mix");
  ModTgtReverb_A->Add("Delay");
  ModTgtReverb_A->Add("Diffusion");

  // Default the list of Midi Modulation targets
  MainForm->ModTarget->Items=ModTgtReverb_A;

}

//---------------------------------------------------------------------------
// Name        : QuadGT_Redraw_Patch
// Description : Update the on screen display of the parameters for
//               the given program based on the data in QuadtGT_Bank
// Parameter 1 : Program number for which display is to be updated
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Redraw_Patch(UInt8 bank, UInt8 prog)
{
  UInt8 config;

  /* Program Number */
  MainForm->QuadPatchNum->Text=AnsiString(prog);

  /* Program name */
  MainForm->EditQuadPatchName->Text=AnsiString(QuadtGT_Bank[bank][prog].name);

  /* Configuration */
  MainForm->QuadConfig->ItemIndex=QuadtGT_Bank[bank][prog].config;

  QuadGT_Redraw_Reverb(prog);
  QuadGT_Redraw_Delay(prog);
  QuadGT_Redraw_Pitch(prog);
  QuadGT_Redraw_Eq(prog);
  QuadGT_Redraw_Mix(prog);
  QuadGT_Redraw_Mod(prog);
  QuadGT_Redraw_Preamp(prog);
  QuadGT_Redraw_Resonator(prog);
}

//---------------------------------------------------------------------------
// Name        : QuadGT_Redraw_Reverb
// Description : Update the on screen display of the reverb parameters for
//               the given program based on the data in QuadtGT_Bank[bank]
// Parameter 1 : Program number for which display is to be updated
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Redraw_Reverb(const UInt8 prog)
{
  UInt8 val;

  // Select whether reverb is visible based on configuration
  if (QuadtGT_Bank[bank][prog].config==CFG0_EQ_PITCH_DELAY_REVERB)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=true;
    MainForm->ReverbInMix->Visible=true;
    MainForm->ReverbInMixVal->Visible=true;

    MainForm->ReverbInput1->Items->Strings[0]="Preamp";
    MainForm->ReverbInput1->Items->Strings[1]="Eq";
    MainForm->ReverbInput1->Items->Strings[2]="Pitch";
    if (MainForm->ReverbInput1->Items->Count == 3) MainForm->ReverbInput1->Items->Add("Delay");

    MainForm->ReverbInput2->Items->Strings[0]="Pitch";
    MainForm->ReverbInput2->Items->Strings[1]="Delay";
  }
  else if (QuadtGT_Bank[bank][prog].config==CFG1_LESLIE_DELAY_REVERB)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=true;
    MainForm->ReverbInMix->Visible=true;
    MainForm->ReverbInMixVal->Visible=true;

    MainForm->ReverbInput1->Items->Strings[0]="Preamp";
    MainForm->ReverbInput1->Items->Strings[1]="Leslie";
    MainForm->ReverbInput1->Items->Strings[2]="Delay";
    if (MainForm->ReverbInput1->Items->Count == 4) MainForm->ReverbInput1->Items->Delete(3);

    MainForm->ReverbInput2->Items->Strings[0]="Leslie";
    MainForm->ReverbInput2->Items->Strings[1]="Delay";
  }
  else if ((QuadtGT_Bank[bank][prog].config==CFG2_GEQ_DELAY) || 
           (QuadtGT_Bank[bank][prog].config==CFG3_5BANDEQ_PITCH_DELAY) ||
           (QuadtGT_Bank[bank][prog].config==CFG7_SAMPLING))
  {
    MainForm->QuadReverb->Visible=false;
  }
  else if (QuadtGT_Bank[bank][prog].config==CFG4_3BANDEQ_REVERB)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=FALSE;
    MainForm->ReverbInMix->Visible=FALSE;
    MainForm->ReverbInMixVal->Visible=false;
  }
  else if (QuadtGT_Bank[bank][prog].config==CFG5_RINGMOD_DELAY_REVERB)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=true;
    MainForm->ReverbInMix->Visible=true;
    MainForm->ReverbInMixVal->Visible=true;

    MainForm->ReverbInput1->Items->Strings[0]="Preamp";
    MainForm->ReverbInput1->Items->Strings[1]="Ring";
    MainForm->ReverbInput1->Items->Strings[2]="Delay";
    if (MainForm->ReverbInput1->Items->Count == 4) MainForm->ReverbInput1->Items->Delete(3);

    MainForm->ReverbInput2->Items->Strings[0]="Ring";
    MainForm->ReverbInput2->Items->Strings[1]="Delay";
  }
  else if (QuadtGT_Bank[bank][prog].config==CFG6_RESONATOR_DELAY_REVERB)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=true;
    MainForm->ReverbInMix->Visible=true;
    MainForm->ReverbInMixVal->Visible=true;

    MainForm->ReverbInput1->Items->Strings[0]="Preamp";
    MainForm->ReverbInput1->Items->Strings[1]="Resonator";
    MainForm->ReverbInput1->Items->Strings[2]="Delay";
    if (MainForm->ReverbInput1->Items->Count == 4) MainForm->ReverbInput1->Items->Delete(3);

    MainForm->ReverbInput2->Items->Strings[0]="Resonator";
    MainForm->ReverbInput2->Items->Strings[1]="Delay";
  }

  if (MainForm->QuadReverb->Visible == true)
  {
    MainForm->ReverbType->ItemIndex = QuadtGT_Bank[bank][prog].reverb_mode;

    MainForm->ReverbInput1->ItemIndex = QuadtGT_Bank[bank][prog].reverb_input_1;
    MainForm->ReverbInput2->ItemIndex = QuadtGT_Bank[bank][prog].reverb_input_2;
    FormDebug->Log(NULL, "Reverb In 1 "+AnsiString(QuadtGT_Bank[bank][prog].reverb_input_1));
    FormDebug->Log(NULL, "Reverb In 2 "+AnsiString(QuadtGT_Bank[bank][prog].reverb_input_2));

    RedrawVertBarTextU8(MainForm->ReverbPreDelay, MainForm->ReverbPreDelayVal, QuadtGT_Bank[bank][prog].reverb_predelay, 1);
    
    RedrawHorizBarTextS8(MainForm->ReverbInMix, MainForm->ReverbInMixVal, QuadtGT_Bank[bank][prog].reverb_input_mix);
    RedrawHorizBarTextS8(MainForm->ReverbPreMix, MainForm->ReverbPreMixVal, QuadtGT_Bank[bank][prog].reverb_predelay_mix);

    RedrawVertBarTextU8(MainForm->ReverbLowDecay,  MainForm->ReverbLowDecayVal,  QuadtGT_Bank[bank][prog].reverb_low_decay, 0);
    RedrawVertBarTextU8(MainForm->ReverbHighDecay, MainForm->ReverbHighDecayVal, QuadtGT_Bank[bank][prog].reverb_high_decay, 0);
    RedrawVertBarTextU8(MainForm->ReverbDecay,     MainForm->ReverbDecayVal,     QuadtGT_Bank[bank][prog].reverb_decay, 0);

    RedrawVertBarTextU8(MainForm->ReverbDiffusion,  MainForm->ReverbDiffusionVal, QuadtGT_Bank[bank][prog].reverb_diffusion,0);

    RedrawVertBarTextU8(MainForm->ReverbDensity,  MainForm->ReverbDensityVal, QuadtGT_Bank[bank][prog].reverb_density,0);

    MainForm->ReverbGateOn->Checked = (QuadtGT_Bank[bank][prog].reverb_gate==1) ? TRUE : FALSE;
    RedrawVertBarTextU8(MainForm->ReverbGateHold,  MainForm->ReverbGateHoldVal, QuadtGT_Bank[bank][prog].reverb_gate_hold,0);
    RedrawVertBarTextU8(MainForm->ReverbGateRelease,  MainForm->ReverbGateReleaseVal, QuadtGT_Bank[bank][prog].reverb_gate_release,0);
    RedrawVertBarTextU8(MainForm->ReverbGateLevel,  MainForm->ReverbGateLevelVal, QuadtGT_Bank[bank][prog].reverb_gated_level,0);
  }
}

//---------------------------------------------------------------------------
// Name        : QuadGT_Redraw_Delay
// Description : 
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Redraw_Delay(const UInt8 prog)
{
  UInt8 mode;
  mode=QuadtGT_Bank[bank][prog].delay_mode;

  if (QuadtGT_Bank[bank][prog].config==CFG4_3BANDEQ_REVERB) 
  {
    MainForm->PanelQuadDelay->Visible = FALSE;
  }
  else
  {
    MainForm->PanelQuadDelay->Visible = TRUE;

    // Position the delay panel based on configuration
    if (QuadtGT_Bank[bank][prog].config==CFG6_RESONATOR_DELAY_REVERB)
    {
      MainForm->PanelQuadDelay->Left=660;
    }
    else
    {
      MainForm->PanelQuadDelay->Left=420;
    }

    if (QuadtGT_Bank[bank][prog].config==CFG3_5BANDEQ_PITCH_DELAY)
    {
      // Add "Multitap" to list of possibilities
      int i=MainForm->DelayMode->Items->IndexOf("Multitap");
      if (i < 0) MainForm->DelayMode->Items->Add("Multitap");
    }
    else
    {
      // Remove "Multitap" from list of possibilities
      int i=MainForm->DelayMode->Items->IndexOf("Multitap");
      if (i >= 0) MainForm->DelayMode->Items->Delete(i);
      if (mode == DELAYMODE3_MULTITAP)
      {
        mode=QuadtGT_Bank[bank][prog].delay_mode=DELAYMODE0_MONO;
      }
    }

    // Mono or pingpong
    if ((mode == DELAYMODE0_MONO) || (mode == DELAYMODE2_PINGPONG))
    {
      MainForm->DelayTap->Visible=FALSE;
      MainForm->DelayNonTap->Visible=TRUE;
      MainForm->DelayRight->Visible=FALSE;
      MainForm->DelayRightVal->Visible=FALSE;
      MainForm->DelayRightLabel->Visible=FALSE;
      MainForm->DelayRighFBack->Visible=FALSE;
      MainForm->DelayRightFBackVal->Visible=FALSE;
      MainForm->DelayRightFBackLabel->Visible=FALSE;

      // Change left delay labels to suit mono
      MainForm->DelayLeftLabel->Caption="Delay";
      MainForm->DelayLeftFBackLabel->Caption="FBack";
    }

    // Stereo
    else if (mode == DELAYMODE1_STEREO)
    {
      MainForm->DelayTap->Visible=FALSE;
      MainForm->DelayNonTap->Visible=TRUE;
      MainForm->DelayRight->Visible=TRUE;
      MainForm->DelayRightVal->Visible=TRUE;
      MainForm->DelayRightLabel->Visible=TRUE;
      MainForm->DelayRighFBack->Visible=TRUE;
      MainForm->DelayRightFBackVal->Visible=TRUE;
      MainForm->DelayRightFBackLabel->Visible=TRUE;

      // Change left delay labels to suit stereo
      MainForm->DelayLeftLabel->Caption="L.Delay";
      MainForm->DelayLeftFBackLabel->Caption="L.FB";
    }

    // Multitap
    else if (mode == DELAYMODE3_MULTITAP)
    {
      MainForm->DelayTap->Visible=TRUE;
      MainForm->DelayNonTap->Visible=FALSE;
    }
  }

  if ((QuadtGT_Bank[bank][prog].config==CFG0_EQ_PITCH_DELAY_REVERB) ||
      (QuadtGT_Bank[bank][prog].config==CFG2_GEQ_DELAY) ||
      (QuadtGT_Bank[bank][prog].config==CFG3_5BANDEQ_PITCH_DELAY))
  {
    MainForm->DelayInput->Visible = TRUE;
  }
  else if ((QuadtGT_Bank[bank][prog].config==CFG1_LESLIE_DELAY_REVERB) ||
      (QuadtGT_Bank[bank][prog].config==CFG5_RINGMOD_DELAY_REVERB) ||
      (QuadtGT_Bank[bank][prog].config==CFG6_RESONATOR_DELAY_REVERB))
  {
    MainForm->DelayInput->Visible = TRUE;
  }
  
  if (MainForm->PanelQuadDelay->Visible == TRUE)
  {
    MainForm->DelayMode->ItemIndex=mode;
    MainForm->DelayInput->ItemIndex=QuadtGT_Bank[bank][prog].delay_input;
    RedrawVertBarTextS8(MainForm->DelayInMix, MainForm->DelayInMixVal, QuadtGT_Bank[bank][prog].delay_input_mix);

    // TBD: is this fixed as reverb?
    MainForm->DelayInMix1->Caption = "Reverb";
    if (MainForm->DelayInput->ItemIndex == 0) MainForm->DelayInMix2->Caption = "Preamp";
    else  MainForm->DelayInMix2->Caption = "Eq";
 
    if ((mode == DELAYMODE0_MONO) || (mode == DELAYMODE2_PINGPONG))
    {
      RedrawVertBarTextU16(MainForm->DelayLeft, MainForm->DelayLeftVal, QuadtGT_Bank[bank][prog].delay_left,0);
      RedrawVertBarTextU8(MainForm->DelayLeftFBack, MainForm->DelayLeftFBackVal, QuadtGT_Bank[bank][prog].delay_left_feedback,0);
      // TBD: Set different max delay times dependenat on delay mode and configuration
    }
    else if (mode == DELAYMODE1_STEREO)
    {
      RedrawVertBarTextU16(MainForm->DelayLeft, MainForm->DelayLeftVal, QuadtGT_Bank[bank][prog].delay_left,0);
      RedrawVertBarTextU8(MainForm->DelayLeftFBack, MainForm->DelayLeftFBackVal, QuadtGT_Bank[bank][prog].delay_left_feedback,0);
      RedrawVertBarTextU16(MainForm->DelayRight, MainForm->DelayRightVal, QuadtGT_Bank[bank][prog].delay_right,0);
      RedrawVertBarTextU8(MainForm->DelayRighFBack, MainForm->DelayRightFBackVal, QuadtGT_Bank[bank][prog].delay_right_feedback,0);
    }
    else if (mode == DELAYMODE3_MULTITAP)
    {
      UInt8 tap = StrToInt(MainForm->TapNumber->Text)-1;

      RedrawVertBarTextU8(MainForm->TapDelay,    MainForm->TapDelayVal, QuadtGT_Bank[bank][prog].tap.tap_delay[tap], 0);
      RedrawVertBarTextU8(MainForm->TapPan,      MainForm->TapPanVal, QuadtGT_Bank[bank][prog].tap.tap_pan[tap], 0);
      RedrawVertBarTextU8(MainForm->TapVol,      MainForm->TapVolVal, QuadtGT_Bank[bank][prog].tap.tap_volume[tap], 0);
      RedrawVertBarTextU8(MainForm->TapFeedback, MainForm->TapFeedbackVal, QuadtGT_Bank[bank][prog].tap.tap_feedback[tap], 0);
    }
  }
}

//---------------------------------------------------------------------------
// Name        : QuadGT_Redraw_Pitch
// Description : 
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Redraw_Pitch(const UInt8 prog)
{
  if ((QuadtGT_Bank[bank][prog].config==CFG0_EQ_PITCH_DELAY_REVERB) ||
      (QuadtGT_Bank[bank][prog].config==CFG3_5BANDEQ_PITCH_DELAY))
  {
    MainForm->QuadPitch->Visible=TRUE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;

    MainForm->PitchInput->Visible = TRUE;

    MainForm->PitchMode->ItemIndex = QuadtGT_Bank[bank][prog].pitch.pitch_mode;
    MainForm->PitchInput->ItemIndex = QuadtGT_Bank[bank][prog].pitch.pitch_input;
    MainForm->PitchWave->ItemIndex = QuadtGT_Bank[bank][prog].pitch.lfo_waveform;

    RedrawVertBarTextU8(MainForm->PitchSpeed,    MainForm->PitchSpeedVal,    QuadtGT_Bank[bank][prog].pitch.lfo_speed, 0);
    RedrawVertBarTextU8(MainForm->PitchDepth,    MainForm->PitchDepthVal,    QuadtGT_Bank[bank][prog].pitch.lfo_depth, 0);
    RedrawVertBarTextU8(MainForm->PitchFeedback, MainForm->PitchFeedbackVal, QuadtGT_Bank[bank][prog].pitch.pitch_feedback, 0);
  }
  else if (QuadtGT_Bank[bank][prog].config==CFG1_LESLIE_DELAY_REVERB)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=TRUE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;

    MainForm->PitchInput->Visible = FALSE;

    // TBD: Redraw Leslie parameters
  }
  else if ((QuadtGT_Bank[bank][prog].config==CFG2_GEQ_DELAY) || 
           (QuadtGT_Bank[bank][prog].config==CFG6_RESONATOR_DELAY_REVERB) || 
           (QuadtGT_Bank[bank][prog].config==CFG7_SAMPLING))
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;

    MainForm->PitchInput->Visible = FALSE;

  }
  else if (QuadtGT_Bank[bank][prog].config==CFG4_3BANDEQ_REVERB)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=TRUE;

    // Redraw Reverb Chorus parameters
    RedrawVertBarTextU8(MainForm->ChorusSpeed, MainForm->ChorusSpeedVal, QuadtGT_Bank[bank][prog].pitch.lfo_speed, 1);
    RedrawVertBarTextU8(MainForm->ChorusDepth, MainForm->ChorusDepthVal, QuadtGT_Bank[bank][prog].pitch.lfo_depth, 1);

  }
  else if (QuadtGT_Bank[bank][prog].config==CFG5_RINGMOD_DELAY_REVERB)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=TRUE;
    MainForm->QuadChorus->Visible=FALSE;

    // TBD: Redraw Ring Mod parameters
    RedrawVertBarTextU16(MainForm->RingModSS,    MainForm->RingModSSVal,    QuadtGT_Bank[bank][prog].ring_mod_shift, 0);
    RedrawVertBarTextS8(MainForm->RingModInMix, MainForm->RingModInMixVal, QuadtGT_Bank[bank][prog].ring_mod_in_mix);
    RedrawVertBarTextS8(MainForm->RingModOutMix, MainForm->RingModOutMixVal, QuadtGT_Bank[bank][prog].ring_mod_out_mix);
  }

  if (MainForm->QuadPitch->Visible==TRUE)
  {
    if ((MainForm->PitchMode->ItemIndex == PITCHMODE0_MONO_CHORUS) ||
        (MainForm->PitchMode->ItemIndex == PITCHMODE1_STEREO_CHORUS))
    {
      MainForm->PitchWave->Visible = TRUE;
    }
    else
    {
      MainForm->PitchWave->Visible = FALSE;
    }

    if ((MainForm->PitchMode->ItemIndex == PITCHMODE0_MONO_CHORUS) ||
        (MainForm->PitchMode->ItemIndex == PITCHMODE1_STEREO_CHORUS) ||
        (MainForm->PitchMode->ItemIndex == PITCHMODE2_MONO_FLANGE) ||
        (MainForm->PitchMode->ItemIndex == PITCHMODE3_STEREO_FLANGE))
    {
      MainForm->PitchChorusFlange->Visible = TRUE;
    }
    else
    {
      MainForm->PitchChorusFlange->Visible = FALSE;
    }

    if ((MainForm->PitchMode->ItemIndex == PITCHMODE2_MONO_FLANGE) ||
        (MainForm->PitchMode->ItemIndex == PITCHMODE3_STEREO_FLANGE))
    {
      MainForm->PitchFlangeTriggerGroup->Visible = TRUE;
    }
    else
    {
      MainForm->PitchFlangeTriggerGroup->Visible = FALSE;
    }

    if (MainForm->PitchMode->ItemIndex == PITCHMODE4_DETUNE)
    {
      MainForm->PitchDetuneGroup->Visible = TRUE;
    }
    else
    {
      MainForm->PitchDetuneGroup->Visible = FALSE;
    }

    if (MainForm->PitchMode->ItemIndex == PITCHMODE5_PHASER)
    {
      MainForm->PhaserGroup->Visible = TRUE;
    }
    else
    {
      MainForm->PhaserGroup->Visible = FALSE;
    }
  }
}

//---------------------------------------------------------------------------
// Name        : QuadGT_Redraw_Eq
// Description : Redraw the EQ parameters
// Parameter 1 : Number of program to redraw
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Redraw_Eq(const UInt8 prog)
{
  // 3 Band Eq
  if ((QuadtGT_Bank[bank][prog].config==CFG0_EQ_PITCH_DELAY_REVERB) || (QuadtGT_Bank[bank][prog].config==CFG4_3BANDEQ_REVERB))
  {
    MainForm->PanelQuadEq3->Visible=TRUE;
    MainForm->Eq3Mode->ItemIndex =QuadtGT_Bank[bank][prog].mode;

    MainForm->EqFreq1->Position = QuadtGT_Bank[bank][prog].low_freq;
    MainForm->EqFreq1Val->Text = AnsiString(QuadtGT_Bank[bank][prog].low_freq);
    MainForm->EqAmp1->Position = -1 * (QuadtGT_Bank[bank][prog].low_amp - MainForm->EqAmp1->Max);
    MainForm->Eq3Amp1Val->Text = AnsiString(((double)(QuadtGT_Bank[bank][prog].low_amp - MainForm->EqAmp1->Max))/20.0);

    MainForm->EqFreq2->Position = QuadtGT_Bank[bank][prog].mid_freq;
    MainForm->EqFreq2Val->Text = AnsiString(QuadtGT_Bank[bank][prog].mid_freq);
    MainForm->EqAmp2->Position = -1 * (QuadtGT_Bank[bank][prog].mid_amp - MainForm->EqAmp2->Max);
    MainForm->Eq3Amp2Val->Text = AnsiString(((double)(QuadtGT_Bank[bank][prog].mid_amp - MainForm->EqAmp2->Max))/20.0);
    MainForm->EqQ2->Position = QuadtGT_Bank[bank][prog].mid_q;
    MainForm->EqQ2Val->Text = AnsiString(QuadtGT_Bank[bank][prog].mid_q);

    MainForm->EqFreq3->Position = QuadtGT_Bank[bank][prog].high_freq;
    MainForm->EqFreq3Val->Text = AnsiString(QuadtGT_Bank[bank][prog].high_freq);
    MainForm->EqAmp3->Position = -1 * (QuadtGT_Bank[bank][prog].high_amp - MainForm->EqAmp3->Max);
    MainForm->Eq3Amp3Val->Text = AnsiString(((double)(QuadtGT_Bank[bank][prog].high_amp - MainForm->EqAmp3->Max))/20.0);
    if (QuadtGT_Bank[bank][prog].config==CFG0_EQ_PITCH_DELAY_REVERB) MainForm->Eq3Mode->Visible=TRUE;
    else                                                             MainForm->Eq3Mode->Visible=FALSE;
  }
  else
  {
    MainForm->PanelQuadEq3->Visible=FALSE;
  }

  // 5 Band Eq
  if (QuadtGT_Bank[bank][prog].config==CFG3_5BANDEQ_PITCH_DELAY) 
  {
    MainForm->QuadEq5->Visible=TRUE;

    MainForm->Eq5Mode->ItemIndex =QuadtGT_Bank[bank][prog].mode;

    MainForm->Eq5Freq1->Position = QuadtGT_Bank[bank][prog].low_freq;
    MainForm->Eq5Freq1Val->Text = AnsiString(QuadtGT_Bank[bank][prog].low_freq);
    MainForm->Eq5Amp1->Position = -1 * (QuadtGT_Bank[bank][prog].low_amp - MainForm->Eq5Amp1->Max);
    MainForm->Eq5Amp1Val->Text = AnsiString(((double)(QuadtGT_Bank[bank][prog].low_amp - MainForm->Eq5Amp1->Max))/20.0);

    MainForm->Eq5Freq2->Position = QuadtGT_Bank[bank][prog].low_mid_freq;
    MainForm->Eq5Freq2Val->Text = AnsiString(QuadtGT_Bank[bank][prog].low_mid_freq);
    MainForm->Eq5Amp2->Position = -1 * (QuadtGT_Bank[bank][prog].low_mid_amp - MainForm->Eq5Amp2->Max);
    MainForm->Eq5Amp2Val->Text = AnsiString((double)(QuadtGT_Bank[bank][prog].low_mid_amp - MainForm->Eq5Amp2->Max)/20.0);
    MainForm->Eq5Q2->Position = QuadtGT_Bank[bank][prog].low_mid_q;
    MainForm->Eq5Q2Val->Text = AnsiString(QuadtGT_Bank[bank][prog].low_mid_q);

    MainForm->Eq5Freq3->Position = QuadtGT_Bank[bank][prog].mid_freq;
    MainForm->Eq5Freq3Val->Text = AnsiString(QuadtGT_Bank[bank][prog].mid_freq);
    MainForm->Eq5Amp3->Position = -1 * (QuadtGT_Bank[bank][prog].mid_amp - MainForm->Eq5Amp3->Max);
    MainForm->Eq5Amp3Val->Text = AnsiString((double)(QuadtGT_Bank[bank][prog].mid_amp - MainForm->Eq5Amp3->Max)/20.0);
    MainForm->Eq5Q3->Position = QuadtGT_Bank[bank][prog].mid_q;
    MainForm->Eq5Q3Val->Text = AnsiString(QuadtGT_Bank[bank][prog].mid_q);

    MainForm->Eq5Freq4->Position = QuadtGT_Bank[bank][prog].high_mid_freq;
    MainForm->Eq5Freq4Val->Text = AnsiString(QuadtGT_Bank[bank][prog].high_mid_freq);
    MainForm->Eq5Amp4->Position = -1 * (QuadtGT_Bank[bank][prog].high_mid_amp - MainForm->Eq5Amp4->Max);
    MainForm->Eq5Amp4Val->Text = AnsiString((double)(QuadtGT_Bank[bank][prog].high_mid_amp - MainForm->Eq5Amp4->Max)/20.0);
    MainForm->Eq5Q4->Position = QuadtGT_Bank[bank][prog].high_mid_q;
    MainForm->Eq5Q4Val->Text = AnsiString(QuadtGT_Bank[bank][prog].high_mid_q);

    MainForm->Eq5Freq5->Position = QuadtGT_Bank[bank][prog].high_freq;
    MainForm->Eq5Freq5Val->Text = AnsiString(QuadtGT_Bank[bank][prog].high_freq);
    MainForm->Eq5Amp5->Position = -1 * (QuadtGT_Bank[bank][prog].high_amp - MainForm->Eq5Amp5->Max);
    MainForm->Eq5Amp5Val->Text = AnsiString((double)(QuadtGT_Bank[bank][prog].high_amp - MainForm->Eq5Amp5->Max)/20.0);
  }
  else 
  {
    MainForm->QuadEq5->Visible=FALSE;
  }

  // Graphic Eq
  if (QuadtGT_Bank[bank][prog].config==CFG2_GEQ_DELAY) 
  {
    MainForm->QuadGraphEq->Visible=TRUE;

    MainForm->GEQPreset->ItemIndex =QuadtGT_Bank[bank][prog].preset;

    // TBD: If using preset, disable sliders and display preset amplitudes
   
    MainForm->GEQ_16Hz->Position =-1*QuadtGT_Bank[bank][prog].geq_16hz  ;
    MainForm->GEQ_32Hz->Position =-1*QuadtGT_Bank[bank][prog].geq_32hz  ;
    MainForm->GEQ_62Hz->Position =-1*QuadtGT_Bank[bank][prog].geq_62hz  ;
    MainForm->GEQ_126Hz->Position=-1*QuadtGT_Bank[bank][prog].geq_126hz ;
    MainForm->GEQ_250Hz->Position=-1*QuadtGT_Bank[bank][prog].geq_250hz ;
    MainForm->GEQ_500Hz->Position=-1*QuadtGT_Bank[bank][prog].geq_500hz ;
    MainForm->GEQ_1KHz->Position =-1*QuadtGT_Bank[bank][prog].geq_1khz  ;
    MainForm->GEQ_2KHz->Position =-1*QuadtGT_Bank[bank][prog].geq_2khz  ;
    MainForm->GEQ_4KHz->Position =-1*QuadtGT_Bank[bank][prog].geq_4khz  ;
    MainForm->GEQ_8KHz->Position =-1*QuadtGT_Bank[bank][prog].geq_8khz  ;
    MainForm->GEQ_16KHz->Position=-1*QuadtGT_Bank[bank][prog].geq_16khz ;

  }
  else
  {
    MainForm->QuadGraphEq->Visible=FALSE;
  }

}

//---------------------------------------------------------------------------
// Name        : QuadGT_Redraw_Mix
// Description : 
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Redraw_Mix(const UInt8 prog)
{
  UInt8 val;

  // Direct level
  MainForm->MixDirect->Position=MainForm->MixDirect->Max - QuadtGT_Bank[bank][prog].direct_level;
  MainForm->MixDirectVal->Text=AnsiString(QuadtGT_Bank[bank][prog].direct_level);

  // Master Fx Level
  if (QuadtGT_Bank[bank][prog].config != 7)
  {
    val=QuadtGT_Bank[bank][prog].master_effects_level;
    MainForm->MixMaster->Position=99-val;
    MainForm->MixMasterVal->Text=AnsiString(QuadtGT_Bank[bank][prog].master_effects_level);
    MainForm->Master->Visible=TRUE;
    MainForm->MixMaster->Visible=TRUE;
    MainForm->MixMasterVal->Visible=TRUE;
  }
  else
  {
    MainForm->Master->Visible=FALSE;
    MainForm->MixMaster->Visible=FALSE;
    MainForm->MixMasterVal->Visible=FALSE;
  }

  // Pre or Post Eq
  MainForm->PrePostEq->ItemIndex=QuadtGT_Bank[bank][prog].prepost_eq;
 
  QuadGT_Redraw_PrePostEq(prog);

  // Pitch Level
  if ( (QuadtGT_Bank[bank][prog].config == CFG0_EQ_PITCH_DELAY_REVERB) ||
       (QuadtGT_Bank[bank][prog].config == CFG3_5BANDEQ_PITCH_DELAY))
  {
    val=QuadtGT_Bank[bank][prog].pitch_level;
    MainForm->MixPitch->Position=99-val;
    MainForm->MixPitchVal->Text=AnsiString(val);

    MainForm->Pitch->Visible=TRUE;
    MainForm->MixPitch->Visible=TRUE;
    MainForm->MixPitchVal->Visible=TRUE;
  }
  else
  {
    MainForm->Pitch->Visible=FALSE;
    MainForm->MixPitch->Visible=FALSE;
    MainForm->MixPitchVal->Visible=FALSE;
  }

  // Delay Level
  val=QuadtGT_Bank[bank][prog].delay_level;
  MainForm->MixDelay->Position=99-val;
  MainForm->MixDelayVal->Text=AnsiString(val);

  // Reverb Level
  val=QuadtGT_Bank[bank][prog].reverb_level;
  MainForm->MixReverb->Position=99-val;
  MainForm->MixReverbVal->Text=AnsiString(val);

  val=QuadtGT_Bank[bank][prog].mix_mod;
  if (val == 0)
  {
    MainForm->MixModNone->Checked=TRUE;
    MainForm->ModDepth->Enabled=FALSE;
    MainForm->ModDepthVal->Enabled=FALSE;
    MainForm->Depth->Enabled=FALSE;
    MainForm->ModSpeed->Enabled=FALSE;
    MainForm->ModSpeedVal->Enabled=FALSE;
    MainForm->Speed->Enabled=FALSE;
  }
  else MainForm->MixModNone->Checked=FALSE;
  if (val == 1)
  {
    MainForm->MixModTrem->Checked=TRUE;
    MainForm->ModDepth->Enabled=TRUE;
    MainForm->ModDepthVal->Enabled=TRUE;
    MainForm->Depth->Enabled=TRUE;
    MainForm->ModSpeed->Enabled=TRUE;
    MainForm->ModSpeedVal->Enabled=TRUE;
    MainForm->Speed->Enabled=TRUE;
  }
  else MainForm->MixModTrem->Checked=FALSE;
  if (val == 2) 
  {
    MainForm->MixModPan->Checked=TRUE;
    MainForm->ModDepth->Enabled=TRUE;
    MainForm->ModDepthVal->Enabled=TRUE;
    MainForm->Depth->Enabled=TRUE;
    MainForm->ModSpeed->Enabled=TRUE;
    MainForm->ModSpeedVal->Enabled=TRUE;
    MainForm->Speed->Enabled=TRUE;
  }
  else MainForm->MixModPan->Checked=FALSE;

  // Mod Depth
  val=QuadtGT_Bank[bank][prog].pan_depth;
  MainForm->ModDepth->Position=99-val;
  MainForm->ModDepthVal->Text=AnsiString(val);

  // Mod Speed
  val=QuadtGT_Bank[bank][prog].pan_speed;
  MainForm->ModSpeed->Position=99-val;
  MainForm->ModSpeedVal->Text=AnsiString(val);
 
  // Leslie Level
  if (QuadtGT_Bank[bank][prog].config == CFG1_LESLIE_DELAY_REVERB)
  {
    val=QuadtGT_Bank[bank][prog].leslie_level;
    MainForm->MixLeslieRingModReson->Position=99-val;
    MainForm->MixLeslieRingModResonVal->Text=AnsiString(val);
    MainForm->MixLeslieRingModReson->Visible=TRUE;
    MainForm->LeslieRingModReson->Caption="Leslie";
    MainForm->LeslieRingModReson->Visible=TRUE;
    MainForm->MixLeslieRingModResonVal->Visible=TRUE;
  }

  // Ring Level
  else if (QuadtGT_Bank[bank][prog].config == CFG5_RINGMOD_DELAY_REVERB)
  {
    val=QuadtGT_Bank[bank][prog].ring_mod_level;
    MainForm->MixLeslieRingModReson->Position=99-val;
    MainForm->MixLeslieRingModResonVal->Text=AnsiString(val);
    MainForm->MixLeslieRingModReson->Visible=TRUE;
    MainForm->LeslieRingModReson->Caption="RingMod";
    MainForm->LeslieRingModReson->Visible=TRUE;
    MainForm->MixLeslieRingModResonVal->Visible=TRUE;
  }
  
  // Resonator Level
  else if (QuadtGT_Bank[bank][prog].config == CFG6_RESONATOR_DELAY_REVERB)
  {
    //val=QuadtGT_Bank[bank][prog].TBD;
    MainForm->MixLeslieRingModReson->Position=99-val;
    MainForm->MixLeslieRingModResonVal->Text=AnsiString(val);
    MainForm->MixLeslieRingModReson->Visible=TRUE;
    MainForm->LeslieRingModReson->Caption="Reson.";
    MainForm->LeslieRingModReson->Visible=TRUE;
    MainForm->MixLeslieRingModResonVal->Visible=TRUE;
  }

  else
  {
    MainForm->MixLeslieRingModReson->Visible=FALSE;
    MainForm->LeslieRingModReson->Visible=FALSE;
    MainForm->MixLeslieRingModResonVal->Visible=FALSE;
  }
}

//---------------------------------------------------------------------------
// Name        : QuadGT_Redraw_Mod
// Description : 
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Redraw_Mod(const UInt8 prog)
{
  UInt8 mod;

  if (QuadtGT_Bank[bank][prog].config != CFG7_SAMPLING)
  {
    mod = StrToInt(MainForm->ModNumber->Text)-1;
    MainForm->ModSource->Text=AnsiString(QuadtGT_Bank[bank][prog].mod_source[mod]);
    MainForm->ModTarget->ItemIndex=QuadtGT_Bank[bank][prog].mod_target[mod];
    RedrawHorizBarTextS8( MainForm->ModAmp,  MainForm->ModAmpVal,QuadtGT_Bank[bank][prog].mod_amp[mod]);

    // TBD: Replace numeric source field with a text list
    if (MainForm->ModSource->ItemIndex==MainForm->ModSource->Items->Count-1)
    {
      MainForm->ModSourceCtrl->Enabled=true;
    }
    else
    {
      MainForm->ModSourceCtrl->Enabled=false;
    }
    // TBD: Change list of possible targets based on current config
  }
}

//---------------------------------------------------------------------------
// Name        : QuadGT_Redraw_Preamp
// Description : 
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Redraw_Preamp(const UInt8 prog)
{
  UInt8 val;

  // Compression (0-7)
  MainForm->PreComp->Position = MainForm->PreComp->Max - QuadtGT_Bank[bank][prog].preamp.comp;
  MainForm->PreCompVal->Text = AnsiString(QuadtGT_Bank[bank][prog].preamp.comp);

  // Overdrive (0-7)
  MainForm->PreOd->Position = MainForm->PreOd->Max - QuadtGT_Bank[bank][prog].preamp.od;
  MainForm->PreOdVal->Text = AnsiString(QuadtGT_Bank[bank][prog].preamp.od);

  // Distortion (0-8)
  MainForm->PreDist->Position = MainForm->PreDist->Max - QuadtGT_Bank[bank][prog].preamp.dist;
  MainForm->PreDistVal->Text = AnsiString(QuadtGT_Bank[bank][prog].preamp.dist);

  // Tone (0-2 Flat, Presence, Bright)
  MainForm->PreTone->ItemIndex=QuadtGT_Bank[bank][prog].preamp.preamp_tone;

  // Bass Boost (0-1 On, Off)
  MainForm->BassBoost->Checked = QuadtGT_Bank[bank][prog].preamp.bass_boost == 1 ? TRUE : FALSE;

  // Cab sim (0-2 Off, Cab 1, Cab 2)
  MainForm->CabSim->ItemIndex=QuadtGT_Bank[bank][prog].preamp.cab_sim;
  
  // Effect Loop (0-1)
  MainForm->EffectLoopIn->Checked = QuadtGT_Bank[bank][prog].preamp.effect_loop == 1 ? TRUE : FALSE;

  // Noise Gate (0-17)
  MainForm->PreGate->Position=MainForm->PreGate->Max-QuadtGT_Bank[bank][prog].preamp.preamp_gate;
  MainForm->PreGateVal->Text = AnsiString(QuadtGT_Bank[bank][prog].preamp.preamp_gate);

  /* Preamp Output Level (0-99) */
  if (MainForm->QuadConfig->ItemIndex == CFG7_SAMPLING)
  {
    MainForm->PreOutLevel->Visible = FALSE;
    MainForm->PreOutVal->Visible = FALSE;
    MainForm->PreOutLabel->Visible = FALSE;
  }
  else
  {
    MainForm->PreOutLevel->Position = MainForm->PreOutLevel->Max-QuadtGT_Bank[bank][prog].preamp.preamp_out_level;
    MainForm->PreOutVal->Text = AnsiString(QuadtGT_Bank[bank][prog].preamp.preamp_out_level);
    MainForm->PreOutLevel->Visible = TRUE;
    MainForm->PreOutVal->Visible = TRUE;
    MainForm->PreOutLabel->Visible = TRUE;
  }

}

//---------------------------------------------------------------------------
// Name        : QuadGT_Redraw_Resonator
// Description : 
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Redraw_Resonator(const UInt8 prog)
{
  UInt8 resnum;

  if ((QuadtGT_Bank[bank][prog].config==CFG0_EQ_PITCH_DELAY_REVERB) && (QuadtGT_Bank[bank][prog].mode == EQMODE1_EQ_PLUS_RESONATOR))
  {
    // 2 Resonators
    MainForm->QuadResonator->Visible=TRUE;

    MainForm->ResUpDown->Max = 2;
    MainForm->ResDecayLabel->Visible = TRUE;
    MainForm->ResDecay->Visible = TRUE;
    MainForm->ResDecayVal->Visible = TRUE;
    MainForm->ResAmpLabel->Visible = TRUE;
    MainForm->ResAmp->Visible = TRUE;
    MainForm->ResAmpVal->Visible = TRUE;
  }
  else if ((QuadtGT_Bank[bank][prog].config == CFG3_5BANDEQ_PITCH_DELAY) && (QuadtGT_Bank[bank][prog].mode == EQMODE1_EQ_PLUS_RESONATOR))
  {
    // 5 Resonators
    MainForm->QuadResonator->Visible=TRUE;
    MainForm->ResUpDown->Max = 5;
  }
  else if (QuadtGT_Bank[bank][prog].config==CFG6_RESONATOR_DELAY_REVERB)
  {
    // 5 Resonators
    MainForm->QuadResonator->Visible=TRUE;

    MainForm->ResUpDown->Max = 5;
    MainForm->ResDecayLabel->Visible = FALSE;
    MainForm->ResDecay->Visible = FALSE;
    MainForm->ResDecayVal->Visible = FALSE;
    MainForm->ResAmpLabel->Visible = FALSE;
    MainForm->ResAmp->Visible = FALSE;
    MainForm->ResAmpVal->Visible = FALSE;

    RedrawHorizBarTextS8(MainForm->ResGlobalDecay, MainForm->ResGlobalDecayVal, QuadtGT_Bank[bank][prog].res_decay_all);
    MainForm->ResMidiGate->ItemIndex= QuadtGT_Bank[bank][prog].res_midi_gate;
  }
  else
  {
    MainForm->QuadResonator->Visible=FALSE;
  }

  if (MainForm->QuadResonator->Visible==TRUE)
  {
    resnum = (UInt8)StrToInt(MainForm->ResNumber->Text);
    RedrawHorizBarTextS8(MainForm->ResTune, MainForm->ResTuneVal, QuadtGT_Bank[bank][prog].res_tune[resnum -1]);
    RedrawHorizBarTextS8(MainForm->ResDecay, MainForm->ResDecayVal, QuadtGT_Bank[bank][prog].res_decay[resnum -1]);
    RedrawHorizBarTextS8(MainForm->ResAmp, MainForm->ResAmpVal, QuadtGT_Bank[bank][prog].res_amp[resnum -1]);

  }
}


#define PREPOSTEQ_PREAMP   (0)
#define PREPOSTEQ_EQ       (1)
//---------------------------------------------------------------------------
// Name        : QuadGT_Redraw_PrePostEq
// Description : 
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Redraw_PrePostEq(UInt8 prog)
{
  // Preamp Level
  if (MainForm->PrePostEq->ItemIndex==PREPOSTEQ_PREAMP)
  {
    MainForm->MixPreampEq->Position=99-QuadtGT_Bank[bank][prog].preamp_level;
    MainForm->MixPreampEq->Visible=TRUE;
    MainForm->MixPreampEqVal->Text=AnsiString(QuadtGT_Bank[bank][prog].preamp_level);
    MainForm->PreAmpEq->Visible=TRUE;
    MainForm->PreAmpEq->Caption="Preamp";
  }
  // Eq Level
  else
  {
    MainForm->MixPreampEq->Position=99-QuadtGT_Bank[bank][prog].eq_level;
    MainForm->MixPreampEq->Visible=TRUE;
    MainForm->MixPreampEqVal->Text=AnsiString(QuadtGT_Bank[bank][prog].eq_level);
    MainForm->PreAmpEq->Visible=TRUE;
    MainForm->PreAmpEq->Caption="Eq";
  }
}

//---------------------------------------------------------------------------
// Name        : QuadParamChange
// Description : Handles the user changing a parameter via the on screen
//               controls.
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void __fastcall TMainForm::QuadParamChange(TObject *Sender)
{
  UInt8 prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  UInt8 tap = StrToInt(MainForm->TapNumber->Text)-1;

  // Patch number and name
  if (Sender == MainForm->QuadConfig)
  {
    QuadtGT_Bank[bank][prog].config=MainForm->QuadConfig->ItemIndex;
    QuadGT_Redraw_Patch(bank, prog);
  }
  else if (Sender == MainForm->EditQuadPatchName)
  {
    strncpy(QuadtGT_Bank[bank][prog].name,MainForm->EditQuadPatchName->Text.c_str(),NAME_LENGTH);
  }

  // Pre-amp parameters
  else if (Sender == MainForm->PreComp) VertBarChangeU8((TTrackBar *)Sender, MainForm->PreCompVal, &QuadtGT_Bank[bank][prog].preamp.comp);
  else if (Sender == MainForm->PreOd)   VertBarChangeU8((TTrackBar *)Sender, MainForm->PreOdVal, &QuadtGT_Bank[bank][prog].preamp.od);
  else if (Sender == MainForm->PreDist) VertBarChangeU8((TTrackBar *)Sender, MainForm->PreDistVal, &QuadtGT_Bank[bank][prog].preamp.dist);
  else if (Sender == MainForm->PreGate) VertBarChangeU8((TTrackBar *)Sender, MainForm->PreGateVal, &QuadtGT_Bank[bank][prog].preamp.preamp_gate);
  else if (Sender == MainForm->PreOutLevel) VertBarChangeU8((TTrackBar *)Sender, MainForm->PreOutVal, &QuadtGT_Bank[bank][prog].preamp.preamp_out_level);

  // 3 Band Eq
  // TBD: EqFreq1 EqFreq2 EqQ2 EqFreq3
  else if (Sender == MainForm->Eq3Mode)
  {
    QuadtGT_Bank[bank][prog].mode = MainForm->Eq3Mode->ItemIndex;
    QuadGT_Redraw_Eq(prog);
    QuadGT_Redraw_Resonator(prog);
  }
  else if (Sender == MainForm->EqAmp1) EqBarChange(MainForm->EqAmp1, MainForm->Eq3Amp1Val, &QuadtGT_Bank[bank][prog].low_amp);
  else if (Sender == MainForm->EqAmp2) EqBarChange(MainForm->EqAmp2, MainForm->Eq3Amp2Val, &QuadtGT_Bank[bank][prog].mid_amp);
  else if (Sender == MainForm->EqAmp3) EqBarChange(MainForm->EqAmp3, MainForm->Eq3Amp3Val, &QuadtGT_Bank[bank][prog].high_amp);

  else if (Sender == MainForm->EqFreq1) HorizBarChange(MainForm->EqFreq1, MainForm->EqFreq1Val, &QuadtGT_Bank[bank][prog].low_freq);
  else if (Sender == MainForm->EqFreq2) HorizBarChange(MainForm->EqFreq2, MainForm->EqFreq2Val, &QuadtGT_Bank[bank][prog].mid_freq);
  else if (Sender == MainForm->EqFreq3) HorizBarChange(MainForm->EqFreq3, MainForm->EqFreq3Val, &QuadtGT_Bank[bank][prog].high_freq);

  else if (Sender == MainForm->EqQ2) HorizBarChangeU8(MainForm->EqQ2, MainForm->EqQ2Val, &QuadtGT_Bank[bank][prog].mid_q);

  // 5 Band Eq
  else if (Sender == MainForm->Eq5Mode)
  {
    QuadtGT_Bank[bank][prog].mode = MainForm->Eq5Mode->ItemIndex;
    QuadGT_Redraw_Eq(prog);
    QuadGT_Redraw_Resonator(prog);
  }
  else if (Sender == MainForm->Eq5Amp1) EqBarChange(MainForm->Eq5Amp1, MainForm->Eq5Amp1Val, &QuadtGT_Bank[bank][prog].low_amp);
  else if (Sender == MainForm->Eq5Amp2) EqBarChange(MainForm->Eq5Amp2, MainForm->Eq5Amp2Val, &QuadtGT_Bank[bank][prog].low_mid_amp);
  else if (Sender == MainForm->Eq5Amp3) EqBarChange(MainForm->Eq5Amp3, MainForm->Eq5Amp3Val, &QuadtGT_Bank[bank][prog].mid_amp);
  else if (Sender == MainForm->Eq5Amp4) EqBarChange(MainForm->Eq5Amp4, MainForm->Eq5Amp4Val, &QuadtGT_Bank[bank][prog].high_mid_amp);
  else if (Sender == MainForm->Eq5Amp5) EqBarChange(MainForm->Eq5Amp5, MainForm->Eq5Amp5Val, &QuadtGT_Bank[bank][prog].high_amp);

  else if (Sender == MainForm->Eq5Freq1) HorizBarChange(MainForm->Eq5Freq1, MainForm->Eq5Freq1Val, &QuadtGT_Bank[bank][prog].low_freq);
  else if (Sender == MainForm->Eq5Freq2) HorizBarChange(MainForm->Eq5Freq2, MainForm->Eq5Freq2Val, &QuadtGT_Bank[bank][prog].low_mid_freq);
  else if (Sender == MainForm->Eq5Freq3) HorizBarChange(MainForm->Eq5Freq3, MainForm->Eq5Freq3Val, &QuadtGT_Bank[bank][prog].mid_freq);
  else if (Sender == MainForm->Eq5Freq4) HorizBarChange(MainForm->Eq5Freq4, MainForm->Eq5Freq4Val, &QuadtGT_Bank[bank][prog].high_mid_freq);
  else if (Sender == MainForm->Eq5Freq5) HorizBarChange(MainForm->Eq5Freq5, MainForm->Eq5Freq5Val, &QuadtGT_Bank[bank][prog].high_freq);

  else if (Sender == MainForm->Eq5Q2) HorizBarChangeU8(MainForm->Eq5Q2, MainForm->Eq5Q2Val, &QuadtGT_Bank[bank][prog].low_mid_q);
  else if (Sender == MainForm->Eq5Q3) HorizBarChangeU8(MainForm->Eq5Q3, MainForm->Eq5Q3Val, &QuadtGT_Bank[bank][prog].mid_q);
  else if (Sender == MainForm->Eq5Q4) HorizBarChangeU8(MainForm->Eq5Q4, MainForm->Eq5Q4Val, &QuadtGT_Bank[bank][prog].high_mid_q);

  // Mix parameters
  else if (Sender == MainForm->MixDirect) VertBarChangeU8((TTrackBar *)Sender, MainForm->MixDirectVal, &QuadtGT_Bank[bank][prog].direct_level);
  else if (Sender == MainForm->MixMaster) VertBarChangeU8((TTrackBar *)Sender, MainForm->MixMasterVal, &QuadtGT_Bank[bank][prog].master_effects_level);
  else if (Sender == MainForm->MixPitch)  VertBarChangeU8((TTrackBar *)Sender, MainForm->MixPitchVal,  &QuadtGT_Bank[bank][prog].pitch_level);
  else if (Sender == MainForm->PrePostEq) 
  {
    QuadtGT_Bank[bank][prog].prepost_eq=MainForm->PrePostEq->ItemIndex;
    QuadGT_Redraw_PrePostEq(prog);
  }
  else if (Sender == MainForm->MixPreampEq) 
  {
    if (QuadtGT_Bank[bank][prog].prepost_eq==0) VertBarChangeU8((TTrackBar *)Sender, MainForm->MixPreampEqVal, &QuadtGT_Bank[bank][prog].preamp_level);
    else                                  VertBarChangeU8((TTrackBar *)Sender, MainForm->MixPreampEqVal, &QuadtGT_Bank[bank][prog].eq_level);
  }
  else if (Sender == MainForm->MixDelay) VertBarChangeU8((TTrackBar *)Sender, MainForm->MixDelayVal, &QuadtGT_Bank[bank][prog].delay_level);
  else if (Sender == MainForm->MixReverb) VertBarChangeU8((TTrackBar *)Sender, MainForm->MixReverbVal, &QuadtGT_Bank[bank][prog].reverb_level);
  else if (Sender == MainForm->MixModNone) 
  {
    QuadtGT_Bank[bank][prog].mix_mod=0;
    MainForm->ModDepth->Enabled=FALSE;
    MainForm->ModDepthVal->Enabled=FALSE;
    MainForm->Depth->Enabled=FALSE;
    MainForm->ModSpeed->Enabled=FALSE;
    MainForm->ModSpeedVal->Enabled=FALSE;
    MainForm->Speed->Enabled=FALSE;
  }
  else if (Sender == MainForm->MixModTrem) 
  {
    QuadtGT_Bank[bank][prog].mix_mod=1;
    MainForm->ModDepth->Enabled=TRUE;
    MainForm->ModDepthVal->Enabled=TRUE;
    MainForm->Depth->Enabled=TRUE;
    MainForm->ModSpeed->Enabled=TRUE;
    MainForm->ModSpeedVal->Enabled=TRUE;
    MainForm->Speed->Enabled=TRUE;
  }
  else if (Sender == MainForm->MixModPan) 
  {
    QuadtGT_Bank[bank][prog].mix_mod=2;
    MainForm->ModDepth->Enabled=TRUE;
    MainForm->ModDepthVal->Enabled=TRUE;
    MainForm->Depth->Enabled=TRUE;
    MainForm->ModSpeed->Enabled=TRUE;
    MainForm->ModSpeedVal->Enabled=TRUE;
    MainForm->Speed->Enabled=TRUE;
  }
  else if (Sender == MainForm->ModSpeed) VertBarChangeU8((TTrackBar *)Sender, MainForm->ModSpeedVal, &QuadtGT_Bank[bank][prog].pan_speed);
  else if (Sender == MainForm->ModDepth) VertBarChangeU8((TTrackBar *)Sender, MainForm->ModDepthVal, &QuadtGT_Bank[bank][prog].pan_depth);
  else if (Sender == MainForm->MixLeslieRingModReson)
  {
    if (QuadtGT_Bank[bank][prog].config == CFG1_LESLIE_DELAY_REVERB)
      VertBarChangeU8((TTrackBar *)Sender, MainForm->MixLeslieRingModResonVal, &QuadtGT_Bank[bank][prog].leslie_level);
    else if (QuadtGT_Bank[bank][prog].config == CFG5_RINGMOD_DELAY_REVERB)
      VertBarChangeU8((TTrackBar *)Sender, MainForm->MixLeslieRingModResonVal, &QuadtGT_Bank[bank][prog].ring_mod_level);
    //else if (QuadtGT_Bank[bank][prog].config == CFG6_RESONATOR_DELAY_REVERB)
    //  VertBarChangeU8((TTrackBar *)Sender, MainForm->MixLeslieRingModResonVal, &QuadtGT_Bank[bank][prog].TBD);

  }

  // Pitch parameters
  else if (Sender == MainForm->PitchMode) { QuadtGT_Bank[bank][prog].pitch.pitch_mode=MainForm->PitchMode->ItemIndex; QuadGT_Redraw_Pitch(prog);}
  else if (Sender == MainForm->PitchInput){ QuadtGT_Bank[bank][prog].pitch.pitch_input=MainForm->PitchInput->ItemIndex;  QuadGT_Redraw_Pitch(prog);}
  else if (Sender == MainForm->PitchWave) { QuadtGT_Bank[bank][prog].pitch.lfo_waveform=MainForm->PitchWave->ItemIndex; QuadGT_Redraw_Pitch(prog);}
  else if (Sender == MainForm->PitchSpeed) VertBarChangeU8((TTrackBar *)Sender, MainForm->PitchSpeedVal, &QuadtGT_Bank[bank][prog].pitch.lfo_speed);
  else if (Sender == MainForm->PitchDepth) VertBarChangeU8((TTrackBar *)Sender, MainForm->PitchDepthVal, &QuadtGT_Bank[bank][prog].pitch.lfo_depth);
  else if (Sender == MainForm->PitchFeedback) VertBarChangeU8((TTrackBar *)Sender, MainForm->PitchFeedbackVal, &QuadtGT_Bank[bank][prog].pitch.pitch_feedback);

  else if (Sender == MainForm->ChorusSpeed) VertBarChangeOffsetU8((TTrackBar *)Sender, MainForm->ChorusSpeedVal, &QuadtGT_Bank[bank][prog].pitch.lfo_speed, 1);
  else if (Sender == MainForm->ChorusDepth) VertBarChangeOffsetU8((TTrackBar *)Sender, MainForm->ChorusDepthVal, &QuadtGT_Bank[bank][prog].pitch.lfo_depth, 1);

  // Midi Modulation Parameters
  else if (Sender == MainForm->ModNumber) QuadGT_Redraw_Mod(prog);
  else if (Sender == MainForm->ModSource) QuadGT_Redraw_Mod(prog);
  else if (Sender == MainForm->ModAmp)
  {
    UInt8 mod = StrToInt(MainForm->ModNumber->Text)-1;
    HorizBarChangeS8((TTrackBar *)Sender, MainForm->ModAmpVal, &QuadtGT_Bank[bank][prog].mod_amp[mod]);
  }

  // Reverb parameters
  else if (Sender == MainForm->ReverbPreDelay)  VertBarChangeOffsetU8((TTrackBar *)Sender,  MainForm->ReverbPreDelayVal,       &QuadtGT_Bank[bank][prog].reverb_predelay, 1);
  else if (Sender == MainForm->ReverbDecay)     VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbDecayVal,          &QuadtGT_Bank[bank][prog].reverb_decay);
  else if (Sender == MainForm->ReverbLowDecay)  VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbLowDecayVal,       &QuadtGT_Bank[bank][prog].reverb_low_decay);
  else if (Sender == MainForm->ReverbHighDecay) VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbHighDecayVal,      &QuadtGT_Bank[bank][prog].reverb_high_decay);
  else if (Sender == MainForm->ReverbDensity)   VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbDensityVal,        &QuadtGT_Bank[bank][prog].reverb_density);
  else if (Sender == MainForm->ReverbDiffusion) VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbDiffusionVal,      &QuadtGT_Bank[bank][prog].reverb_diffusion);
  else if (Sender == MainForm->ReverbGateHold)  VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbGateHoldVal,       &QuadtGT_Bank[bank][prog].reverb_gate_hold);
  else if (Sender == MainForm->ReverbGateRelease)  VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbGateReleaseVal, &QuadtGT_Bank[bank][prog].reverb_gate_release);
  else if (Sender == MainForm->ReverbGateLevel)  VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbGateLevelVal,     &QuadtGT_Bank[bank][prog].reverb_gated_level);
  else if (Sender == MainForm->ReverbInMix)     HorizBarChangeS8((TTrackBar *)Sender, MainForm->ReverbInMixVal,          &QuadtGT_Bank[bank][prog].reverb_input_mix);
  else if (Sender == MainForm->ReverbPreMix)    HorizBarChangeS8((TTrackBar *)Sender, MainForm->ReverbPreMixVal,         &QuadtGT_Bank[bank][prog].reverb_predelay_mix);

  // Delay Parameters
  else if (Sender == MainForm->TapDelay) VertBarChangeU8((TTrackBar *)Sender, MainForm->TapDelayVal, &QuadtGT_Bank[bank][prog].tap.tap_delay[tap]);
  else if (Sender == MainForm->TapPan) VertBarChangeU8((TTrackBar *)Sender, MainForm->TapPanVal, &QuadtGT_Bank[bank][prog].tap.tap_pan[tap]);
  else if (Sender == MainForm->TapVol) VertBarChangeU8((TTrackBar *)Sender, MainForm->TapVolVal, &QuadtGT_Bank[bank][prog].tap.tap_volume[tap]);
  else if (Sender == MainForm->TapFeedback) VertBarChangeU8((TTrackBar *)Sender, MainForm->TapFeedbackVal, &QuadtGT_Bank[bank][prog].tap.tap_feedback[tap]);

  else if (Sender == MainForm->DelayInMix)     VertBarChangeS8((TTrackBar *)Sender, MainForm->DelayInMixVal,     &QuadtGT_Bank[bank][prog].delay_input_mix);
  else if (Sender == MainForm->DelayLeft)      VertBarChangeU16((TTrackBar *)Sender, MainForm->DelayLeftVal,      &QuadtGT_Bank[bank][prog].delay_left);
  else if (Sender == MainForm->DelayLeftFBack) VertBarChangeU8((TTrackBar *)Sender, MainForm->DelayLeftFBackVal, &QuadtGT_Bank[bank][prog].delay_left_feedback);
  else if (Sender == MainForm->DelayRight)     VertBarChangeU16((TTrackBar *)Sender, MainForm->DelayRightVal,     &QuadtGT_Bank[bank][prog].delay_right);
  else if (Sender == MainForm->DelayRighFBack) VertBarChangeU8((TTrackBar *)Sender, MainForm->DelayRightFBackVal, &QuadtGT_Bank[bank][prog].delay_right_feedback);
  else if (Sender == MainForm->DelayInput)
  {
    QuadtGT_Bank[bank][prog].delay_input=MainForm->DelayInput->ItemIndex;
    QuadGT_Redraw_Delay(prog);
  }

  // Ring Modulation parameters
  else if (Sender == MainForm->RingModSS)     VertBarChangeU16((TTrackBar *)Sender, MainForm->RingModSSVal,    &QuadtGT_Bank[bank][prog].ring_mod_shift);
  else if (Sender == MainForm->RingModInMix)  VertBarChangeS8((TTrackBar *)Sender, MainForm->RingModInMixVal,  &QuadtGT_Bank[bank][prog].ring_mod_in_mix);
  else if (Sender == MainForm->RingModOutMix) VertBarChangeS8((TTrackBar *)Sender, MainForm->RingModOutMixVal, &QuadtGT_Bank[bank][prog].ring_mod_out_mix);
}

void __fastcall TMainForm::DelayModeClick(TObject *Sender)
{
  //TBD: Move this logic into QuadParamChange
  UInt8 prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  QuadtGT_Bank[bank][prog].delay_mode=MainForm->DelayMode->ItemIndex;
  QuadGT_Redraw_Delay(prog);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Name     : QuadGT_Convert_Data_From_Internal
// Description : Convert internal data structure to raw Quadraverb GT data.
//---------------------------------------------------------------------------
UInt32 QuadGT_Convert_Data_From_Internal(UInt8 prog, UInt8* data)
{
  if (prog >= QUAD_NUM_PATCH) return 1;

  // Resonator parameters, alternative one
  if ((QuadtGT_Bank[bank][prog].config == CFG0_EQ_PITCH_DELAY_REVERB) &&
      (QuadtGT_Bank[bank][prog].mode == EQMODE1_EQ_PLUS_RESONATOR))
  {
    data[RES1_TUNE_IDX]  = QuadtGT_Bank[bank][prog].res_tune[0] + RES_TUNE_OFFSET;
    data[RES1_DECAY_IDX] = QuadtGT_Bank[bank][prog].res_decay[0];
    data[RES1_AMP_IDX]   = QuadtGT_Bank[bank][prog].res_amp[0];

    data[RES2_TUNE_IDX]  = QuadtGT_Bank[bank][prog].res_tune[1] + RES_TUNE_OFFSET;
    data[RES2_DECAY_IDX] = QuadtGT_Bank[bank][prog].res_decay[1];
    data[RES2_AMP_IDX]   = QuadtGT_Bank[bank][prog].res_amp[1];
  }
  // Graphic Eq
  else if (QuadtGT_Bank[bank][prog].config == CFG2_GEQ_DELAY)
  {
    data[GEQ_16HZ_IDX] = QuadtGT_Bank[bank][prog].geq_16hz + GEQ_OFFSET;
    data[GEQ_32HZ_IDX] = QuadtGT_Bank[bank][prog].geq_32hz + GEQ_OFFSET;
    data[GEQ_62HZ_IDX] = QuadtGT_Bank[bank][prog].geq_62hz + GEQ_OFFSET;
                         
    data[GEQ_4KHZ_IDX] = QuadtGT_Bank[bank][prog].geq_4khz + GEQ_OFFSET;
    data[GEQ_8KHZ_IDX] = QuadtGT_Bank[bank][prog].geq_8khz + GEQ_OFFSET;
  }
  // 3 and 5 Band Eq
  else
  {
    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].low_freq, &data[LOW_EQ_FREQ_IDX]);
    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].low_amp, &data[LOW_EQ_AMP_IDX]);

    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].high_freq, &data[HIGH_EQ_FREQ_IDX]);
  }

  if (QuadtGT_Bank[bank][prog].config==CFG2_GEQ_DELAY)
  {
    data[GEQ_126HZ_IDX] = QuadtGT_Bank[bank][prog].geq_126hz + GEQ_OFFSET;
    data[GEQ_250HZ_IDX] = QuadtGT_Bank[bank][prog].geq_250hz + GEQ_OFFSET;
    data[GEQ_500HZ_IDX] = QuadtGT_Bank[bank][prog].geq_500hz + GEQ_OFFSET;
    data[GEQ_1KHZ_IDX]  = QuadtGT_Bank[bank][prog].geq_1khz  + GEQ_OFFSET;
    data[GEQ_2KHZ_IDX]  = QuadtGT_Bank[bank][prog].geq_2khz  + GEQ_OFFSET;
                        
    data[GEQ_16KHZ_IDX] = QuadtGT_Bank[bank][prog].geq_16khz + GEQ_OFFSET;
  }
  else
  {
    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].mid_freq, &data[MID_EQ_FREQ_IDX]);
    data[MID_EQ_BW_IDX] = QuadtGT_Bank[bank][prog].mid_q;
    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].mid_amp, &data[MID_EQ_AMP_IDX]);

    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].high_amp, &data[HIGH_EQ_AMP_IDX]);
  }


  // Type = 3 Eq and 5 Resonators
  if ((QuadtGT_Bank[bank][prog].config == CFG3_5BANDEQ_PITCH_DELAY) && 
      (QuadtGT_Bank[bank][prog].mode == EQMODE1_EQ_PLUS_RESONATOR))
  {
    data[RES1_TUNE_IDX_B]  = QuadtGT_Bank[bank][prog].res_tune[0];
    data[RES1_DECAY_IDX_B] = QuadtGT_Bank[bank][prog].res_decay[0];

    data[RES2_TUNE_IDX_B]  = QuadtGT_Bank[bank][prog].res_tune[1];
    data[RES2_DECAY_IDX_B] = QuadtGT_Bank[bank][prog].res_decay[1];

    data[RES3_TUNE_IDX]  = QuadtGT_Bank[bank][prog].res_tune[2];
    data[RES3_DECAY_IDX] = QuadtGT_Bank[bank][prog].res_decay[2];

    data[RES4_TUNE_IDX]  = QuadtGT_Bank[bank][prog].res_tune[3];
    data[RES4_DECAY_IDX] = QuadtGT_Bank[bank][prog].res_decay[3];

    data[RES5_TUNE_IDX]  = QuadtGT_Bank[bank][prog].res_tune[4];
    data[RES5_DECAY_IDX] = QuadtGT_Bank[bank][prog].res_decay[4];
  }
  // Otherwise, assume 5 band Eq
  else
  {
    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].low_mid_freq, &data[LOW_MID_EQ_FREQ_IDX]);
    data[LOW_MID_EQ_BW_IDX] = QuadtGT_Bank[bank][prog].low_mid_q;
    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].low_mid_amp, &data[LOW_MID_EQ_AMP_IDX]);
    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].high_mid_freq, &data[HIGH_MID_EQ_FREQ_IDX]);
    data[HIGH_MID_EQ_BW_IDX] = QuadtGT_Bank[bank][prog].high_mid_q;
    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].high_mid_amp, &data[HIGH_MID_EQ_AMP_IDX]);
  }

  // Select Sampling or Tap 1
  if (QuadtGT_Bank[bank][prog].config == CFG7_SAMPLING) 
  {
    data[SAMPLE_START_IDX]           = QuadtGT_Bank[bank][prog].sample.start;
    data[SAMPLE_LENGTH_IDX]          = QuadtGT_Bank[bank][prog].sample.length;
    data[SAMPLE_PLAYBACK_MODE_IDX]   = QuadtGT_Bank[bank][prog].sample.playback_mode;
    data[SAMPLE_PITCH_IDX]           = QuadtGT_Bank[bank][prog].sample.pitch;
    data[SAMPLE_REC_AUDIO_TRIG_IDX]  = QuadtGT_Bank[bank][prog].sample.rec_audio_trigger;
    data[SAMPLE_MIDI_TRIG_IDX]       = QuadtGT_Bank[bank][prog].sample.midi_trigger;
    data[SAMPLE_MIDI_BASE_NOTE_IDX]  = QuadtGT_Bank[bank][prog].sample.midi_base_note;
    data[SAMPLE_LOW_MIDI_NOTE_IDX]   = QuadtGT_Bank[bank][prog].sample.low_midi_note;
    data[SAMPLE_HIGH_MIDI_NOTE_IDX]  = QuadtGT_Bank[bank][prog].sample.high_midi_note;
  }
  else 
  {
    if (QuadtGT_Bank[bank][prog].config == CFG1_LESLIE_DELAY_REVERB)
    {
      data[LESLIE_HIGH_ROTOR_LEVEL_IDX] = QuadtGT_Bank[bank][prog].leslie_high_rotor_level;
      data[LESLIE_MOTOR_IDX]            = QuadtGT_Bank[bank][prog].leslie_motor;
      data[LESLIE_SPEED_IDX]            = QuadtGT_Bank[bank][prog].leslie_speed;
    }
    else
    {
      QuadGT_Encode_16Bit_Split(QuadtGT_Bank[bank][prog].tap.tap_delay[0], &data[TAP1_DELAY_MSB_IDX],
                                                               &data[TAP1_DELAY_LSB_IDX]);
      QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].tap.tap_delay[1], &data[TAP2_DELAY_IDX]);
    }

    data[TAP1_VOLUME_IDX]   = QuadtGT_Bank[bank][prog].tap.tap_volume[0];
    data[TAP1_PAN_IDX]      = QuadtGT_Bank[bank][prog].tap.tap_pan[0];

    data[DETUNE_AMOUNT_IDX] = QuadtGT_Bank[bank][prog].pitch.detune_amount;

    data[TAP2_VOLUME_IDX]   = QuadtGT_Bank[bank][prog].tap.tap_volume[1];
    data[TAP3_FEEDBACK_IDX] = QuadtGT_Bank[bank][prog].tap.tap_feedback[2];

    if (QuadtGT_Bank[bank][prog].config == CFG3_5BANDEQ_PITCH_DELAY)
    {
      QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].tap.tap_delay[3], &data[TAP4_DELAY_IDX]);
    }
    else
    {
      data[REVERB_MODE_IDX]   = QuadtGT_Bank[bank][prog].reverb_mode;
    }
  }

  if (QuadtGT_Bank[bank][prog].config == CFG1_LESLIE_DELAY_REVERB)
  {
    data[LESLIE_SEPARATION_IDX]       = QuadtGT_Bank[bank][prog].leslie_seperation;
  }
  else
  {
    data[TAP1_FEEDBACK_IDX] = QuadtGT_Bank[bank][prog].tap.tap_feedback[0];
  }

  data[TAP4_VOLUME_IDX]   = QuadtGT_Bank[bank][prog].tap.tap_volume[3];

  if (QuadtGT_Bank[bank][prog].config == CFG3_5BANDEQ_PITCH_DELAY)
  {
    data[TAP4_PAN_IDX]      = QuadtGT_Bank[bank][prog].tap.tap_pan[3];
    data[TAP4_FEEDBACK_IDX] = QuadtGT_Bank[bank][prog].tap.tap_feedback[3];

    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].tap.tap_delay[4], &data[TAP5_DELAY_IDX]);
    data[TAP5_VOLUME_IDX]   = QuadtGT_Bank[bank][prog].tap.tap_volume[4];
    data[TAP5_PAN_IDX]      = QuadtGT_Bank[bank][prog].tap.tap_pan[4];
    data[TAP5_FEEDBACK_IDX] = QuadtGT_Bank[bank][prog].tap.tap_feedback[4];

    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].tap.tap_delay[5], &data[TAP6_DELAY_IDX]);
    data[TAP6_VOLUME_IDX]   = QuadtGT_Bank[bank][prog].tap.tap_volume[5];
    data[TAP6_PAN_IDX]      = QuadtGT_Bank[bank][prog].tap.tap_pan[5];
    data[TAP6_FEEDBACK_IDX] = QuadtGT_Bank[bank][prog].tap.tap_feedback[5];

    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].tap.tap_delay[6], &data[TAP7_DELAY_IDX]);
    data[TAP7_VOLUME_IDX]   = QuadtGT_Bank[bank][prog].tap.tap_volume[6];
    data[TAP7_PAN_IDX]      = QuadtGT_Bank[bank][prog].tap.tap_pan[6];
  }
  else
  {
    data[REVERB_INPUT_1_IDX]=     QuadtGT_Bank[bank][prog].reverb_input_1	;
    data[REVERB_INPUT_2_IDX]=     QuadtGT_Bank[bank][prog].reverb_input_2	;
    data[REVERB_INPUT_MIX_IDX]=   QuadtGT_Bank[bank][prog].reverb_input_mix+99;
    data[REVERB_PREDELAY_IDX]=    QuadtGT_Bank[bank][prog].reverb_predelay	;
    data[REVERB_PREDELAY_MIX_IDX]=QuadtGT_Bank[bank][prog].reverb_predelay_mix+99;
    data[REVERB_DECAY_IDX]=       QuadtGT_Bank[bank][prog].reverb_decay	;
    data[REVERB_DIFFUSION_IDX]=   QuadtGT_Bank[bank][prog].reverb_diffusion	;
    data[REVERB_LOW_DECAY_IDX]=   QuadtGT_Bank[bank][prog].reverb_low_decay	;
    data[REVERB_HIGH_DECAY_IDX]=  QuadtGT_Bank[bank][prog].reverb_high_decay	;
    data[REVERB_DENSITY_IDX]=     QuadtGT_Bank[bank][prog].reverb_density	;
    data[REVERB_GATE_IDX]=        QuadtGT_Bank[bank][prog].reverb_gate	;
    data[REVERB_GATE_HOLD_IDX]=   QuadtGT_Bank[bank][prog].reverb_gate_hold	;
    data[REVERB_GATE_RELEASE_IDX]=QuadtGT_Bank[bank][prog].reverb_gate_release;
    data[REVERB_GATED_LEVEL_IDX]= QuadtGT_Bank[bank][prog].reverb_gated_level	;

    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].ring_mod_shift, &data[RING_MOD_SHIFT_IDX]);
  }

   data[MULTITAP_MASTER_FEEDBACK_IDX] = QuadtGT_Bank[bank][prog].multitap_master_feedback;
   data[MULTITAP_NUMBER_IDX]          = QuadtGT_Bank[bank][prog].multitap_number;

  //-------------------------------------------------------------------------
  // Pitch Parameters (0x1A - 
  //-------------------------------------------------------------------------
  data[PITCH_MODE_IDX]   = QuadtGT_Bank[bank][prog].pitch.pitch_mode;
  data[PITCH_FEEDBACK_IDX] =  QuadtGT_Bank[bank][prog].pitch.pitch_feedback;

  data[PITCH_INPUT_IDX]  = QuadtGT_Bank[bank][prog].pitch.pitch_input & BIT0;
  data[RES4_AMP_IDX]    |= (QuadtGT_Bank[bank][prog].res_amp[3] << 1) & BITS1to7;

  data[LFO_WAVEFORM_IDX] = QuadtGT_Bank[bank][prog].pitch.lfo_waveform & BIT0;
  data[EQ_PRESET_IDX]   |= (QuadtGT_Bank[bank][prog].preset << 1) & BITS1to7;

  data[LFO_SPEED_IDX] = QuadtGT_Bank[bank][prog].pitch.lfo_speed;

  if (QuadtGT_Bank[bank][prog].config == CFG6_RESONATOR_DELAY_REVERB) 
  {
    data[RES_DECAY_IDX]     = QuadtGT_Bank[bank][prog].res_decay_all;
    data[RES_MIDI_GATE_IDX] = QuadtGT_Bank[bank][prog].res_midi_gate;

    data[RES1_PITCH_IDX] = QuadtGT_Bank[bank][prog].res_pitch[0];
    data[RES2_PITCH_IDX] = QuadtGT_Bank[bank][prog].res_pitch[1];
    data[RES3_PITCH_IDX] = QuadtGT_Bank[bank][prog].res_pitch[2];
    data[RES4_PITCH_IDX] = QuadtGT_Bank[bank][prog].res_pitch[3];
    data[RES5_PITCH_IDX] = QuadtGT_Bank[bank][prog].res_pitch[4];
  }
  else
  {
    data[LFO_DEPTH_IDX] = QuadtGT_Bank[bank][prog].pitch.lfo_depth;
    data[TRIGGER_FLANGE_IDX] = QuadtGT_Bank[bank][prog].pitch.trigger_flange;

    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].tap.tap_delay[7], &data[TAP8_DELAY_IDX]);
    data[TAP8_VOLUME_IDX]  = QuadtGT_Bank[bank][prog].tap.tap_volume[7];
    data[TAP8_PAN_IDX]     = QuadtGT_Bank[bank][prog].tap.tap_pan[7];
    data[TAP8_FEEDBACK_IDX]= QuadtGT_Bank[bank][prog].tap.tap_feedback[7];
  }

  //-------------------------------------------------------------------------
  // Delay Parameters (0x27 - 0x31)
  //-------------------------------------------------------------------------
  data[DELAY_MODE_IDX]   = QuadtGT_Bank[bank][prog].delay_mode;

  data[DELAY_INPUT_IDX]  = QuadtGT_Bank[bank][prog].delay_input & BIT0;
  data[RES5_AMP_IDX]    |= (QuadtGT_Bank[bank][prog].res_amp[4] << 1) & BITS1to7;

  data[DELAY_INPUT_MIX_IDX] = QuadtGT_Bank[bank][prog].delay_input_mix;

  if ((QuadtGT_Bank[bank][prog].delay_mode == 0) ||
      (QuadtGT_Bank[bank][prog].delay_mode == 1) ||
      (QuadtGT_Bank[bank][prog].delay_mode == 2))
  {
    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].delay_left, &data[DELAY_LEFT_IDX]);
    data[DELAY_LEFT_FEEDBACK_IDX] = QuadtGT_Bank[bank][prog].delay_left_feedback;

    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].delay_right, &data[DELAY_RIGHT_IDX]);
    data[DELAY_RIGHT_FEEDBACK_IDX] = QuadtGT_Bank[bank][prog].delay_right_feedback;
  }
  else if (QuadtGT_Bank[bank][prog].delay_mode == 3)
  {
    data[TAP2_PAN_IDX]     = QuadtGT_Bank[bank][prog].tap.tap_pan[1];
    data[TAP2_FEEDBACK_IDX]= QuadtGT_Bank[bank][prog].tap.tap_feedback[1];
    QuadGT_Encode_16Bit(QuadtGT_Bank[bank][prog].tap.tap_delay[2], &data[TAP3_DELAY_IDX]);
    data[TAP3_VOLUME_IDX]  = QuadtGT_Bank[bank][prog].tap.tap_volume[2];
    data[TAP3_PAN_IDX]     = QuadtGT_Bank[bank][prog].tap.tap_pan[2];
  }

  //-------------------------------------------------------------------------
  // Reverb Parameters (0x32 - 0x43)
  //-------------------------------------------------------------------------

  //-------------------------------------------------------------------------
  // Config (0x44)
  //-------------------------------------------------------------------------
  data[CONFIG_IDX]    = QuadtGT_Bank[bank][prog].config;

  //-------------------------------------------------------------------------
  // Mix Parameters (0x45 - 
  //-------------------------------------------------------------------------
  data[PREPOST_EQ_IDX]    = QuadtGT_Bank[bank][prog].prepost_eq & BIT0;
  data[DIRECT_LEVEL_IDX] |= (QuadtGT_Bank[bank][prog].direct_level << 1) & BITS1to7;
  data[MASTER_EFFECTS_LEVEL_IDX] = QuadtGT_Bank[bank][prog].master_effects_level;

  if (QuadtGT_Bank[bank][prog].prepost_eq == 0) data[PREAMP_LEVEL_IDX]=QuadtGT_Bank[bank][prog].preamp_level;
  else                                    data[EQ_LEVEL_IDX]=QuadtGT_Bank[bank][prog].eq_level;

  if (QuadtGT_Bank[bank][prog].config==CFG1_LESLIE_DELAY_REVERB)
  {
    data[LESLIE_LEVEL_IDX] = QuadtGT_Bank[bank][prog].leslie_level;
  }
  else if (QuadtGT_Bank[bank][prog].config==CFG5_RINGMOD_DELAY_REVERB)
  {
    data[RING_MOD_LEVEL_IDX] = QuadtGT_Bank[bank][prog].ring_mod_level;
  }
  else
  {
    data[PITCH_LEVEL_IDX] = QuadtGT_Bank[bank][prog].pitch_level;
  }

  data[DELAY_LEVEL_IDX ] = QuadtGT_Bank[bank][prog].delay_level;
  if (QuadtGT_Bank[bank][prog].config!=CFG5_RINGMOD_DELAY_REVERB)
  {
    data[REVERB_LEVEL_IDX ] = QuadtGT_Bank[bank][prog].reverb_level;
  }
  else
  {
    data[RES1_AMP_IDX_B] = QuadtGT_Bank[bank][prog].res_amp[0]; 
  }

  //-------------------------------------------------------------------------
  // Modulation Parameters (0x50 - 0x67)
  //-------------------------------------------------------------------------
  data[MOD1_SOURCE_IDX]=QuadtGT_Bank[bank][prog].mod_source[0];
  data[MOD1_TARGET_IDX]=QuadtGT_Bank[bank][prog].mod_target[0];
  data[MOD1_AMP_IDX]=   QuadtGT_Bank[bank][prog].mod_amp[0]+99;
                         
  data[MOD2_SOURCE_IDX]=QuadtGT_Bank[bank][prog].mod_source[1];
  data[MOD2_TARGET_IDX]=QuadtGT_Bank[bank][prog].mod_target[1];
  data[MOD2_AMP_IDX]=   QuadtGT_Bank[bank][prog].mod_amp[1]+99;
                         
  data[MOD3_SOURCE_IDX]=QuadtGT_Bank[bank][prog].mod_source[2];
  data[MOD3_TARGET_IDX]=QuadtGT_Bank[bank][prog].mod_target[2];
  data[MOD3_AMP_IDX]=   QuadtGT_Bank[bank][prog].mod_amp[2]+99;
                         
  data[MOD4_SOURCE_IDX]=QuadtGT_Bank[bank][prog].mod_source[3];
  data[MOD4_TARGET_IDX]=QuadtGT_Bank[bank][prog].mod_target[3];
  data[MOD4_AMP_IDX]=   QuadtGT_Bank[bank][prog].mod_amp[3]+99;
                         
  data[MOD5_SOURCE_IDX]=QuadtGT_Bank[bank][prog].mod_source[4];
  data[MOD5_TARGET_IDX]=QuadtGT_Bank[bank][prog].mod_target[4];
  data[MOD4_AMP_IDX]=   QuadtGT_Bank[bank][prog].mod_amp[4]+99;
                         
  data[MOD6_SOURCE_IDX]=QuadtGT_Bank[bank][prog].mod_source[5];
  data[MOD6_TARGET_IDX]=QuadtGT_Bank[bank][prog].mod_target[5];
  data[MOD5_AMP_IDX]=   QuadtGT_Bank[bank][prog].mod_amp[5]+99;
                         
  data[MOD7_SOURCE_IDX]=QuadtGT_Bank[bank][prog].mod_source[6];
  data[MOD7_TARGET_IDX]=QuadtGT_Bank[bank][prog].mod_target[6];
  data[MOD7_AMP_IDX]=   QuadtGT_Bank[bank][prog].mod_amp[6]+99;
                         
  data[MOD8_SOURCE_IDX]=QuadtGT_Bank[bank][prog].mod_source[7];
  data[MOD8_TARGET_IDX]=QuadtGT_Bank[bank][prog].mod_target[7];
  data[MOD8_AMP_IDX]=   QuadtGT_Bank[bank][prog].mod_amp[7]+99;
  
  //-------------------------------------------------------------------------
  // Multi-tap Parameters (0x68 - 0x69)
  //-------------------------------------------------------------------------
  
  //-------------------------------------------------------------------------
  // Name (0x6A)
  //-------------------------------------------------------------------------
  memcpy(&data[NAME_IDX], QuadtGT_Bank[bank][prog].name, NAME_LENGTH);

  //-------------------------------------------------------------------------
  // More mix/ring mod/res/pan Parameters (0x78 - 0x7B)
  //-------------------------------------------------------------------------
  if (QuadtGT_Bank[bank][prog].config==CFG5_RINGMOD_DELAY_REVERB)
  {
    data[RING_MOD_OUTPUT_MIX_IDX]  = QuadtGT_Bank[bank][prog].ring_mod_out_mix+99;
    data[RING_MOD_DEL_REV_MIX_IDX] = QuadtGT_Bank[bank][prog].ring_mod_in_mix+99;
  }
  else
  {
    data[RES2_AMP_IDX_B] = QuadtGT_Bank[bank][prog].res_amp[1];
    data[RES3_AMP_IDX]   = QuadtGT_Bank[bank][prog].res_amp[2];
  }

  data[PAN_SPEED_IDX] = QuadtGT_Bank[bank][prog].pan_speed;
  data[PAN_DEPTH_IDX] = QuadtGT_Bank[bank][prog].pan_depth;
  
  //-------------------------------------------------------------------------
  // Pre-amp Parameters (0x7C - 0x7F)
  //-------------------------------------------------------------------------
  data[PREAMP_DIST_IDX]  = QuadtGT_Bank[bank][prog].preamp.dist & BITS0to3;
  data[PREAMP_COMP_IDX] |= (QuadtGT_Bank[bank][prog].preamp.comp << 4) & BITS4to6;
  data[EQ_MODE_IDX]     |= (QuadtGT_Bank[bank][prog].mode<<7) & BIT7;

  data[MIX_MOD_IDX]      = (QuadtGT_Bank[bank][prog].mix_mod      << 6) & BITS6to7;
  data[EFFECT_LOOP_IDX] |= (QuadtGT_Bank[bank][prog].preamp.effect_loop  << 5) & BIT5;
  data[BASS_BOOST_IDX]  |= (QuadtGT_Bank[bank][prog].preamp.bass_boost   << 4) & BIT4;
  data[PREAMP_TONE_IDX] |= (QuadtGT_Bank[bank][prog].preamp.preamp_tone  << 2) & BITS2to3;
  data[CAB_SIM_IDX]     |= QuadtGT_Bank[bank][prog].preamp.cab_sim  & BITS0to1;

  data[PREAMP_OD_IDX]         = (QuadtGT_Bank[bank][prog].preamp.od << 5) & BITS5to7;
  data[PREAMP_GATE_IDX]      |= (QuadtGT_Bank[bank][prog].preamp.preamp_gate & BITS0to4);

  data[PREAMP_OUT_LEVEL_IDX] = QuadtGT_Bank[bank][prog].preamp.preamp_out_level;
  
  // DEBUG: Write the patch in QuadGT format to a disk file
  save_patch(data,QUAD_PATCH_SIZE,"patch_outquadgt.txt");

  return 0;
}


//---------------------------------------------------------------------------
// Name     : QuadGT_Convert_QuadGT_To_Internal
// Description : Convert raw Quadraverb GT data to internal data structure
//---------------------------------------------------------------------------
UInt32 QuadGT_Convert_QuadGT_To_Internal(UInt8 prog, UInt8* data)
{
  if (prog >= QUAD_NUM_PATCH) return 1;

  // DEBUG: Write the patch in QuadGT format to a disk file
  save_patch(data,QUAD_PATCH_SIZE,"patch_inquadgt.txt");

  memcpy(QuadtGT_Bank[bank][prog].name, &data[NAME_IDX], NAME_LENGTH);
  QuadtGT_Bank[bank][prog].name[NAME_LENGTH]=0;

  //-------------------------------------------------------------------------
  // Decode modes and configurations first so we can use them later for decision making
  //-------------------------------------------------------------------------
  QuadtGT_Bank[bank][prog].config        = data[CONFIG_IDX];
  QuadtGT_Bank[bank][prog].mode       = ((data[EQ_MODE_IDX]&BIT7)>>7);   // Eq Mode: Eq or Eq/Resonator
  QuadtGT_Bank[bank][prog].pitch.pitch_mode    = data[PITCH_MODE_IDX];
  QuadtGT_Bank[bank][prog].delay_mode    = data[DELAY_MODE_IDX];
  if (QuadtGT_Bank[bank][prog].config != CFG3_5BANDEQ_PITCH_DELAY)
  {
    QuadtGT_Bank[bank][prog].reverb_mode   = data[REVERB_MODE_IDX];
  }

  //-------------------------------------------------------------------------
  // Preamp Parameters (0x7C - 0x7F)
  //-------------------------------------------------------------------------
  QuadtGT_Bank[bank][prog].preamp.comp             = (data[PREAMP_COMP_IDX] & BITS4to6) >> 4;
  QuadtGT_Bank[bank][prog].preamp.od               = (data[PREAMP_OD_IDX]   & BITS5to7) >> 5;
  QuadtGT_Bank[bank][prog].preamp.dist             = (data[PREAMP_DIST_IDX] & BITS0to3);
  QuadtGT_Bank[bank][prog].preamp.preamp_tone      = (data[PREAMP_TONE_IDX] & BITS2to3) >> 2;
  QuadtGT_Bank[bank][prog].preamp.bass_boost       = (data[BASS_BOOST_IDX]  & BIT4) >> 4;
  QuadtGT_Bank[bank][prog].preamp.cab_sim          = (data[CAB_SIM_IDX]     & BITS0to1);
  QuadtGT_Bank[bank][prog].preamp.effect_loop      = (data[EFFECT_LOOP_IDX] & BIT5) >> 5;
  QuadtGT_Bank[bank][prog].preamp.preamp_gate      = (data[PREAMP_GATE_IDX] & BITS0to4);
  QuadtGT_Bank[bank][prog].preamp.preamp_out_level = data[PREAMP_OUT_LEVEL_IDX];

  //-------------------------------------------------------------------------
  // Eq Parameters
  //-------------------------------------------------------------------------
  QuadtGT_Bank[bank][prog].preset = (data[EQ_PRESET_IDX]&BITS1to7)>>1;       // User or Preset 1-6

  // Resonator parameters, alternative one
  if ((QuadtGT_Bank[bank][prog].config == CFG0_EQ_PITCH_DELAY_REVERB) &&
      (QuadtGT_Bank[bank][prog].mode == EQMODE1_EQ_PLUS_RESONATOR))
  {
    FormDebug->Log(NULL, "Loading resontors 1 and 2");
    QuadtGT_Bank[bank][prog].res_tune[0] = data[RES1_TUNE_IDX] - RES_TUNE_OFFSET; 
    QuadtGT_Bank[bank][prog].res_decay[0] =data[RES1_DECAY_IDX] ; 
    QuadtGT_Bank[bank][prog].res_amp[0] =  data[RES1_AMP_IDX]   ; 
                                     
    QuadtGT_Bank[bank][prog].res_tune[1] = data[RES2_TUNE_IDX]  - RES_TUNE_OFFSET; 
    QuadtGT_Bank[bank][prog].res_decay[1] =data[RES2_DECAY_IDX] ; 
    QuadtGT_Bank[bank][prog].res_amp[1] =  data[RES2_AMP_IDX]   ; 

    FormDebug->Log(NULL, "Res Tune 1 "+AnsiString(QuadtGT_Bank[bank][prog].res_tune[0]));
    FormDebug->Log(NULL, "Res Tune 2 "+AnsiString(QuadtGT_Bank[bank][prog].res_tune[1]));
  }
  // Graphic Eq
  else if (QuadtGT_Bank[bank][prog].config == CFG2_GEQ_DELAY)
  {
    QuadtGT_Bank[bank][prog].geq_16hz   = data[GEQ_16HZ_IDX]-GEQ_OFFSET;
    QuadtGT_Bank[bank][prog].geq_32hz   = data[GEQ_32HZ_IDX]-GEQ_OFFSET;
    QuadtGT_Bank[bank][prog].geq_62hz   = data[GEQ_62HZ_IDX]-GEQ_OFFSET;

    QuadtGT_Bank[bank][prog].geq_4khz   = data[GEQ_4KHZ_IDX]-GEQ_OFFSET;
    QuadtGT_Bank[bank][prog].geq_8khz   = data[GEQ_8KHZ_IDX]-GEQ_OFFSET;
  }
  // 3 and 5 Band Eq
  else
  {
    QuadtGT_Bank[bank][prog].low_freq   = QuadGT_Decode_16Bit(&data[LOW_EQ_FREQ_IDX]);
    QuadtGT_Bank[bank][prog].low_amp    = QuadGT_Decode_16Bit(&data[LOW_EQ_AMP_IDX]);
    QuadtGT_Bank[bank][prog].high_freq  = QuadGT_Decode_16Bit(&data[HIGH_EQ_FREQ_IDX]);
  }

  if (QuadtGT_Bank[bank][prog].config==CFG2_GEQ_DELAY)
  {
    QuadtGT_Bank[bank][prog].geq_126hz  = data[GEQ_126HZ_IDX]-GEQ_OFFSET;
    QuadtGT_Bank[bank][prog].geq_250hz  = data[GEQ_250HZ_IDX]-GEQ_OFFSET;
    QuadtGT_Bank[bank][prog].geq_500hz  = data[GEQ_500HZ_IDX]-GEQ_OFFSET;
    QuadtGT_Bank[bank][prog].geq_1khz   = data[GEQ_1KHZ_IDX]-GEQ_OFFSET;
    QuadtGT_Bank[bank][prog].geq_2khz   = data[GEQ_2KHZ_IDX]-GEQ_OFFSET;

    QuadtGT_Bank[bank][prog].geq_16khz  = data[GEQ_16KHZ_IDX]-GEQ_OFFSET;
  }
  else
  {
    QuadtGT_Bank[bank][prog].mid_freq   = QuadGT_Decode_16Bit(&data[MID_EQ_FREQ_IDX]);
    QuadtGT_Bank[bank][prog].mid_amp    = QuadGT_Decode_16Bit(&data[MID_EQ_AMP_IDX]);
    QuadtGT_Bank[bank][prog].mid_q      = data[MID_EQ_BW_IDX];
    QuadtGT_Bank[bank][prog].high_amp   = QuadGT_Decode_16Bit(&data[HIGH_EQ_AMP_IDX]);
  }

  // Type = 3 Eq and 5 Resonators
  if ( ((QuadtGT_Bank[bank][prog].config == CFG3_5BANDEQ_PITCH_DELAY) && 
        (QuadtGT_Bank[bank][prog].mode == EQMODE1_EQ_PLUS_RESONATOR)) ||
       (QuadtGT_Bank[bank][prog].config == CFG6_RESONATOR_DELAY_REVERB))
  {
    FormDebug->Log(NULL, "Loading resontors tune and decay");
    QuadtGT_Bank[bank][prog].res_tune[0]  = data[RES1_TUNE_IDX_B];
    QuadtGT_Bank[bank][prog].res_decay[0] =data[RES1_DECAY_IDX_B];
                                     
    QuadtGT_Bank[bank][prog].res_tune[1]  = data[RES2_TUNE_IDX_B];
    QuadtGT_Bank[bank][prog].res_decay[1] =data[RES2_DECAY_IDX_B];
                                     
    QuadtGT_Bank[bank][prog].res_tune[2]  = data[RES3_TUNE_IDX];
    QuadtGT_Bank[bank][prog].res_decay[2] =data[RES3_DECAY_IDX];
                                     
    QuadtGT_Bank[bank][prog].res_tune[3]  = data[RES4_TUNE_IDX];
    QuadtGT_Bank[bank][prog].res_decay[3] =data[RES4_DECAY_IDX];
                                     
    QuadtGT_Bank[bank][prog].res_tune[4]  = data[RES5_TUNE_IDX];
    QuadtGT_Bank[bank][prog].res_decay[4] =data[RES5_DECAY_IDX];

    FormDebug->Log(NULL, "Res Tune 1 "+AnsiString(QuadtGT_Bank[bank][prog].res_tune[0]));
    FormDebug->Log(NULL, "Res Tune 2 "+AnsiString(QuadtGT_Bank[bank][prog].res_tune[1]));
    FormDebug->Log(NULL, "Res Tune 3 "+AnsiString(QuadtGT_Bank[bank][prog].res_tune[2]));
    FormDebug->Log(NULL, "Res Tune 4 "+AnsiString(QuadtGT_Bank[bank][prog].res_tune[3]));
    FormDebug->Log(NULL, "Res Tune 5 "+AnsiString(QuadtGT_Bank[bank][prog].res_tune[4]));
  }
  // Otherwise, assume 5 band Eq
  else
  {
    QuadtGT_Bank[bank][prog].low_mid_freq   = QuadGT_Decode_16Bit(&data[LOW_MID_EQ_FREQ_IDX]);
    QuadtGT_Bank[bank][prog].low_mid_amp    = QuadGT_Decode_16Bit(&data[LOW_MID_EQ_AMP_IDX]);
    QuadtGT_Bank[bank][prog].low_mid_q      = data[LOW_MID_EQ_BW_IDX];

    QuadtGT_Bank[bank][prog].high_mid_freq   = QuadGT_Decode_16Bit(&data[HIGH_MID_EQ_FREQ_IDX]);
    QuadtGT_Bank[bank][prog].high_mid_amp    = QuadGT_Decode_16Bit(&data[HIGH_MID_EQ_AMP_IDX]);
    QuadtGT_Bank[bank][prog].high_mid_q      = data[HIGH_MID_EQ_BW_IDX];
  }

  // Sampling or Tap/Leslie parameters
  if (QuadtGT_Bank[bank][prog].config == CFG7_SAMPLING) 
  {
    QuadtGT_Bank[bank][prog].sample.start            = data[SAMPLE_START_IDX]; 
    QuadtGT_Bank[bank][prog].sample.length           = data[SAMPLE_LENGTH_IDX]; 
    QuadtGT_Bank[bank][prog].sample.playback_mode    = data[SAMPLE_PLAYBACK_MODE_IDX]; 
    QuadtGT_Bank[bank][prog].sample.pitch            = data[SAMPLE_PITCH_IDX]; 
    QuadtGT_Bank[bank][prog].sample.rec_audio_trigger= data[SAMPLE_REC_AUDIO_TRIG_IDX]; 
    QuadtGT_Bank[bank][prog].sample.midi_trigger     = data[SAMPLE_MIDI_TRIG_IDX]; 
    QuadtGT_Bank[bank][prog].sample.midi_base_note   = data[SAMPLE_MIDI_BASE_NOTE_IDX]; 
    QuadtGT_Bank[bank][prog].sample.low_midi_note    = data[SAMPLE_LOW_MIDI_NOTE_IDX]; 
    QuadtGT_Bank[bank][prog].sample.high_midi_note   = data[SAMPLE_HIGH_MIDI_NOTE_IDX]; 
  }
  else
  {
    if (QuadtGT_Bank[bank][prog].config == CFG1_LESLIE_DELAY_REVERB)
    {
      QuadtGT_Bank[bank][prog].leslie_high_rotor_level = data[LESLIE_HIGH_ROTOR_LEVEL_IDX];
      QuadtGT_Bank[bank][prog].leslie_motor            = data[LESLIE_MOTOR_IDX];
      QuadtGT_Bank[bank][prog].leslie_speed            = data[LESLIE_SPEED_IDX];
    }
    else
    {
      QuadtGT_Bank[bank][prog].tap.tap_delay[0] = QuadGT_Decode_16Bit_Split(&data[TAP1_DELAY_MSB_IDX],
                                                                &data[TAP1_DELAY_LSB_IDX]);
      QuadtGT_Bank[bank][prog].tap.tap_delay[1] = QuadGT_Decode_16Bit(&data[TAP2_DELAY_IDX]);
    }

    QuadtGT_Bank[bank][prog].tap.tap_volume[0] = data[TAP1_VOLUME_IDX];
    QuadtGT_Bank[bank][prog].tap.tap_pan[0]    = data[TAP1_PAN_IDX];

    QuadtGT_Bank[bank][prog].pitch.detune_amount = data[DETUNE_AMOUNT_IDX];

    QuadtGT_Bank[bank][prog].tap.tap_volume[1] = data[TAP2_VOLUME_IDX];
    QuadtGT_Bank[bank][prog].tap.tap_feedback[2] = data[TAP3_FEEDBACK_IDX];

    if (QuadtGT_Bank[bank][prog].config == CFG3_5BANDEQ_PITCH_DELAY)
    {
      QuadtGT_Bank[bank][prog].tap.tap_delay[3] = QuadGT_Decode_16Bit(&data[TAP4_DELAY_IDX]);
    }
  }

  if (QuadtGT_Bank[bank][prog].config == CFG1_LESLIE_DELAY_REVERB)
  {
    QuadtGT_Bank[bank][prog].leslie_seperation = data[LESLIE_SEPARATION_IDX];
  }
  else
  {
    QuadtGT_Bank[bank][prog].tap.tap_feedback[0] = data[TAP1_FEEDBACK_IDX];
  }

  QuadtGT_Bank[bank][prog].tap.tap_volume[3] = data[TAP4_VOLUME_IDX];

  if (QuadtGT_Bank[bank][prog].config == CFG3_5BANDEQ_PITCH_DELAY)
  {
    QuadtGT_Bank[bank][prog].tap.tap_pan[3]      = data[TAP4_PAN_IDX];
    QuadtGT_Bank[bank][prog].tap.tap_feedback[3] = data[TAP4_FEEDBACK_IDX];

    QuadtGT_Bank[bank][prog].tap.tap_delay[4] = QuadGT_Decode_16Bit(&data[TAP5_DELAY_IDX]);
    QuadtGT_Bank[bank][prog].tap.tap_volume[4] = data[TAP5_VOLUME_IDX];
    QuadtGT_Bank[bank][prog].tap.tap_pan[4]      = data[TAP5_PAN_IDX];
    QuadtGT_Bank[bank][prog].tap.tap_feedback[4] = data[TAP5_FEEDBACK_IDX];

    QuadtGT_Bank[bank][prog].tap.tap_delay[5] = QuadGT_Decode_16Bit(&data[TAP6_DELAY_IDX]);
    QuadtGT_Bank[bank][prog].tap.tap_volume[5] = data[TAP6_VOLUME_IDX];
    QuadtGT_Bank[bank][prog].tap.tap_pan[5]      = data[TAP6_PAN_IDX];
    QuadtGT_Bank[bank][prog].tap.tap_feedback[5] = data[TAP6_FEEDBACK_IDX];

    QuadtGT_Bank[bank][prog].tap.tap_delay[6] = QuadGT_Decode_16Bit(&data[TAP7_DELAY_IDX]);
    QuadtGT_Bank[bank][prog].tap.tap_volume[6] = data[TAP7_VOLUME_IDX];
    QuadtGT_Bank[bank][prog].tap.tap_pan[6]      = data[TAP7_PAN_IDX];
  }
  else
  {
    //-------------------------------------------------------------------------
    // Reverb Parameters
    //-------------------------------------------------------------------------
    QuadtGT_Bank[bank][prog].reverb_input_1	        = data[REVERB_INPUT_1_IDX];
    QuadtGT_Bank[bank][prog].reverb_input_2	        = data[REVERB_INPUT_2_IDX];
    QuadtGT_Bank[bank][prog].reverb_input_mix	        = data[REVERB_INPUT_MIX_IDX]-99;
    QuadtGT_Bank[bank][prog].reverb_predelay	        = data[REVERB_PREDELAY_IDX];
    QuadtGT_Bank[bank][prog].reverb_predelay_mix	= data[REVERB_PREDELAY_MIX_IDX]-99;
    QuadtGT_Bank[bank][prog].reverb_decay	        = data[REVERB_DECAY_IDX];
    QuadtGT_Bank[bank][prog].reverb_diffusion	        = data[REVERB_DIFFUSION_IDX];
    QuadtGT_Bank[bank][prog].reverb_low_decay	        = data[REVERB_LOW_DECAY_IDX];
    QuadtGT_Bank[bank][prog].reverb_high_decay	= data[REVERB_HIGH_DECAY_IDX];
    QuadtGT_Bank[bank][prog].reverb_density	        = data[REVERB_DENSITY_IDX];
    QuadtGT_Bank[bank][prog].reverb_gate	        = data[REVERB_GATE_IDX];
    QuadtGT_Bank[bank][prog].reverb_gate_hold	        = data[REVERB_GATE_HOLD_IDX];
    QuadtGT_Bank[bank][prog].reverb_gate_release	= data[REVERB_GATE_RELEASE_IDX];
    QuadtGT_Bank[bank][prog].reverb_gated_level	= data[REVERB_GATED_LEVEL_IDX];

    QuadtGT_Bank[bank][prog].ring_mod_shift = QuadGT_Decode_16Bit(&data[RING_MOD_SHIFT_IDX]);
  }

  QuadtGT_Bank[bank][prog].multitap_master_feedback = data[MULTITAP_MASTER_FEEDBACK_IDX];
  QuadtGT_Bank[bank][prog].multitap_number          = data[MULTITAP_NUMBER_IDX];

  //-------------------------------------------------------------------------
  // Pitch Parameters
  //-------------------------------------------------------------------------
  QuadtGT_Bank[bank][prog].pitch.pitch_input   = data[PITCH_INPUT_IDX] & BIT0;
  QuadtGT_Bank[bank][prog].res_amp[3] = (data[RES4_AMP_IDX] && BITS1to7) >> 1;

  QuadtGT_Bank[bank][prog].pitch.lfo_waveform  = data[LFO_WAVEFORM_IDX] & BIT0;
  QuadtGT_Bank[bank][prog].pitch.lfo_speed     = data[LFO_SPEED_IDX];
  if (QuadtGT_Bank[bank][prog].config == CFG6_RESONATOR_DELAY_REVERB) 
  {
    QuadtGT_Bank[bank][prog].res_decay_all = data[RES_DECAY_IDX];
    QuadtGT_Bank[bank][prog].res_midi_gate = data[RES_MIDI_GATE_IDX];
    FormDebug->Log(NULL, "Midi Gate "+AnsiString(QuadtGT_Bank[bank][prog].res_midi_gate));

    QuadtGT_Bank[bank][prog].res_pitch[0]  = data[RES1_PITCH_IDX];
    QuadtGT_Bank[bank][prog].res_pitch[1]  = data[RES2_PITCH_IDX];
    QuadtGT_Bank[bank][prog].res_pitch[2]  = data[RES3_PITCH_IDX];
    QuadtGT_Bank[bank][prog].res_pitch[3]  = data[RES4_PITCH_IDX];
    QuadtGT_Bank[bank][prog].res_pitch[4]  = data[RES5_PITCH_IDX];
  }
  else
  {
    QuadtGT_Bank[bank][prog].pitch.lfo_depth      = data[LFO_DEPTH_IDX];
    QuadtGT_Bank[bank][prog].pitch.trigger_flange = data[TRIGGER_FLANGE_IDX];

    QuadtGT_Bank[bank][prog].tap.tap_delay[7] = QuadGT_Decode_16Bit(&data[TAP8_DELAY_IDX]);
    QuadtGT_Bank[bank][prog].tap.tap_volume[7] = data[TAP8_VOLUME_IDX];
    QuadtGT_Bank[bank][prog].tap.tap_pan[7]    = data[TAP8_PAN_IDX];
    QuadtGT_Bank[bank][prog].tap.tap_feedback[7]= data[TAP8_FEEDBACK_IDX];
  }
  QuadtGT_Bank[bank][prog].pitch.pitch_feedback= data[PITCH_FEEDBACK_IDX];

  //-------------------------------------------------------------------------
  // Delay Parameters
  //-------------------------------------------------------------------------
  QuadtGT_Bank[bank][prog].delay_input=data[DELAY_INPUT_IDX]&BIT0;
  QuadtGT_Bank[bank][prog].delay_input_mix = data[DELAY_INPUT_MIX_IDX];

  if ((QuadtGT_Bank[bank][prog].delay_mode == 0) ||
      (QuadtGT_Bank[bank][prog].delay_mode == 1) ||
      (QuadtGT_Bank[bank][prog].delay_mode == 2))
  {
    QuadtGT_Bank[bank][prog].delay_left          = QuadGT_Decode_16Bit(&data[DELAY_LEFT_IDX]);
    QuadtGT_Bank[bank][prog].delay_left_feedback =data[DELAY_LEFT_FEEDBACK_IDX];
    QuadtGT_Bank[bank][prog].delay_right         = QuadGT_Decode_16Bit(&data[DELAY_RIGHT_IDX]);
    QuadtGT_Bank[bank][prog].delay_right_feedback=data[DELAY_RIGHT_FEEDBACK_IDX];
  }
  else if (QuadtGT_Bank[bank][prog].delay_mode == 3)
  {
    QuadtGT_Bank[bank][prog].tap.tap_pan[1]      = data[TAP2_PAN_IDX];
    QuadtGT_Bank[bank][prog].tap.tap_feedback[1] = data[TAP2_FEEDBACK_IDX];
    QuadtGT_Bank[bank][prog].tap.tap_delay[2]    = QuadGT_Decode_16Bit(&data[TAP3_DELAY_IDX]);
    QuadtGT_Bank[bank][prog].tap.tap_volume[2]   = data[TAP3_VOLUME_IDX];
    QuadtGT_Bank[bank][prog].tap.tap_pan[2]      = data[TAP3_PAN_IDX];
  }

  //-------------------------------------------------------------------------
  // Mix Parameters
  //-------------------------------------------------------------------------
  QuadtGT_Bank[bank][prog].prepost_eq=data[PREPOST_EQ_IDX] & BIT0;
  QuadtGT_Bank[bank][prog].direct_level=(data[DIRECT_LEVEL_IDX ] & BITS1to7) >> 1;
  QuadtGT_Bank[bank][prog].master_effects_level=data[MASTER_EFFECTS_LEVEL_IDX ];

  if (QuadtGT_Bank[bank][prog].prepost_eq == 0) QuadtGT_Bank[bank][prog].preamp_level=data[PREAMP_LEVEL_IDX];
  else                                    QuadtGT_Bank[bank][prog].eq_level=data[EQ_LEVEL_IDX];

  if (QuadtGT_Bank[bank][prog].config==CFG1_LESLIE_DELAY_REVERB)
  {
    QuadtGT_Bank[bank][prog].leslie_level=data[LESLIE_LEVEL_IDX];
  }
  else if (QuadtGT_Bank[bank][prog].config==CFG5_RINGMOD_DELAY_REVERB)
  {
    QuadtGT_Bank[bank][prog].ring_mod_level=data[RING_MOD_LEVEL_IDX];
  }
  else
  {
    QuadtGT_Bank[bank][prog].pitch_level=data[PITCH_LEVEL_IDX];
  }

  QuadtGT_Bank[bank][prog].delay_level=data[DELAY_LEVEL_IDX ];
  if (QuadtGT_Bank[bank][prog].config!=CFG5_RINGMOD_DELAY_REVERB)
  {
    QuadtGT_Bank[bank][prog].reverb_level=data[REVERB_LEVEL_IDX ];
  }
  else
  {
    QuadtGT_Bank[bank][prog].res_amp[0] =  data[RES1_AMP_IDX_B]; 
  }

  QuadtGT_Bank[bank][prog].mix_mod=(data[MIX_MOD_IDX] & BITS6to7) >> 6;

  //-------------------------------------------------------------------------
  // Modulation Parameters
  //-------------------------------------------------------------------------
  QuadtGT_Bank[bank][prog].mod_source[0]=data[MOD1_SOURCE_IDX];
  QuadtGT_Bank[bank][prog].mod_target[0]=data[MOD1_TARGET_IDX];
  QuadtGT_Bank[bank][prog].mod_amp[0]   =data[MOD1_AMP_IDX]-99;

  QuadtGT_Bank[bank][prog].mod_source[1]=data[MOD2_SOURCE_IDX];
  QuadtGT_Bank[bank][prog].mod_target[1]=data[MOD2_TARGET_IDX];
  QuadtGT_Bank[bank][prog].mod_amp[1]   =data[MOD2_AMP_IDX]-99;

  QuadtGT_Bank[bank][prog].mod_source[2]=data[MOD3_SOURCE_IDX];
  QuadtGT_Bank[bank][prog].mod_target[2]=data[MOD3_TARGET_IDX];
  QuadtGT_Bank[bank][prog].mod_amp[2]   =data[MOD3_AMP_IDX]-99;

  QuadtGT_Bank[bank][prog].mod_source[3]=data[MOD4_SOURCE_IDX];
  QuadtGT_Bank[bank][prog].mod_target[3]=data[MOD4_TARGET_IDX];
  QuadtGT_Bank[bank][prog].mod_amp[3]   =data[MOD4_AMP_IDX]-99;

  QuadtGT_Bank[bank][prog].mod_source[4]=data[MOD5_SOURCE_IDX];
  QuadtGT_Bank[bank][prog].mod_target[4]=data[MOD5_TARGET_IDX];
  QuadtGT_Bank[bank][prog].mod_amp[4]   =data[MOD4_AMP_IDX]-99;

  QuadtGT_Bank[bank][prog].mod_source[5]=data[MOD6_SOURCE_IDX];
  QuadtGT_Bank[bank][prog].mod_target[5]=data[MOD6_TARGET_IDX];
  QuadtGT_Bank[bank][prog].mod_amp[5]   =data[MOD5_AMP_IDX]-99;

  QuadtGT_Bank[bank][prog].mod_source[6]=data[MOD7_SOURCE_IDX];
  QuadtGT_Bank[bank][prog].mod_target[6]=data[MOD7_TARGET_IDX];
  QuadtGT_Bank[bank][prog].mod_amp[6]   =data[MOD7_AMP_IDX]-99;

  QuadtGT_Bank[bank][prog].mod_source[7]=data[MOD8_SOURCE_IDX];
  QuadtGT_Bank[bank][prog].mod_target[7]=data[MOD8_TARGET_IDX];
  QuadtGT_Bank[bank][prog].mod_amp[7]   =data[MOD8_AMP_IDX]-99;

  if (QuadtGT_Bank[bank][prog].config==CFG5_RINGMOD_DELAY_REVERB)
  {
    QuadtGT_Bank[bank][prog].ring_mod_out_mix = data[RING_MOD_OUTPUT_MIX_IDX]-99;
    QuadtGT_Bank[bank][prog].ring_mod_in_mix= data[RING_MOD_DEL_REV_MIX_IDX]-99;
  }
  else
  {
    QuadtGT_Bank[bank][prog].res_amp[1] =  data[RES2_AMP_IDX_B]; 
    QuadtGT_Bank[bank][prog].res_amp[2] =  data[RES3_AMP_IDX]; 
  }
  QuadtGT_Bank[bank][prog].pan_speed = data[PAN_SPEED_IDX];
  QuadtGT_Bank[bank][prog].pan_depth = data[PAN_DEPTH_IDX];
  //-------------------------------------------------------------------------
  // Resonator Parameters
  //-------------------------------------------------------------------------
  QuadtGT_Bank[bank][prog].res_amp[4] = (data[RES5_AMP_IDX] & BITS1to7) >> 1;

  return 0;
}

//---------------------------------------------------------------------------
// Name        : QuadPatchWriteClick
// Description : Write the cyurrent patch to the QuadGT via MIDI
// Parameter 1 : Pointer object that generated the event
// Returns     : NONE
//---------------------------------------------------------------------------
void __fastcall TMainForm::QuadPatchWriteClick(TObject *Sender)
{
  UInt8 prog;
  long int status;
  UInt8 quadgt[QUAD_PATCH_SIZE+1];
  UInt8 sysex[SYSEX_PATCH_SIZE];
  UInt32 sysex_size;

  prog = (UInt8) StrToInt(QuadPatchNum->Text);

  // Convert internal format to QuadGT format
  QuadGT_Convert_Data_From_Internal(prog, quadgt);
  quadgt[QUAD_PATCH_SIZE]=0;

  // Convert QuadGT format to Sysex
  sysex_size=QuadGT_Encode_To_Sysex(quadgt, QUAD_PATCH_SIZE, sysex, sizeof(sysex));

  // Send message
  if (sysex_size > 0)
  {
    Midi_Out_Dump(prog, sysex, sysex_size);
  }
}

//---------------------------------------------------------------------------
// Name        : QuadPatchReadClick
// Description : Read a single patch from the QuadGT via MIDI
// Parameter 1 : Pointer to object that generated the event (In, unused)
// Returns     : NONE.
//---------------------------------------------------------------------------
void __fastcall TMainForm::QuadPatchReadClick(TObject *Sender)
{
  UInt8 patch;
  long int status;

  patch = (UInt8) StrToInt(QuadPatchNum->Text);
  status=Midi_Out_Dump_Req(patch);

  if (status != 0)
  {
     FormError->ShowErrorCode(status, "sending SYSEX to Midi output device");
  }

}
//---------------------------------------------------------------------------
// Name        : QuadPatchSaveClick
// Description : Save a program to a .bnk file (internal
//               data format as raw binary) on disk.
// Parameter 1 : Pointer to object that generated the event (In, unused)
// Returns     : NONE.
//---------------------------------------------------------------------------
void __fastcall TMainForm::QuadPatchSaveClick(TObject *Sender)
{
  UInt8 prog;
  FILE *patch_file;

  if (!MainForm->QuadGtPatchOpenDialog->Execute()) return;

  patch_file = fopen(MainForm->QuadGtPatchOpenDialog->Files->Strings[0].c_str(),"wb");

  if (patch_file == NULL)
  {
    FormError->ShowErrorCode(ferror(patch_file),"opening patch file");
    return;
  }

  prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);

  // TBD: Write a header

  if (fwrite(&QuadtGT_Bank[bank][prog], sizeof(tQuadGT_Prog), 1, patch_file) != 1)
  {
    FormError->ShowErrorCode(ferror(patch_file),"writing patch file ");
  }

  fclose(patch_file);

}

//---------------------------------------------------------------------------
// Name        : QuadSaveBankClick
// Description : Save an entire bank of programs to a .bnk file (internal
//               data format as raw binary) on disk.
// Parameter 1 : Pointer to object that generated the event (In, unused)
// Returns     : NONE.
//---------------------------------------------------------------------------
void __fastcall TMainForm::QuadBankSaveClick(TObject *Sender)
{
  UInt8 prog;
  FILE *bank_file;
  size_t val;

  if (!MainForm->InternalOpenDialog->Execute()) return;

  bank_file = fopen(MainForm->InternalOpenDialog->Files->Strings[0].c_str(),"wb");

  if (bank_file == NULL)
  {
    FormError->ShowErrorCode(ferror(bank_file),"opening bank file");
    return;
  }

  // TBD: Write a header

  // Write each patch
  for(prog=0; prog<QUAD_NUM_PATCH; prog++)
  {
    if (fwrite(&QuadtGT_Bank[bank][prog], sizeof(tQuadGT_Prog), 1, bank_file) != 1)
    {
      FormError->ShowErrorCode(ferror(bank_file),"writing bank file ");
      break;
    }
  }

  fclose(bank_file);

}
//---------------------------------------------------------------------------
// Name        : QuadPatchLoadClick
// Description : Load a single patch (program) from a .bnk file (internal
//               data format as raw binary) on disk.
// Parameter 1 : Pointer to object that generated the event (In, unused)
// Returns     : NONE.
//---------------------------------------------------------------------------
void __fastcall TMainForm::QuadPatchLoadClick(TObject *Sender)
{
  UInt8 prog;
  FILE *patch_file;
  tQuadGT_Prog quadgt_patch;

  FormDebug->Log(NULL, "Load Patch");

  if (!MainForm->QuadGtPatchOpenDialog->Execute()) return;

  patch_file = fopen(MainForm->QuadGtPatchOpenDialog->Files->Strings[0].c_str(),"rb");

  if (patch_file == NULL)
  {
    FormError->ShowErrorCode(ferror(patch_file),"opening patch file");
    return;
  }

  prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);

  // TBD: Read the header and check for validity
  if (fread(&quadgt_patch, sizeof(tQuadGT_Prog), 1, patch_file) != 1)
  {
    FormError->ShowErrorCode(ferror(patch_file),"reading patch file ");
    fclose(patch_file);
  }
  else
  {
    memcpy(&QuadtGT_Bank[bank][prog], &quadgt_patch, sizeof(quadgt_patch));
    fclose(patch_file);
    QuadGT_Redraw_Patch(bank, prog);
  }

}


//---------------------------------------------------------------------------
// Name        : QuadBankLoadClick
// Description : Load an entire bank of programs from a .bnk file (internal
//               data format as raw binary) on disk.
// Parameter 1 : Pointer to object that generated the event (In, unused)
// Returns     : NONE.
//---------------------------------------------------------------------------
void __fastcall TMainForm::QuadBankLoadClick(TObject *Sender)
{
  UInt8 prog;
  FILE *bank_file;
  tQuadGT_Prog quadgt_patch[QUAD_NUM_PATCH];

  if (!MainForm->InternalOpenDialog->Execute()) return;

  bank_file = fopen(MainForm->InternalOpenDialog->Files->Strings[0].c_str(),"rb");

  if (bank_file == NULL)
  {
    FormError->ShowErrorCode(ferror(bank_file),"opening bank file");
    return;
  }

  // TBD: Read the header and check for validity
  
  for(prog=0; prog<QUAD_NUM_PATCH; prog++)
  {
    if (fread(&quadgt_patch[prog], sizeof(tQuadGT_Prog), 1, bank_file) != 1)
    {
      FormError->ShowErrorCode(ferror(bank_file),"reading bank file ");
      break;
    }
  }

  fclose(bank_file);
  if (prog >= QUAD_NUM_PATCH)
  {
    memcpy(QuadtGT_Bank[bank], quadgt_patch, sizeof(QuadtGT_Bank[bank]));
  }

  prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  QuadGT_Redraw_Patch(bank, prog);
}

//---------------------------------------------------------------------------
// Name        : SysexBankLoadClick
// Description : Load an entire bank of programs from a raw binary dump of a 
//               complete SYSEX on disk.
// Parameter 1 : Pointer to object that generated the event (In, unused)
// Returns     : NONE.
//---------------------------------------------------------------------------
void __fastcall TMainForm::SysexBankLoadClick(TObject *Sender)
{
  UInt8 prog;
  FILE *sysex_file;
  tBuffer data;
  UInt32 size;

  if (!MainForm->SysexOpenDialog->Execute()) return;

  sysex_file = fopen(MainForm->SysexOpenDialog->Files->Strings[0].c_str(),"rb");
  if (sysex_file == NULL)
  {
    FormError->ShowErrorCode(ferror(sysex_file),"opening SysEx file");
    return;
  }

  // Determine size of the file
  fseek(sysex_file, 0, SEEK_END);
  size=ftell(sysex_file);
  rewind(sysex_file);

  data.buffer=(UInt8 *)malloc(sizeof(UInt8) * size);
  if (data.buffer == NULL)
  {
      FormError->ShowErrorCode(1,"allocating memory for SysEx file ");
  }
  else
  {
    if (fread(data.buffer, 1, size, sysex_file) != size)
    {
      FormError->ShowErrorCode(ferror(sysex_file),"reading SysEx file ");
    }
    else
    {
      data.length = size;
      QuadGT_Sysex_Process(data);
    }
  }

  fclose(sysex_file);
  free(data.buffer);

  prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  QuadGT_Redraw_Patch(bank, prog);
}

//---------------------------------------------------------------------------
// Name        : QuadPatchAuditionClick
// Description : Sends the current program to the QuadGT as the edit program
//               (for auditoning the program).  The program is not saved on
//               the QuadGT unless the user does so manually
// Param 1     : Pointer object that generated the event
//---------------------------------------------------------------------------
void __fastcall TMainForm::QuadPatchAuditionClick(TObject *Sender)
{
  UInt8 quadgt[QUAD_PATCH_SIZE];
  UInt8 sysex[200];
  UInt8 prog=(UInt8)StrToInt(QuadPatchNum->Text);
  UInt32 sysex_size;

  // Convert internal format to QuadGT format
  QuadGT_Convert_Data_From_Internal(prog, quadgt);

  // Convert QuadGT format to Sysex
  sysex_size=QuadGT_Encode_To_Sysex(quadgt, QUAD_PATCH_SIZE, sysex, sizeof(sysex));

  // Send message
  Midi_Out_Dump(EDIT_BUFFER, sysex, sysex_size);
}

//---------------------------------------------------------------------------
// Name        : QuadGT_Sysex_Process
// Description : Process a Sysex message from an Alesis Quadraverb or 
//               QuadraverbGT.  Checks that message has valid SysEx headers,
//               has an Alesis manufacturers code, a Quad or QuadGT device
//               code
// Parameter 1 : Buffer full of raw sysex data
// Parameters  : NONE.
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Sysex_Process(tBuffer sysex)
{
   UInt32 offset;
   UInt8 code,prog;
   UInt8 quadgt[QUAD_PATCH_SIZE];
   offset=0;

   if (sysex.buffer != NULL)
   {
     if (memcmp(Sysex_Start, sysex.buffer+offset, sizeof(Sysex_Start))==0)
     {
       offset+=sizeof(Sysex_Start);

       // SysEx contains Alesis data
       if (memcmp(Sysex_Alesis, sysex.buffer+offset, sizeof(Sysex_Alesis))==0)
       {
         offset+=sizeof(Sysex_Alesis);

         // SysEx contains Quadraverb or QuadraverbGT data
         if ((memcmp(Sysex_QuadGT, sysex.buffer+offset, sizeof(Sysex_QuadGT))==0) ||
             (memcmp(Sysex_Quad, sysex.buffer+offset, sizeof(Sysex_Quad))==0))
         {
           offset+=sizeof(Sysex_QuadGT);

	   code = *(sysex.buffer+offset);
	   offset+=1;
	   prog = *(sysex.buffer+offset);
	   offset+=1;

           if (code == *Sysex_Data_Dump)
           {
             // SysEx contains a single patch
             if (prog < QUAD_NUM_PATCH)
             {
               QuadGT_Decode_From_Sysex(sysex.buffer+offset,sysex.length-offset-1, quadgt, QUAD_PATCH_SIZE);
               QuadGT_Convert_QuadGT_To_Internal(prog, quadgt);
               QuadGT_Redraw_Patch(bank, prog);
             }

             // Sysex contains a bank of patches
             else if (prog > QUAD_NUM_PATCH)
             {
               for (prog = 0; prog < QUAD_NUM_PATCH; prog++)
               {
                 QuadGT_Decode_From_Sysex(sysex.buffer+offset, SYSEX_PATCH_SIZE, quadgt, QUAD_PATCH_SIZE);
                 QuadGT_Convert_QuadGT_To_Internal(prog, quadgt);

                 offset+= SYSEX_PATCH_SIZE;
               }
               QuadGT_Redraw_Patch(bank, 0);
             }
             else
             {
	       FormDebug->Log(NULL, "Data Dump Program: "+AnsiString(prog)+"   Bytes: "+AnsiString(sysex.length-offset));
             }
           }
           // TBD: Handle other SysEx data types
	 }
         else FormDebug->Log(NULL, "Not for Alesis Quad/QuadGT\n");
       }
       else FormDebug->Log(NULL, "Not for Alesis\n");
     }
     else FormDebug->Log(NULL, "No Sysex start\n");
   }
}

//---------------------------------------------------------------------------
// Name        : QuadGtBankLoadClick
// Description : Load a QuadGT bank file (as used by XXX program)
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void __fastcall TMainForm::QuadGtBankLoadClick(TObject *Sender)
{
  UInt8 prog;
  FILE *quadgt_file;
  tBuffer data;
  UInt32 size;

  if (!MainForm->QuadGtOpenDialog->Execute()) return;

  quadgt_file = fopen(MainForm->QuadGtOpenDialog->Files->Strings[0].c_str(),"rb");

  if (quadgt_file == NULL)
  {
    FormError->ShowErrorCode(ferror(quadgt_file),"opening QuadGT file");
    return;
  }

  // Determine size of the file
  fseek(quadgt_file, 0, SEEK_END);
  size=ftell(quadgt_file);
  rewind(quadgt_file);

  data.buffer=(UInt8 *)malloc(sizeof(UInt8) * size);
  if (data.buffer == NULL)
  {
      FormError->ShowErrorCode(1,"allocating memory for QuadGT file ");
  }
  else
  {
    if (fread(data.buffer, 1, size, quadgt_file) != size)
    {
      FormError->ShowErrorCode(ferror(quadgt_file),"reading QuadGT file ");
    }
    else
    {
      data.length = size;

      for (prog = 0; prog < QUAD_NUM_PATCH; prog++)
      {
        QuadGT_Convert_QuadGT_To_Internal(prog, data.buffer+(prog*QUAD_PATCH_SIZE));
      }
    }
  }

  fclose(quadgt_file);
  free(data.buffer);

  QuadGT_Redraw_Patch(bank, 0);
}

//---------------------------------------------------------------------------
// Name        : QuadGt_Swap
// Description : Swap two programs
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Swap(const int bank1, const int patch1, const int bank2, const int patch2)
{
  UInt8 prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  tQuadGT_Prog swap;

  memcpy(&swap,                        &QuadtGT_Bank[bank1][patch1], sizeof(tQuadGT_Prog));
  memcpy(&QuadtGT_Bank[bank1][patch1], &QuadtGT_Bank[bank2][patch2], sizeof(tQuadGT_Prog));
  memcpy(&QuadtGT_Bank[bank2][patch2], &swap,                        sizeof(tQuadGT_Prog));

  if ( ((bank == bank1) && (prog == patch1)) || ((bank == bank2) && (prog == patch2)) )
  {
    QuadGT_Redraw_Patch(bank, prog);
  }
}

typedef enum
{
  Param_Preamp,
  Param_Mix,
  Param_Mod,
  Param_Reverb,
  Param_Pitch,
  Param_Eq,
  Param_Resonator
} tParam;

//---------------------------------------------------------------------------
// Name        : QuadGt_Copy
// Description : Copy a set of parameters from one patch to another
// Parameter 1 : Parameter settings to copy
// Parameter 2 : Bank to copy from
// Parameter 3 : Patch to copy from
// Parameter 4 : Bank to copy to
// Parameter 5 : Patch to copy to
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Copy(const int param, const int bankfrom, const int patchfrom, const int bankto, const int patchto)
{
  UInt8 prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);

  FormDebug->Log(NULL,"Copy From Bank" + AnsiString(bankfrom) + " Patch " + AnsiString(patchfrom));
  FormDebug->Log(NULL,"Copy To   Bank" + AnsiString(bankto) + " Patch " + AnsiString(patchto));

  switch (param)
  {
    case Param_Preamp:
      QuadtGT_Bank[bankto][patchto].preamp.comp             = QuadtGT_Bank[bankfrom][patchfrom].preamp.comp;
      QuadtGT_Bank[bankto][patchto].preamp.od               = QuadtGT_Bank[bankfrom][patchfrom].preamp.od;
      QuadtGT_Bank[bankto][patchto].preamp.dist             = QuadtGT_Bank[bankfrom][patchfrom].preamp.dist;
      QuadtGT_Bank[bankto][patchto].preamp.preamp_tone      = QuadtGT_Bank[bankfrom][patchfrom].preamp.preamp_tone;
      QuadtGT_Bank[bankto][patchto].preamp.preamp_gate      = QuadtGT_Bank[bankfrom][patchfrom].preamp.preamp_gate;
      QuadtGT_Bank[bankto][patchto].preamp.preamp_out_level = QuadtGT_Bank[bankfrom][patchfrom].preamp.preamp_out_level;
      QuadtGT_Bank[bankto][patchto].preamp.effect_loop      = QuadtGT_Bank[bankfrom][patchfrom].preamp.effect_loop;
      QuadtGT_Bank[bankto][patchto].preamp.bass_boost       = QuadtGT_Bank[bankfrom][patchfrom].preamp.bass_boost;
      QuadtGT_Bank[bankto][patchto].preamp.cab_sim          = QuadtGT_Bank[bankfrom][patchfrom].preamp.cab_sim;
      break;

    case Param_Mix:
      QuadtGT_Bank[bankto][patchto].prepost_eq           = QuadtGT_Bank[bankfrom][patchfrom].prepost_eq;
      QuadtGT_Bank[bankto][patchto].direct_level         = QuadtGT_Bank[bankfrom][patchfrom].direct_level;
      QuadtGT_Bank[bankto][patchto].master_effects_level = QuadtGT_Bank[bankfrom][patchfrom].master_effects_level;
      QuadtGT_Bank[bankto][patchto].preamp_level         = QuadtGT_Bank[bankfrom][patchfrom].preamp_level;
      QuadtGT_Bank[bankto][patchto].pitch_level          = QuadtGT_Bank[bankfrom][patchfrom].pitch_level;
      QuadtGT_Bank[bankto][patchto].leslie_level         = QuadtGT_Bank[bankfrom][patchfrom].leslie_level;
      QuadtGT_Bank[bankto][patchto].ring_mod_level       = QuadtGT_Bank[bankfrom][patchfrom].ring_mod_level;
      QuadtGT_Bank[bankto][patchto].delay_level          = QuadtGT_Bank[bankfrom][patchfrom].delay_level;
      QuadtGT_Bank[bankto][patchto].reverb_level         = QuadtGT_Bank[bankfrom][patchfrom].reverb_level;
      QuadtGT_Bank[bankto][patchto].mix_mod              = QuadtGT_Bank[bankfrom][patchfrom].mix_mod;
      QuadtGT_Bank[bankto][patchto].eq_level             = QuadtGT_Bank[bankfrom][patchfrom].eq_level;
      break;

    case Param_Mod:
      // TBD
      break;

    case Param_Reverb:
      // TBD
      break;

    case Param_Pitch:
      // TBD
      break;

    case Param_Eq:
      // TBD
      break;

    case Param_Resonator:
      // TBD
      break;
  }

  if ( (bank == bankto) && (prog == patchto) )
  {
    QuadGT_Redraw_Patch(bank, prog);
  }
}
//---------------------------------------------------------------------------
