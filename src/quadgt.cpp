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

#include "quadgt.h"


static tQuadGT_Prog QuadGT_Progs[QUAD_NUM_PATCH];

void RedrawHorizBarTextU8(TTrackBar *bar, TEdit *text, UInt8 param);
void RedrawHorizBarTextS8(TTrackBar *bar, TEdit *text, SInt8 param);

void RedrawVertBarTextS8(TTrackBar *bar, TEdit *text, SInt8 param);
void RedrawVertBarTextU8(TTrackBar *bar, TEdit *text, UInt8 param, UInt8 offset);
void RedrawVertBarTextU16(TTrackBar *bar, TEdit *text, UInt16 param, UInt16 offset);

// 
//---------------------------------------------------------------------------
// Name        : 
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

// 
//---------------------------------------------------------------------------
// Name        : 
// Description : Decode Midi SysEx data in 7bits to Quadraverb data in 8bits
// Parameters  : 
// Returns     : NONE.
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
// Name        : 
// Description : Encode 8bit quadraverb data into low 7 bits for SysEx
// Parameters  : 
// Returns     : NONE.
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
// Description : 
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Init(void)
{
  UInt8 patch;
  char patch_name[NAME_LENGTH+1];
  TStringList *ModTgtReverb_A=new(TStringList);

  for (patch=0; patch<QUAD_NUM_PATCH; patch++)
  {
    // Set all parameters to 0
    memset(&QuadGT_Progs[patch], 0x00, sizeof(tQuadGT_Prog));

    // Create a program name
    sprintf(patch_name,"Patch-%2.2d",patch);
    strcpy(QuadGT_Progs[patch].name,patch_name);
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
//               the given program based on the data in QuadGT_Progs
// Parameter 1 : Program number for which display is to be updated
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Redraw_Patch(UInt8 prog)
{
  UInt8 config;

  /* Program Number */
  MainForm->QuadPatchNum->Text=AnsiString(prog);

  /* Program name */
  MainForm->EditQuadPatchName->Text=AnsiString(QuadGT_Progs[prog].name);

  /* Configuration */
  MainForm->QuadConfig->ItemIndex=QuadGT_Progs[prog].config;

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
//               the given program based on the data in QuadGT_Progs
// Parameter 1 : Program number for which display is to be updated
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Redraw_Reverb(const UInt8 prog)
{
  UInt8 val;

  if (QuadGT_Progs[prog].config==CFG0_EQ_PITCH_DELAY_REVERB)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=true;
    MainForm->ReverbInMix->Visible=true;
    MainForm->ReverbInMixVal->Visible=true;
  }
  else if ((QuadGT_Progs[prog].config==CFG1_LESLIE_DELAY_REVERB) || 
           (QuadGT_Progs[prog].config==CFG5_RINGMOD_DELAY_REVERB) || 
           (QuadGT_Progs[prog].config==CFG6_RESONATOR_DELAY_REVERB))
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=true;
    MainForm->ReverbInMix->Visible=true;
    MainForm->ReverbInMixVal->Visible=true;
    // TBD: Disable input 1, selection 3
  }
  else if ((QuadGT_Progs[prog].config==CFG2_GEQ_DELAY) || 
           (QuadGT_Progs[prog].config==CFG7_SAMPLING))
  {
    MainForm->QuadReverb->Visible=false;
  }
  else if (QuadGT_Progs[prog].config==CFG3_5BANDEQ_PITCH_DELAY)
  {
    MainForm->QuadReverb->Visible=false;
  }
  else if (QuadGT_Progs[prog].config==CFG4_3BANDEQ_REVERB)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=FALSE;
    MainForm->ReverbInMix->Visible=FALSE;
    MainForm->ReverbInMixVal->Visible=false;
  }

  if (MainForm->QuadReverb->Visible == true)
  {
    MainForm->ReverbType->ItemIndex = QuadGT_Progs[prog].reverb_mode;

    MainForm->ReverbInput1->ItemIndex = QuadGT_Progs[prog].reverb_input_1;
    MainForm->ReverbInput2->ItemIndex = QuadGT_Progs[prog].reverb_input_2;

    RedrawVertBarTextU8(MainForm->ReverbPreDelay, MainForm->ReverbPreDelayVal, QuadGT_Progs[prog].reverb_predelay, 1);
    
    RedrawHorizBarTextS8(MainForm->ReverbInMix, MainForm->ReverbInMixVal, QuadGT_Progs[prog].reverb_input_mix);
    RedrawHorizBarTextS8(MainForm->ReverbPreMix, MainForm->ReverbPreMixVal, QuadGT_Progs[prog].reverb_predelay_mix);

    RedrawVertBarTextU8(MainForm->ReverbLowDecay,  MainForm->ReverbLowDecayVal,  QuadGT_Progs[prog].reverb_low_decay, 0);
    RedrawVertBarTextU8(MainForm->ReverbHighDecay, MainForm->ReverbHighDecayVal, QuadGT_Progs[prog].reverb_high_decay, 0);
    RedrawVertBarTextU8(MainForm->ReverbDecay,     MainForm->ReverbDecayVal,     QuadGT_Progs[prog].reverb_decay, 0);

    RedrawVertBarTextU8(MainForm->ReverbDiffusion,  MainForm->ReverbDiffusionVal, QuadGT_Progs[prog].reverb_diffusion,0);

    RedrawVertBarTextU8(MainForm->ReverbDensity,  MainForm->ReverbDensityVal, QuadGT_Progs[prog].reverb_density,0);

    MainForm->ReverbGateOn->Checked = (QuadGT_Progs[prog].reverb_gate==1) ? TRUE : FALSE;
    RedrawVertBarTextU8(MainForm->ReverbGateHold,  MainForm->ReverbGateHoldVal, QuadGT_Progs[prog].reverb_gate_hold,0);
    RedrawVertBarTextU8(MainForm->ReverbGateRelease,  MainForm->ReverbGateReleaseVal, QuadGT_Progs[prog].reverb_gate_release,0);
    RedrawVertBarTextU8(MainForm->ReverbGateLevel,  MainForm->ReverbGateLevelVal, QuadGT_Progs[prog].reverb_gated_level,0);
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
  mode=QuadGT_Progs[prog].delay_mode;

  if (QuadGT_Progs[prog].config==CFG4_3BANDEQ_REVERB) 
  {
    MainForm->PanelQuadDelay->Visible = FALSE;
  }
  else
  {
    MainForm->PanelQuadDelay->Visible = TRUE;

    if (QuadGT_Progs[prog].config==CFG6_RESONATOR_DELAY_REVERB)
    {
      MainForm->PanelQuadDelay->Left=660;
    }
    else
    {
      MainForm->PanelQuadDelay->Left=420;
    }

    if (QuadGT_Progs[prog].config==CFG3_5BANDEQ_PITCH_DELAY)
    {
      // Add "Multitap" to list of possibilities
      int i=MainForm->DelayMode->Items->IndexOf("Multitap");
      if (i < 0) MainForm->DelayMode->Items->Add("Multitap");
      FormDebug->Log(NULL,"Add multitap: "+AnsiString(i));
    }
    else
    {
      // Remove "Multitap" from list of possibilities
      int i=MainForm->DelayMode->Items->IndexOf("Multitap");
      if (i >= 0) MainForm->DelayMode->Items->Delete(i);
      FormDebug->Log(NULL,"Hide multitap: "+AnsiString(i));
      if (mode == DELAYMODE3_MULTITAP)
      {
        mode=QuadGT_Progs[prog].delay_mode=DELAYMODE0_MONO;
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

  if ((QuadGT_Progs[prog].config==CFG0_EQ_PITCH_DELAY_REVERB) ||
      (QuadGT_Progs[prog].config==CFG2_GEQ_DELAY) ||
      (QuadGT_Progs[prog].config==CFG3_5BANDEQ_PITCH_DELAY))
  {
    MainForm->DelayInput->Visible = TRUE;
  }
  else if ((QuadGT_Progs[prog].config==CFG1_LESLIE_DELAY_REVERB) ||
      (QuadGT_Progs[prog].config==CFG5_RINGMOD_DELAY_REVERB) ||
      (QuadGT_Progs[prog].config==CFG6_RESONATOR_DELAY_REVERB))
  {
    MainForm->DelayInput->Visible = TRUE;
  }
  
  if (MainForm->PanelQuadDelay->Visible == TRUE)
  {
    MainForm->DelayMode->ItemIndex=mode;
    MainForm->DelayInput->ItemIndex=QuadGT_Progs[prog].delay_input;
    RedrawVertBarTextS8(MainForm->DelayInMix, MainForm->DelayInMixVal, QuadGT_Progs[prog].delay_input_mix);

    // TBD: is this fixed as reverb?
    MainForm->DelayInMix1->Caption = "Reverb";
    if (MainForm->DelayInput->ItemIndex == 0) MainForm->DelayInMix2->Caption = "Preamp";
    else  MainForm->DelayInMix2->Caption = "Eq";
 
    if ((mode == DELAYMODE0_MONO) || (mode == DELAYMODE2_PINGPONG))
    {
      RedrawVertBarTextU16(MainForm->DelayLeft, MainForm->DelayLeftVal, QuadGT_Progs[prog].delay_left,0);
      RedrawVertBarTextU8(MainForm->DelayLeftFBack, MainForm->DelayLeftFBackVal, QuadGT_Progs[prog].delay_left_feedback,0);
      // TBD: Set different max delay times dependenat on delay mode and configuration
    }
    else if (mode == DELAYMODE1_STEREO)
    {
      RedrawVertBarTextU16(MainForm->DelayLeft, MainForm->DelayLeftVal, QuadGT_Progs[prog].delay_left,0);
      RedrawVertBarTextU8(MainForm->DelayLeftFBack, MainForm->DelayLeftFBackVal, QuadGT_Progs[prog].delay_left_feedback,0);
      RedrawVertBarTextU16(MainForm->DelayRight, MainForm->DelayRightVal, QuadGT_Progs[prog].delay_right,0);
      RedrawVertBarTextU8(MainForm->DelayRighFBack, MainForm->DelayRightFBackVal, QuadGT_Progs[prog].delay_right_feedback,0);
    }
    else if (mode == DELAYMODE3_MULTITAP)
    {
      UInt8 tap = StrToInt(MainForm->TapNumber->Text)-1;

      RedrawVertBarTextU8(MainForm->TapDelay,    MainForm->TapDelayVal, QuadGT_Progs[prog].tap_delay[tap], 0);
      RedrawVertBarTextU8(MainForm->TapPan,      MainForm->TapPanVal, QuadGT_Progs[prog].tap_pan[tap], 0);
      RedrawVertBarTextU8(MainForm->TapVol,      MainForm->TapVolVal, QuadGT_Progs[prog].tap_volume[tap], 0);
      RedrawVertBarTextU8(MainForm->TapFeedback, MainForm->TapFeedbackVal, QuadGT_Progs[prog].tap_feedback[tap], 0);
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
  if ((QuadGT_Progs[prog].config==CFG0_EQ_PITCH_DELAY_REVERB) ||
      (QuadGT_Progs[prog].config==CFG3_5BANDEQ_PITCH_DELAY))
  {
    MainForm->QuadPitch->Visible=TRUE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;

    MainForm->PitchInput->Visible = TRUE;

    MainForm->PitchMode->ItemIndex = QuadGT_Progs[prog].pitch_mode;
    MainForm->PitchInput->ItemIndex = QuadGT_Progs[prog].pitch_input;
    MainForm->PitchWave->ItemIndex = QuadGT_Progs[prog].lfo_waveform;

    FormDebug->Log(NULL,"Redraw pitch");
    RedrawVertBarTextU8(MainForm->PitchSpeed,    MainForm->PitchSpeedVal,    QuadGT_Progs[prog].lfo_speed, 0);
    RedrawVertBarTextU8(MainForm->PitchDepth,    MainForm->PitchDepthVal,    QuadGT_Progs[prog].lfo_depth, 0);
    RedrawVertBarTextU8(MainForm->PitchFeedback, MainForm->PitchFeedbackVal, QuadGT_Progs[prog].pitch_feedback, 0);
  }
  else if (QuadGT_Progs[prog].config==CFG1_LESLIE_DELAY_REVERB)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=TRUE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;

    MainForm->PitchInput->Visible = FALSE;

    // TBD: Redraw Leslie parameters
    FormDebug->Log(NULL,"Redraw leslie");
  }
  else if ((QuadGT_Progs[prog].config==CFG2_GEQ_DELAY) || 
           (QuadGT_Progs[prog].config==CFG6_RESONATOR_DELAY_REVERB) || 
           (QuadGT_Progs[prog].config==CFG7_SAMPLING))
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;

    MainForm->PitchInput->Visible = FALSE;

    FormDebug->Log(NULL,"Redraw no-pitch");
  }
  else if (QuadGT_Progs[prog].config==CFG4_3BANDEQ_REVERB)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=TRUE;

    // Redraw Reverb Chorus parameters
    RedrawVertBarTextU8(MainForm->ChorusSpeed, MainForm->ChorusSpeedVal, QuadGT_Progs[prog].lfo_speed, 1);
    RedrawVertBarTextU8(MainForm->ChorusDepth, MainForm->ChorusDepthVal, QuadGT_Progs[prog].lfo_depth, 1);
    FormDebug->Log(NULL,"Redraw reverb chorus");

  }
  else if (QuadGT_Progs[prog].config==CFG5_RINGMOD_DELAY_REVERB)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=TRUE;
    MainForm->QuadChorus->Visible=FALSE;

    // TBD: Redraw Ring Mod parameters
    RedrawVertBarTextU16(MainForm->RingModSS,    MainForm->RingModSSVal,    QuadGT_Progs[prog].ring_mod_shift, 0);
    RedrawVertBarTextS8(MainForm->RingModInMix, MainForm->RingModInMixVal, QuadGT_Progs[prog].ring_mod_in_mix);
    RedrawVertBarTextS8(MainForm->RingModOutMix, MainForm->RingModOutMixVal, QuadGT_Progs[prog].ring_mod_out_mix);
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
// Description : 
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Redraw_Eq(const UInt8 prog)
{
  // 3 Band Eq
  if ((QuadGT_Progs[prog].config==CFG0_EQ_PITCH_DELAY_REVERB) || (QuadGT_Progs[prog].config==CFG4_3BANDEQ_REVERB))
  {
    MainForm->PanelQuadEq3->Visible=TRUE;

    MainForm->EqFreq1->Position = QuadGT_Progs[prog].low_eq_freq;
    MainForm->EqFreq1Val->Text = AnsiString(QuadGT_Progs[prog].low_eq_freq);
    MainForm->EqAmp1->Position = -1 * (QuadGT_Progs[prog].low_eq_amp - MainForm->EqAmp1->Max);
    MainForm->Eq3Amp1Val->Text = AnsiString(((double)(QuadGT_Progs[prog].low_eq_amp - MainForm->EqAmp1->Max))/20.0);

    MainForm->EqFreq2->Position = QuadGT_Progs[prog].mid_eq_freq;
    MainForm->EqFreq2Val->Text = AnsiString(QuadGT_Progs[prog].mid_eq_freq);
    MainForm->EqAmp2->Position = -1 * (QuadGT_Progs[prog].mid_eq_amp - MainForm->EqAmp2->Max);
    MainForm->Eq3Amp2Val->Text = AnsiString(((double)(QuadGT_Progs[prog].mid_eq_amp - MainForm->EqAmp2->Max))/20.0);
    MainForm->EqQ2->Position = QuadGT_Progs[prog].mid_eq_q;
    MainForm->EqQ2Val->Text = AnsiString(QuadGT_Progs[prog].mid_eq_q);

    MainForm->EqFreq3->Position = QuadGT_Progs[prog].high_eq_freq;
    MainForm->EqFreq3Val->Text = AnsiString(QuadGT_Progs[prog].high_eq_freq);
    MainForm->EqAmp3->Position = -1 * (QuadGT_Progs[prog].high_eq_amp - MainForm->EqAmp3->Max);
    MainForm->Eq3Amp3Val->Text = AnsiString(((double)(QuadGT_Progs[prog].high_eq_amp - MainForm->EqAmp3->Max))/20.0);
    if (QuadGT_Progs[prog].config==CFG0_EQ_PITCH_DELAY_REVERB) MainForm->Eq3Mode->Visible=TRUE;
    else                                                       MainForm->Eq3Mode->Visible=FALSE;
  }
  else
  {
    MainForm->PanelQuadEq3->Visible=FALSE;
  }

  // 5 Band Eq
  if (QuadGT_Progs[prog].config==CFG3_5BANDEQ_PITCH_DELAY) 
  {
    MainForm->QuadEq5->Visible=TRUE;

    MainForm->Eq5Freq1->Position = QuadGT_Progs[prog].low_eq_freq;
    MainForm->Eq5Freq1Val->Text = AnsiString(QuadGT_Progs[prog].low_eq_freq);
    MainForm->Eq5Amp1->Position = -1 * (QuadGT_Progs[prog].low_eq_amp - MainForm->Eq5Amp1->Max);
    MainForm->Eq5Amp1Val->Text = AnsiString(((double)(QuadGT_Progs[prog].low_eq_amp - MainForm->Eq5Amp1->Max))/20.0);

    MainForm->Eq5Freq2->Position = QuadGT_Progs[prog].low_mid_eq_freq;
    MainForm->Eq5Freq2Val->Text = AnsiString(QuadGT_Progs[prog].low_mid_eq_freq);
    MainForm->Eq5Amp2->Position = -1 * (QuadGT_Progs[prog].low_mid_eq_amp - MainForm->Eq5Amp2->Max);
    MainForm->Eq5Amp2Val->Text = AnsiString((double)(QuadGT_Progs[prog].low_mid_eq_amp - MainForm->Eq5Amp2->Max)/20.0);
    MainForm->Eq5Q2->Position = QuadGT_Progs[prog].low_mid_eq_q;
    MainForm->Eq5Q2Val->Text = AnsiString(QuadGT_Progs[prog].low_mid_eq_q);

    MainForm->Eq5Freq3->Position = QuadGT_Progs[prog].mid_eq_freq;
    MainForm->Eq5Freq3Val->Text = AnsiString(QuadGT_Progs[prog].mid_eq_freq);
    MainForm->Eq5Amp3->Position = -1 * (QuadGT_Progs[prog].mid_eq_amp - MainForm->Eq5Amp3->Max);
    MainForm->Eq5Amp3Val->Text = AnsiString((double)(QuadGT_Progs[prog].mid_eq_amp - MainForm->Eq5Amp3->Max)/20.0);
    MainForm->Eq5Q3->Position = QuadGT_Progs[prog].mid_eq_q;
    MainForm->Eq5Q3Val->Text = AnsiString(QuadGT_Progs[prog].mid_eq_q);

    MainForm->Eq5Freq4->Position = QuadGT_Progs[prog].high_mid_eq_freq;
    MainForm->Eq5Freq4Val->Text = AnsiString(QuadGT_Progs[prog].high_mid_eq_freq);
    MainForm->Eq5Amp4->Position = -1 * (QuadGT_Progs[prog].high_mid_eq_amp - MainForm->Eq5Amp4->Max);
    MainForm->Eq5Amp4Val->Text = AnsiString((double)(QuadGT_Progs[prog].high_mid_eq_amp - MainForm->Eq5Amp4->Max)/20.0);
    MainForm->Eq5Q4->Position = QuadGT_Progs[prog].high_mid_eq_q;
    MainForm->Eq5Q4Val->Text = AnsiString(QuadGT_Progs[prog].high_mid_eq_q);

    MainForm->Eq5Freq5->Position = QuadGT_Progs[prog].high_eq_freq;
    MainForm->Eq5Freq5Val->Text = AnsiString(QuadGT_Progs[prog].high_eq_freq);
    MainForm->Eq5Amp5->Position = -1 * (QuadGT_Progs[prog].high_eq_amp - MainForm->Eq5Amp5->Max);
    MainForm->Eq5Amp5Val->Text = AnsiString((double)(QuadGT_Progs[prog].high_eq_amp - MainForm->Eq5Amp5->Max)/20.0);
  }
  else 
  {
    MainForm->QuadEq5->Visible=FALSE;
  }

  // Graphic Eq
  if (QuadGT_Progs[prog].config==CFG2_GEQ_DELAY) 
  {
    MainForm->QuadGraphEq->Visible=TRUE;

    MainForm->GEQPreset->ItemIndex =QuadGT_Progs[prog].eq_preset;

    // TBD: If using preset, disable sliders and display preset amplitudes
   
    MainForm->GEQ_16Hz->Position =-1*QuadGT_Progs[prog].geq_16hz  ;
    MainForm->GEQ_32Hz->Position =-1*QuadGT_Progs[prog].geq_32hz  ;
    MainForm->GEQ_62Hz->Position =-1*QuadGT_Progs[prog].geq_62hz  ;
    MainForm->GEQ_126Hz->Position=-1*QuadGT_Progs[prog].geq_126hz ;
    MainForm->GEQ_250Hz->Position=-1*QuadGT_Progs[prog].geq_250hz ;
    MainForm->GEQ_500Hz->Position=-1*QuadGT_Progs[prog].geq_500hz ;
    MainForm->GEQ_1KHz->Position =-1*QuadGT_Progs[prog].geq_1khz  ;
    MainForm->GEQ_2KHz->Position =-1*QuadGT_Progs[prog].geq_2khz  ;
    MainForm->GEQ_4KHz->Position =-1*QuadGT_Progs[prog].geq_4khz  ;
    MainForm->GEQ_8KHz->Position =-1*QuadGT_Progs[prog].geq_8khz  ;
    MainForm->GEQ_16KHz->Position=-1*QuadGT_Progs[prog].geq_16khz ;

  }
  else
  {
    MainForm->QuadGraphEq->Visible=FALSE;
  }

  // Resonator
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
  MainForm->MixDirect->Position=MainForm->MixDirect->Max - QuadGT_Progs[prog].direct_level;
  MainForm->MixDirectVal->Text=AnsiString(QuadGT_Progs[prog].direct_level);

  // Master Fx Level
  if (QuadGT_Progs[prog].config != 7)
  {
    val=QuadGT_Progs[prog].master_effects_level;
    MainForm->MixMaster->Position=99-val;
    MainForm->MixMasterVal->Text=AnsiString(QuadGT_Progs[prog].master_effects_level);
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
  MainForm->PrePostEq->ItemIndex=QuadGT_Progs[prog].prepost_eq;
 
  QuadGT_Redraw_PrePostEq(prog);

  // Pitch Level
  if ( (QuadGT_Progs[prog].config == CFG0_EQ_PITCH_DELAY_REVERB) ||
       (QuadGT_Progs[prog].config == CFG3_5BANDEQ_PITCH_DELAY))
  {
    val=QuadGT_Progs[prog].pitch_level;
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
  val=QuadGT_Progs[prog].delay_level;
  MainForm->MixDelay->Position=99-val;
  MainForm->MixDelayVal->Text=AnsiString(val);

  // Reverb Level
  val=QuadGT_Progs[prog].reverb_level;
  MainForm->MixReverb->Position=99-val;
  MainForm->MixReverbVal->Text=AnsiString(val);

  val=QuadGT_Progs[prog].mix_mod;
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
  val=QuadGT_Progs[prog].pan_depth;
  MainForm->ModDepth->Position=99-val;
  MainForm->ModDepthVal->Text=AnsiString(val);

  // Mod Speed
  val=QuadGT_Progs[prog].pan_speed;
  MainForm->ModSpeed->Position=99-val;
  MainForm->ModSpeedVal->Text=AnsiString(val);
 
  // Leslie Level
  if (QuadGT_Progs[prog].config == CFG1_LESLIE_DELAY_REVERB)
  {
    val=QuadGT_Progs[prog].leslie_level;
    MainForm->MixLeslieRingModReson->Position=99-val;
    MainForm->MixLeslieRingModResonVal->Text=AnsiString(val);
    MainForm->MixLeslieRingModReson->Visible=TRUE;
    MainForm->LeslieRingModReson->Caption="Leslie";
    MainForm->LeslieRingModReson->Visible=TRUE;
    MainForm->MixLeslieRingModResonVal->Visible=TRUE;
  }

  // Ring Level
  else if (QuadGT_Progs[prog].config == CFG5_RINGMOD_DELAY_REVERB)
  {
    val=QuadGT_Progs[prog].ring_mod_level;
    MainForm->MixLeslieRingModReson->Position=99-val;
    MainForm->MixLeslieRingModResonVal->Text=AnsiString(val);
    MainForm->MixLeslieRingModReson->Visible=TRUE;
    MainForm->LeslieRingModReson->Caption="RingMod";
    MainForm->LeslieRingModReson->Visible=TRUE;
    MainForm->MixLeslieRingModResonVal->Visible=TRUE;
  }
  
  // Resonator Level
  else if (QuadGT_Progs[prog].config == CFG6_RESONATOR_DELAY_REVERB)
  {
    //val=QuadGT_Progs[prog].TBD;
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

  if (QuadGT_Progs[prog].config != CFG7_SAMPLING)
  {
    mod = StrToInt(MainForm->ModNumber->Text)-1;
    MainForm->ModSource->Text=AnsiString(QuadGT_Progs[prog].mod_source[mod]);
    MainForm->ModTarget->ItemIndex=QuadGT_Progs[prog].mod_target[mod];
    RedrawHorizBarTextS8( MainForm->ModAmp,  MainForm->ModAmpVal,QuadGT_Progs[prog].mod_amp[mod]);

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

void QuadGT_Redraw_Preamp(const UInt8 prog)
{
  UInt8 val;

  // Compression (0-7)
  MainForm->PreComp->Position = MainForm->PreComp->Max - QuadGT_Progs[prog].comp;
  MainForm->PreCompVal->Text = AnsiString(QuadGT_Progs[prog].comp);

  // Overdrive (0-7)
  MainForm->PreOd->Position = MainForm->PreOd->Max - QuadGT_Progs[prog].od;
  MainForm->PreOdVal->Text = AnsiString(QuadGT_Progs[prog].od);

  // Distortion (0-8)
  MainForm->PreDist->Position = MainForm->PreDist->Max - QuadGT_Progs[prog].dist;
  MainForm->PreDistVal->Text = AnsiString(QuadGT_Progs[prog].dist);

  // Tone (0-2 Flat, Presence, Bright)
  MainForm->PreTone->ItemIndex=QuadGT_Progs[prog].preamp_tone;

  // Bass Boost (0-1 On, Off)
  MainForm->BassBoost->Checked = QuadGT_Progs[prog].bass_boost == 1 ? TRUE : FALSE;

  // Cab sim (0-2 Off, Cab 1, Cab 2)
  MainForm->CabSim->ItemIndex=QuadGT_Progs[prog].cab_sim;
  
  // Effect Loop (0-1)
  MainForm->EffectLoopIn->Checked = QuadGT_Progs[prog].effect_loop == 1 ? TRUE : FALSE;

  // Noise Gate (0-17)
  MainForm->PreGate->Position=MainForm->PreGate->Max-QuadGT_Progs[prog].preamp_gate;
  MainForm->PreGateVal->Text = AnsiString(QuadGT_Progs[prog].preamp_gate);

  /* Preamp Output Level (0-99) */
  if (MainForm->QuadConfig->ItemIndex == CFG7_SAMPLING)
  {
    MainForm->PreOutLevel->Visible = FALSE;
    MainForm->PreOutVal->Visible = FALSE;
    MainForm->PreOutLabel->Visible = FALSE;
  }
  else
  {
    MainForm->PreOutLevel->Position = MainForm->PreOutLevel->Max-QuadGT_Progs[prog].preamp_out_level;
    MainForm->PreOutVal->Text = AnsiString(QuadGT_Progs[prog].preamp_out_level);
    MainForm->PreOutLevel->Visible = TRUE;
    MainForm->PreOutVal->Visible = TRUE;
    MainForm->PreOutLabel->Visible = TRUE;
  }

}


void RedrawHorizBarTextU8(TTrackBar *bar, TEdit *text, UInt8 param)
{
  bar->Position=param;
  bar->Hint=AnsiString(param);
  text->Text=AnsiString(param);
}
void RedrawHorizBarTextS8(TTrackBar *bar, TEdit *text, SInt8 param)
{
  bar->Position=param;
  bar->Hint=AnsiString(param);
  text->Text=AnsiString(param);
}
void RedrawVertBarTextS8(TTrackBar *bar, TEdit *text, SInt8 param)
{
  bar->Position=param;
  bar->Hint=AnsiString(param);
  text->Text=AnsiString(param);
}

void RedrawVertBarTextU8(TTrackBar *bar, TEdit *text, UInt8 param, UInt8 offset)
{
  bar->Position=bar->Max-(param+offset);
  bar->Hint=AnsiString(param+offset);
  text->Text=AnsiString(param+offset);
}

void RedrawVertBarTextU16(TTrackBar *bar, TEdit *text, UInt16 param, UInt16 offset)
{
  bar->Position=bar->Max-(param+offset);
  bar->Hint=AnsiString(param+offset);
  text->Text=AnsiString(param+offset);
}

//---------------------------------------------------------------------------
// Name        : EqBarChange
// Description : Update the parameter, hint text and text field associated 
//               with a vertical Eq frequency bar.
// Parameter 1 : Pointer to bar object
// Parameter 2 : Pointer to text object for numeric value
// Parameter 3 : Pointer to parameter to update
// Returns     : NONE.
//---------------------------------------------------------------------------
void EqBarChange(TTrackBar *bar, TEdit *text, UInt16* param)
{
  *param= -1 * bar->Position + bar->Max;       // Convert to range: 0 to 560
  bar->Hint=AnsiString(bar->Position/-20.0);   // Display in range: -14 to 14
  text->Text=AnsiString(bar->Position/-20.0);  // Display value as text
}

void VertBarChange(TTrackBar *bar, UInt8* param)
{
  *param=(bar->Max - bar->Position);
  bar->Hint=AnsiString(bar->Max - bar->Position);
}

void VertBarChangeS8(TTrackBar *bar, TEdit *text, SInt8* param)
{
  *param=-1*bar->Position;
  bar->Hint=AnsiString(*param);
  if (text != NULL)
  {
    text->Text=AnsiString(*param);
  }
}
void VertBarChangeU8(TTrackBar *bar, TEdit *text, UInt8* param)
{
  *param=(bar->Max - bar->Position);
  bar->Hint=AnsiString(*param);
  if (text != NULL)
  {
    text->Text=AnsiString(*param);
  }
}

void VertBarChangeU16(TTrackBar *bar, TEdit *text, UInt16* param)
{
  *param=(bar->Max - bar->Position);
  bar->Hint=AnsiString(*param);
  if (text != NULL)
  {
    text->Text=AnsiString(*param);
  }
}
void VertBarChangeOffsetU8(TTrackBar *bar, TEdit *text, UInt8* param, UInt8 offset)
{
  *param=(bar->Max - bar->Position);
  bar->Hint=AnsiString(*param+offset);
  if (text != NULL)
  {
    text->Text=AnsiString(*param+offset);
  }
}

void HorizBarChange(TTrackBar *bar, TEdit *text, UInt16* param)
{
  *param= bar->Position;
  bar->Hint=AnsiString(*param);
  if (text != NULL)
  {
    text->Text=AnsiString(*param);
  }
}

void HorizBarChangeU8(TTrackBar *bar, TEdit *text, UInt8* param)
{
  *param= bar->Position;
  bar->Hint=AnsiString(*param);
  if (text != NULL)
  {
    text->Text=AnsiString(*param);
  }
}

void HorizBarChangeS8(TTrackBar *bar, TEdit *text, SInt8* param)
{
  *param= bar->Position;
  bar->Hint=AnsiString(*param);
  if (text != NULL)
  {
    text->Text=AnsiString(*param);
  }
}
void QuadGT_Redraw_PrePostEq(UInt8 prog)
{
  // Preamp Level
  if (MainForm->PrePostEq->ItemIndex==0)
  {
    MainForm->MixPreampEq->Position=99-QuadGT_Progs[prog].preamp_level;
    MainForm->MixPreampEq->Visible=TRUE;
    MainForm->MixPreampEqVal->Text=AnsiString(QuadGT_Progs[prog].preamp_level);
    MainForm->PreAmpEq->Visible=TRUE;
    MainForm->PreAmpEq->Caption="Preamp";
  }
  // Eq Level
  else
  {
    MainForm->MixPreampEq->Position=99-QuadGT_Progs[prog].eq_level;
    MainForm->MixPreampEq->Visible=TRUE;
    MainForm->MixPreampEqVal->Text=AnsiString(QuadGT_Progs[prog].eq_level);
    MainForm->PreAmpEq->Visible=TRUE;
    MainForm->PreAmpEq->Caption="Eq";
  }
}

void __fastcall TMainForm::QuadParamChange(TObject *Sender)
{
  UInt8 prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  UInt8 tap = StrToInt(MainForm->TapNumber->Text)-1;
  //FormDebug->Log(NULL,"Change: "+AnsiString(prog));

  // Patch number and name
  if (Sender == MainForm->QuadConfig)
  {
    QuadGT_Progs[prog].config=MainForm->QuadConfig->ItemIndex;
    QuadGT_Redraw_Patch(prog);
  }
  else if (Sender == MainForm->EditQuadPatchName)
  {
    strncpy(QuadGT_Progs[prog].name,MainForm->EditQuadPatchName->Text.c_str(),NAME_LENGTH);
  }

  // Pre-amp parameters
  else if (Sender == MainForm->PreComp) VertBarChangeU8((TTrackBar *)Sender, MainForm->PreCompVal, &QuadGT_Progs[prog].comp);
  else if (Sender == MainForm->PreOd)   VertBarChangeU8((TTrackBar *)Sender, MainForm->PreOdVal, &QuadGT_Progs[prog].od);
  else if (Sender == MainForm->PreDist) VertBarChangeU8((TTrackBar *)Sender, MainForm->PreDistVal, &QuadGT_Progs[prog].dist);
  else if (Sender == MainForm->PreGate) VertBarChangeU8((TTrackBar *)Sender, MainForm->PreGateVal, &QuadGT_Progs[prog].preamp_gate);
  else if (Sender == MainForm->PreOutLevel) VertBarChangeU8((TTrackBar *)Sender, MainForm->PreOutVal, &QuadGT_Progs[prog].preamp_out_level);

  // 3 Band Eq
  // TBD: EqFreq1 EqFreq2 EqQ2 EqFreq3
  else if (Sender == MainForm->Eq3Mode)
  {
    QuadGT_Progs[prog].eq_mode = MainForm->Eq3Mode->ItemIndex;
    QuadGT_Redraw_Eq(prog);
    QuadGT_Redraw_Resonator(prog);
  }
  else if (Sender == MainForm->EqAmp1) EqBarChange(MainForm->EqAmp1, MainForm->Eq3Amp1Val, &QuadGT_Progs[prog].low_eq_amp);
  else if (Sender == MainForm->EqAmp2) EqBarChange(MainForm->EqAmp2, MainForm->Eq3Amp2Val, &QuadGT_Progs[prog].mid_eq_amp);
  else if (Sender == MainForm->EqAmp3) EqBarChange(MainForm->EqAmp3, MainForm->Eq3Amp3Val, &QuadGT_Progs[prog].high_eq_amp);

  else if (Sender == MainForm->EqFreq1) HorizBarChange(MainForm->EqFreq1, MainForm->EqFreq1Val, &QuadGT_Progs[prog].low_eq_freq);
  else if (Sender == MainForm->EqFreq2) HorizBarChange(MainForm->EqFreq2, MainForm->EqFreq2Val, &QuadGT_Progs[prog].mid_eq_freq);
  else if (Sender == MainForm->EqFreq3) HorizBarChange(MainForm->EqFreq3, MainForm->EqFreq3Val, &QuadGT_Progs[prog].high_eq_freq);

  else if (Sender == MainForm->EqQ2) HorizBarChangeU8(MainForm->EqQ2, MainForm->EqQ2Val, &QuadGT_Progs[prog].mid_eq_q);

  // 5 Band Eq
  else if (Sender == MainForm->Eq5Mode)
  {
    QuadGT_Progs[prog].eq_mode = MainForm->Eq5Mode->ItemIndex;
    QuadGT_Redraw_Eq(prog);
    QuadGT_Redraw_Resonator(prog);
  }
  else if (Sender == MainForm->Eq5Amp1) EqBarChange(MainForm->Eq5Amp1, MainForm->Eq5Amp1Val, &QuadGT_Progs[prog].low_eq_amp);
  else if (Sender == MainForm->Eq5Amp2) EqBarChange(MainForm->Eq5Amp2, MainForm->Eq5Amp2Val, &QuadGT_Progs[prog].low_mid_eq_amp);
  else if (Sender == MainForm->Eq5Amp3) EqBarChange(MainForm->Eq5Amp3, MainForm->Eq5Amp3Val, &QuadGT_Progs[prog].mid_eq_amp);
  else if (Sender == MainForm->Eq5Amp4) EqBarChange(MainForm->Eq5Amp4, MainForm->Eq5Amp4Val, &QuadGT_Progs[prog].high_mid_eq_amp);
  else if (Sender == MainForm->Eq5Amp5) EqBarChange(MainForm->Eq5Amp5, MainForm->Eq5Amp5Val, &QuadGT_Progs[prog].high_eq_amp);

  else if (Sender == MainForm->Eq5Freq1) HorizBarChange(MainForm->Eq5Freq1, MainForm->Eq5Freq1Val, &QuadGT_Progs[prog].low_eq_freq);
  else if (Sender == MainForm->Eq5Freq2) HorizBarChange(MainForm->Eq5Freq2, MainForm->Eq5Freq2Val, &QuadGT_Progs[prog].low_mid_eq_freq);
  else if (Sender == MainForm->Eq5Freq3) HorizBarChange(MainForm->Eq5Freq3, MainForm->Eq5Freq3Val, &QuadGT_Progs[prog].mid_eq_freq);
  else if (Sender == MainForm->Eq5Freq4) HorizBarChange(MainForm->Eq5Freq4, MainForm->Eq5Freq4Val, &QuadGT_Progs[prog].high_mid_eq_freq);
  else if (Sender == MainForm->Eq5Freq5) HorizBarChange(MainForm->Eq5Freq5, MainForm->Eq5Freq5Val, &QuadGT_Progs[prog].high_eq_freq);

  else if (Sender == MainForm->Eq5Q2) HorizBarChangeU8(MainForm->Eq5Q2, MainForm->Eq5Q2Val, &QuadGT_Progs[prog].low_mid_eq_q);
  else if (Sender == MainForm->Eq5Q3) HorizBarChangeU8(MainForm->Eq5Q3, MainForm->Eq5Q3Val, &QuadGT_Progs[prog].mid_eq_q);
  else if (Sender == MainForm->Eq5Q4) HorizBarChangeU8(MainForm->Eq5Q4, MainForm->Eq5Q4Val, &QuadGT_Progs[prog].high_mid_eq_q);

  // Mix parameters
  else if (Sender == MainForm->MixDirect) VertBarChangeU8((TTrackBar *)Sender, MainForm->MixDirectVal, &QuadGT_Progs[prog].direct_level);
  else if (Sender == MainForm->MixMaster) VertBarChangeU8((TTrackBar *)Sender, MainForm->MixMasterVal, &QuadGT_Progs[prog].master_effects_level);
  else if (Sender == MainForm->MixPitch)  VertBarChangeU8((TTrackBar *)Sender, MainForm->MixPitchVal,  &QuadGT_Progs[prog].pitch_level);
  else if (Sender == MainForm->PrePostEq) 
  {
    QuadGT_Progs[prog].prepost_eq=MainForm->PrePostEq->ItemIndex;
    QuadGT_Redraw_PrePostEq(prog);
  }
  else if (Sender == MainForm->MixPreampEq) 
  {
    if (QuadGT_Progs[prog].prepost_eq==0) VertBarChangeU8((TTrackBar *)Sender, MainForm->MixPreampEqVal, &QuadGT_Progs[prog].preamp_level);
    else                                  VertBarChangeU8((TTrackBar *)Sender, MainForm->MixPreampEqVal, &QuadGT_Progs[prog].eq_level);
  }
  else if (Sender == MainForm->MixDelay) VertBarChangeU8((TTrackBar *)Sender, MainForm->MixDelayVal, &QuadGT_Progs[prog].delay_level);
  else if (Sender == MainForm->MixReverb) VertBarChangeU8((TTrackBar *)Sender, MainForm->MixReverbVal, &QuadGT_Progs[prog].reverb_level);
  else if (Sender == MainForm->MixModNone) 
  {
    QuadGT_Progs[prog].mix_mod=0;
    MainForm->ModDepth->Enabled=FALSE;
    MainForm->ModDepthVal->Enabled=FALSE;
    MainForm->Depth->Enabled=FALSE;
    MainForm->ModSpeed->Enabled=FALSE;
    MainForm->ModSpeedVal->Enabled=FALSE;
    MainForm->Speed->Enabled=FALSE;
  }
  else if (Sender == MainForm->MixModTrem) 
  {
    QuadGT_Progs[prog].mix_mod=1;
    MainForm->ModDepth->Enabled=TRUE;
    MainForm->ModDepthVal->Enabled=TRUE;
    MainForm->Depth->Enabled=TRUE;
    MainForm->ModSpeed->Enabled=TRUE;
    MainForm->ModSpeedVal->Enabled=TRUE;
    MainForm->Speed->Enabled=TRUE;
  }
  else if (Sender == MainForm->MixModPan) 
  {
    QuadGT_Progs[prog].mix_mod=2;
    MainForm->ModDepth->Enabled=TRUE;
    MainForm->ModDepthVal->Enabled=TRUE;
    MainForm->Depth->Enabled=TRUE;
    MainForm->ModSpeed->Enabled=TRUE;
    MainForm->ModSpeedVal->Enabled=TRUE;
    MainForm->Speed->Enabled=TRUE;
  }
  else if (Sender == MainForm->ModSpeed) VertBarChangeU8((TTrackBar *)Sender, MainForm->ModSpeedVal, &QuadGT_Progs[prog].pan_speed);
  else if (Sender == MainForm->ModDepth) VertBarChangeU8((TTrackBar *)Sender, MainForm->ModDepthVal, &QuadGT_Progs[prog].pan_depth);
  else if (Sender == MainForm->MixLeslieRingModReson)
  {
    if (QuadGT_Progs[prog].config == CFG1_LESLIE_DELAY_REVERB)
      VertBarChangeU8((TTrackBar *)Sender, MainForm->MixLeslieRingModResonVal, &QuadGT_Progs[prog].leslie_level);
    else if (QuadGT_Progs[prog].config == CFG5_RINGMOD_DELAY_REVERB)
      VertBarChangeU8((TTrackBar *)Sender, MainForm->MixLeslieRingModResonVal, &QuadGT_Progs[prog].ring_mod_level);
    //else if (QuadGT_Progs[prog].config == CFG6_RESONATOR_DELAY_REVERB)
    //  VertBarChangeU8((TTrackBar *)Sender, MainForm->MixLeslieRingModResonVal, &QuadGT_Progs[prog].TBD);

  }

  // Pitch parameters
  else if (Sender == MainForm->PitchMode) { QuadGT_Progs[prog].pitch_mode=MainForm->PitchMode->ItemIndex; QuadGT_Redraw_Pitch(prog);}
  else if (Sender == MainForm->PitchInput){ QuadGT_Progs[prog].pitch_input=MainForm->PitchInput->ItemIndex;  QuadGT_Redraw_Pitch(prog);}
  else if (Sender == MainForm->PitchWave) { QuadGT_Progs[prog].lfo_waveform=MainForm->PitchWave->ItemIndex; QuadGT_Redraw_Pitch(prog);}
  else if (Sender == MainForm->PitchSpeed) VertBarChangeU8((TTrackBar *)Sender, MainForm->PitchSpeedVal, &QuadGT_Progs[prog].lfo_speed);
  else if (Sender == MainForm->PitchDepth) VertBarChangeU8((TTrackBar *)Sender, MainForm->PitchDepthVal, &QuadGT_Progs[prog].lfo_depth);
  else if (Sender == MainForm->PitchFeedback) VertBarChangeU8((TTrackBar *)Sender, MainForm->PitchFeedbackVal, &QuadGT_Progs[prog].pitch_feedback);

  else if (Sender == MainForm->ChorusSpeed) VertBarChangeOffsetU8((TTrackBar *)Sender, MainForm->ChorusSpeedVal, &QuadGT_Progs[prog].lfo_speed, 1);
  else if (Sender == MainForm->ChorusDepth) VertBarChangeOffsetU8((TTrackBar *)Sender, MainForm->ChorusDepthVal, &QuadGT_Progs[prog].lfo_depth, 1);

  // Midi Modulation Parameters
  else if (Sender == MainForm->ModNumber) QuadGT_Redraw_Mod(prog);
  else if (Sender == MainForm->ModSource) QuadGT_Redraw_Mod(prog);
  else if (Sender == MainForm->ModAmp)
  {
    UInt8 mod = StrToInt(MainForm->ModNumber->Text)-1;
    HorizBarChangeS8((TTrackBar *)Sender, MainForm->ModAmpVal, &QuadGT_Progs[prog].mod_amp[mod]);
  }

  // Reverb parameters
  else if (Sender == MainForm->ReverbPreDelay)  VertBarChangeOffsetU8((TTrackBar *)Sender,  MainForm->ReverbPreDelayVal,       &QuadGT_Progs[prog].reverb_predelay, 1);
  else if (Sender == MainForm->ReverbDecay)     VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbDecayVal,          &QuadGT_Progs[prog].reverb_decay);
  else if (Sender == MainForm->ReverbLowDecay)  VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbLowDecayVal,       &QuadGT_Progs[prog].reverb_low_decay);
  else if (Sender == MainForm->ReverbHighDecay) VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbHighDecayVal,      &QuadGT_Progs[prog].reverb_high_decay);
  else if (Sender == MainForm->ReverbDensity)   VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbDensityVal,        &QuadGT_Progs[prog].reverb_density);
  else if (Sender == MainForm->ReverbDiffusion) VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbDiffusionVal,      &QuadGT_Progs[prog].reverb_diffusion);
  else if (Sender == MainForm->ReverbGateHold)  VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbGateHoldVal,       &QuadGT_Progs[prog].reverb_gate_hold);
  else if (Sender == MainForm->ReverbGateRelease)  VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbGateReleaseVal, &QuadGT_Progs[prog].reverb_gate_release);
  else if (Sender == MainForm->ReverbGateLevel)  VertBarChangeU8((TTrackBar *)Sender,  MainForm->ReverbGateLevelVal,     &QuadGT_Progs[prog].reverb_gated_level);
  else if (Sender == MainForm->ReverbInMix)     HorizBarChangeS8((TTrackBar *)Sender, MainForm->ReverbInMixVal,          &QuadGT_Progs[prog].reverb_input_mix);
  else if (Sender == MainForm->ReverbPreMix)    HorizBarChangeS8((TTrackBar *)Sender, MainForm->ReverbPreMixVal,         &QuadGT_Progs[prog].reverb_predelay_mix);

  // Delay Parameters
  else if (Sender == MainForm->TapDelay) VertBarChangeU8((TTrackBar *)Sender, MainForm->TapDelayVal, &QuadGT_Progs[prog].tap_delay[tap]);
  else if (Sender == MainForm->TapPan) VertBarChangeU8((TTrackBar *)Sender, MainForm->TapPanVal, &QuadGT_Progs[prog].tap_pan[tap]);
  else if (Sender == MainForm->TapVol) VertBarChangeU8((TTrackBar *)Sender, MainForm->TapVolVal, &QuadGT_Progs[prog].tap_volume[tap]);
  else if (Sender == MainForm->TapFeedback) VertBarChangeU8((TTrackBar *)Sender, MainForm->TapFeedbackVal, &QuadGT_Progs[prog].tap_feedback[tap]);

  else if (Sender == MainForm->DelayInMix)     VertBarChangeS8((TTrackBar *)Sender, MainForm->DelayInMixVal,     &QuadGT_Progs[prog].delay_input_mix);
  else if (Sender == MainForm->DelayLeft)      VertBarChangeU16((TTrackBar *)Sender, MainForm->DelayLeftVal,      &QuadGT_Progs[prog].delay_left);
  else if (Sender == MainForm->DelayLeftFBack) VertBarChangeU8((TTrackBar *)Sender, MainForm->DelayLeftFBackVal, &QuadGT_Progs[prog].delay_left_feedback);
  else if (Sender == MainForm->DelayRight)     VertBarChangeU16((TTrackBar *)Sender, MainForm->DelayRightVal,     &QuadGT_Progs[prog].delay_right);
  else if (Sender == MainForm->DelayRighFBack) VertBarChangeU8((TTrackBar *)Sender, MainForm->DelayRightFBackVal, &QuadGT_Progs[prog].delay_right_feedback);
  else if (Sender == MainForm->DelayInput)
  {
    QuadGT_Progs[prog].delay_input=MainForm->DelayInput->ItemIndex;
    QuadGT_Redraw_Delay(prog);
  }

  // Ring Modulation parameters
  else if (Sender == MainForm->RingModSS)     VertBarChangeU16((TTrackBar *)Sender, MainForm->RingModSSVal,    &QuadGT_Progs[prog].ring_mod_shift);
  else if (Sender == MainForm->RingModInMix)  VertBarChangeS8((TTrackBar *)Sender, MainForm->RingModInMixVal,  &QuadGT_Progs[prog].ring_mod_in_mix);
  else if (Sender == MainForm->RingModOutMix) VertBarChangeS8((TTrackBar *)Sender, MainForm->RingModOutMixVal, &QuadGT_Progs[prog].ring_mod_out_mix);
}

void __fastcall TMainForm::DelayModeClick(TObject *Sender)
{
  //TBD: Move this logic into QuadParamChange
  UInt8 prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  QuadGT_Progs[prog].delay_mode=MainForm->DelayMode->ItemIndex;
  QuadGT_Redraw_Delay(prog);
}
//---------------------------------------------------------------------------

void QuadGT_Redraw_Resonator(const UInt8 prog)
{
  if ((QuadGT_Progs[prog].config==CFG0_EQ_PITCH_DELAY_REVERB) && (QuadGT_Progs[prog].eq_mode == 1))
  {
    // 2 Resonators
    MainForm->QuadResonator->Visible=TRUE;
  }
  else if ((QuadGT_Progs[prog].config == CFG3_5BANDEQ_PITCH_DELAY) && (QuadGT_Progs[prog].eq_mode == 1))
  {
    // 5 Resonators
    MainForm->QuadResonator->Visible=TRUE;
  }
  else if (QuadGT_Progs[prog].config==CFG6_RESONATOR_DELAY_REVERB)
  {
    // 5 Resonators
    MainForm->QuadResonator->Visible=TRUE;
  }
  else
  {
    MainForm->QuadResonator->Visible=FALSE;
  }
}

//---------------------------------------------------------------------------
// Name     : QuadGT_Convert_Data_From_Internal
// Description : Convert internal data structure to raw Quadraverb GT data.
//---------------------------------------------------------------------------
UInt32 QuadGT_Convert_Data_From_Internal(UInt8 prog, UInt8* data)
{
  if (prog >= QUAD_NUM_PATCH) return 1;

  // Resonator parameters, alternative one
  if ((QuadGT_Progs[prog].config == CFG0_EQ_PITCH_DELAY_REVERB) &&
      (QuadGT_Progs[prog].eq_mode == EQMODE1_EQ_PLUS_RESONATOR))
  {
    data[RES1_TUNE_IDX]  = QuadGT_Progs[prog].res_tune[0];
    data[RES1_DECAY_IDX] = QuadGT_Progs[prog].res_decay[0];
    data[RES1_AMP_IDX]   = QuadGT_Progs[prog].res_amp[0];

    data[RES2_TUNE_IDX]  = QuadGT_Progs[prog].res_tune[1];
    data[RES2_DECAY_IDX] = QuadGT_Progs[prog].res_decay[1];
    data[RES2_AMP_IDX]   = QuadGT_Progs[prog].res_amp[1];
  }
  // Graphic Eq
  else if (QuadGT_Progs[prog].config == CFG2_GEQ_DELAY)
  {
    data[GEQ_16HZ_IDX] = QuadGT_Progs[prog].geq_16hz + 14;
    data[GEQ_32HZ_IDX] = QuadGT_Progs[prog].geq_32hz + 14;
    data[GEQ_62HZ_IDX] = QuadGT_Progs[prog].geq_62hz + 14;
                         
    data[GEQ_4KHZ_IDX] = QuadGT_Progs[prog].geq_4khz + 14;
    data[GEQ_8KHZ_IDX] = QuadGT_Progs[prog].geq_8khz + 14;
  }
  // 3 and 5 Band Eq
  else
  {
    QuadGT_Encode_16Bit(QuadGT_Progs[prog].low_eq_freq, &data[LOW_EQ_FREQ_IDX]);
    QuadGT_Encode_16Bit(QuadGT_Progs[prog].low_eq_amp, &data[LOW_EQ_AMP_IDX]);

    QuadGT_Encode_16Bit(QuadGT_Progs[prog].high_eq_freq, &data[HIGH_EQ_FREQ_IDX]);
  }

  if (QuadGT_Progs[prog].config==CFG2_GEQ_DELAY)
  {
    data[GEQ_126HZ_IDX] = QuadGT_Progs[prog].geq_126hz + 14;
    data[GEQ_250HZ_IDX] = QuadGT_Progs[prog].geq_250hz + 14;
    data[GEQ_500HZ_IDX] = QuadGT_Progs[prog].geq_500hz + 14;
    data[GEQ_1KHZ_IDX]  = QuadGT_Progs[prog].geq_1khz  + 14;
    data[GEQ_2KHZ_IDX]  = QuadGT_Progs[prog].geq_2khz  + 14;
                        
    data[GEQ_16KHZ_IDX] = QuadGT_Progs[prog].geq_16khz + 14;
  }
  else
  {
    QuadGT_Encode_16Bit(QuadGT_Progs[prog].mid_eq_freq, &data[MID_EQ_FREQ_IDX]);
    data[MID_EQ_BW_IDX] = QuadGT_Progs[prog].mid_eq_q;
    QuadGT_Encode_16Bit(QuadGT_Progs[prog].mid_eq_amp, &data[MID_EQ_AMP_IDX]);

    QuadGT_Encode_16Bit(QuadGT_Progs[prog].high_eq_amp, &data[HIGH_EQ_AMP_IDX]);
  }


  // Type = 3 Eq and 5 Resonators
  if ((QuadGT_Progs[prog].config == CFG3_5BANDEQ_PITCH_DELAY) && (QuadGT_Progs[prog].eq_mode == 1))
  {
    data[RES1_TUNE_IDX_B]  = QuadGT_Progs[prog].res_tune[0];
    data[RES1_DECAY_IDX_B] = QuadGT_Progs[prog].res_decay[0];

    data[RES2_TUNE_IDX_B]  = QuadGT_Progs[prog].res_tune[1];
    data[RES2_DECAY_IDX_B] = QuadGT_Progs[prog].res_decay[1];

    data[RES3_TUNE_IDX]  = QuadGT_Progs[prog].res_tune[2];
    data[RES3_DECAY_IDX] = QuadGT_Progs[prog].res_decay[2];

    data[RES4_TUNE_IDX]  = QuadGT_Progs[prog].res_tune[3];
    data[RES4_DECAY_IDX] = QuadGT_Progs[prog].res_decay[3];

    data[RES5_TUNE_IDX]  = QuadGT_Progs[prog].res_tune[4];
    data[RES5_DECAY_IDX] = QuadGT_Progs[prog].res_decay[4];
  }
  // Otherwise, assume 5 band Eq
  else
  {
    QuadGT_Encode_16Bit(QuadGT_Progs[prog].low_mid_eq_freq, &data[LOW_MID_EQ_FREQ_IDX]);
    data[LOW_MID_EQ_BW_IDX] = QuadGT_Progs[prog].low_mid_eq_q;
    QuadGT_Encode_16Bit(QuadGT_Progs[prog].low_mid_eq_amp, &data[LOW_MID_EQ_AMP_IDX]);
    QuadGT_Encode_16Bit(QuadGT_Progs[prog].high_mid_eq_freq, &data[HIGH_MID_EQ_FREQ_IDX]);
    data[HIGH_MID_EQ_BW_IDX] = QuadGT_Progs[prog].high_mid_eq_q;
    QuadGT_Encode_16Bit(QuadGT_Progs[prog].high_mid_eq_amp, &data[HIGH_MID_EQ_AMP_IDX]);
  }

  // Select Sampling or Tap 1
  if (QuadGT_Progs[prog].config == CFG7_SAMPLING) 
  {
    data[SAMPLE_START_IDX]           = QuadGT_Progs[prog].sample_start;
    data[SAMPLE_LENGTH_IDX]          = QuadGT_Progs[prog].sample_length;
    data[SAMPLE_PLAYBACK_MODE_IDX]   = QuadGT_Progs[prog].sample_playback_mode;
    data[SAMPLE_PITCH_IDX]           = QuadGT_Progs[prog].sample_pitch;
    data[SAMPLE_REC_AUDIO_TRIG_IDX]  = QuadGT_Progs[prog].sample_rec_audio_trigger;
    data[SAMPLE_MIDI_TRIG_IDX]       = QuadGT_Progs[prog].sample_midi_trigger;
    data[SAMPLE_MIDI_BASE_NOTE_IDX]  = QuadGT_Progs[prog].sample_midi_base_note;
    data[SAMPLE_LOW_MIDI_NOTE_IDX]   = QuadGT_Progs[prog].sample_low_midi_note;
    data[SAMPLE_HIGH_MIDI_NOTE_IDX]  = QuadGT_Progs[prog].sample_high_midi_note;
  }
  else 
  {
    if (QuadGT_Progs[prog].config == CFG1_LESLIE_DELAY_REVERB)
    {
      data[LESLIE_HIGH_ROTOR_LEVEL_IDX] = QuadGT_Progs[prog].leslie_high_rotor_level;
      data[LESLIE_MOTOR_IDX]            = QuadGT_Progs[prog].leslie_motor;
      data[LESLIE_SPEED_IDX]            = QuadGT_Progs[prog].leslie_speed;
    }
    else
    {
      QuadGT_Encode_16Bit_Split(QuadGT_Progs[prog].tap_delay[0], &data[TAP1_DELAY_MSB_IDX],
                                                               &data[TAP1_DELAY_LSB_IDX]);
      QuadGT_Encode_16Bit(QuadGT_Progs[prog].tap_delay[1], &data[TAP2_DELAY_IDX]);
    }

    data[TAP1_VOLUME_IDX]   = QuadGT_Progs[prog].tap_volume[0];
    data[TAP1_PAN_IDX]      = QuadGT_Progs[prog].tap_pan[0];

    data[DETUNE_AMOUNT_IDX] = QuadGT_Progs[prog].detune_amount;

    data[TAP2_VOLUME_IDX]   = QuadGT_Progs[prog].tap_volume[1];
    data[TAP3_FEEDBACK_IDX] = QuadGT_Progs[prog].tap_feedback[2];

    if (QuadGT_Progs[prog].config == CFG3_5BANDEQ_PITCH_DELAY)
    {
      QuadGT_Encode_16Bit(QuadGT_Progs[prog].tap_delay[3], &data[TAP4_DELAY_IDX]);
    }
    else
    {
      data[REVERB_MODE_IDX]   = QuadGT_Progs[prog].reverb_mode;
    }
  }

  if (QuadGT_Progs[prog].config == CFG1_LESLIE_DELAY_REVERB)
  {
    data[LESLIE_SEPARATION_IDX]       = QuadGT_Progs[prog].leslie_seperation;
  }
  else
  {
    data[TAP1_FEEDBACK_IDX] = QuadGT_Progs[prog].tap_feedback[0];
  }

  data[TAP4_VOLUME_IDX]   = QuadGT_Progs[prog].tap_volume[3];

  if (QuadGT_Progs[prog].config == CFG3_5BANDEQ_PITCH_DELAY)
  {
    data[TAP4_PAN_IDX]      = QuadGT_Progs[prog].tap_pan[3];
    data[TAP4_FEEDBACK_IDX] = QuadGT_Progs[prog].tap_feedback[3];

    QuadGT_Encode_16Bit(QuadGT_Progs[prog].tap_delay[4], &data[TAP5_DELAY_IDX]);
    data[TAP5_VOLUME_IDX]   = QuadGT_Progs[prog].tap_volume[4];
    data[TAP5_PAN_IDX]      = QuadGT_Progs[prog].tap_pan[4];
    data[TAP5_FEEDBACK_IDX] = QuadGT_Progs[prog].tap_feedback[4];

    QuadGT_Encode_16Bit(QuadGT_Progs[prog].tap_delay[5], &data[TAP6_DELAY_IDX]);
    data[TAP6_VOLUME_IDX]   = QuadGT_Progs[prog].tap_volume[5];
    data[TAP6_PAN_IDX]      = QuadGT_Progs[prog].tap_pan[5];
    data[TAP6_FEEDBACK_IDX] = QuadGT_Progs[prog].tap_feedback[5];

    QuadGT_Encode_16Bit(QuadGT_Progs[prog].tap_delay[6], &data[TAP7_DELAY_IDX]);
    data[TAP7_VOLUME_IDX]   = QuadGT_Progs[prog].tap_volume[6];
    data[TAP7_PAN_IDX]      = QuadGT_Progs[prog].tap_pan[6];
  }
  else
  {
    data[REVERB_INPUT_1_IDX]=     QuadGT_Progs[prog].reverb_input_1	;
    data[REVERB_INPUT_2_IDX]=     QuadGT_Progs[prog].reverb_input_2	;
    data[REVERB_INPUT_MIX_IDX]=   QuadGT_Progs[prog].reverb_input_mix+99;
    data[REVERB_PREDELAY_IDX]=    QuadGT_Progs[prog].reverb_predelay	;
    data[REVERB_PREDELAY_MIX_IDX]=QuadGT_Progs[prog].reverb_predelay_mix+99;
    data[REVERB_DECAY_IDX]=       QuadGT_Progs[prog].reverb_decay	;
    data[REVERB_DIFFUSION_IDX]=   QuadGT_Progs[prog].reverb_diffusion	;
    data[REVERB_LOW_DECAY_IDX]=   QuadGT_Progs[prog].reverb_low_decay	;
    data[REVERB_HIGH_DECAY_IDX]=  QuadGT_Progs[prog].reverb_high_decay	;
    data[REVERB_DENSITY_IDX]=     QuadGT_Progs[prog].reverb_density	;
    data[REVERB_GATE_IDX]=        QuadGT_Progs[prog].reverb_gate	;
    data[REVERB_GATE_HOLD_IDX]=   QuadGT_Progs[prog].reverb_gate_hold	;
    data[REVERB_GATE_RELEASE_IDX]=QuadGT_Progs[prog].reverb_gate_release;
    data[REVERB_GATED_LEVEL_IDX]= QuadGT_Progs[prog].reverb_gated_level	;

    QuadGT_Encode_16Bit(QuadGT_Progs[prog].ring_mod_shift, &data[RING_MOD_SHIFT_IDX]);
  }

   data[MULTITAP_MASTER_FEEDBACK_IDX] = QuadGT_Progs[prog].multitap_master_feedback;
   data[MULTITAP_NUMBER_IDX]          = QuadGT_Progs[prog].multitap_number;

  //-------------------------------------------------------------------------
  // Pitch Parameters (0x1A - 
  //-------------------------------------------------------------------------
  data[PITCH_MODE_IDX]   = QuadGT_Progs[prog].pitch_mode;
  data[PITCH_FEEDBACK_IDX] =  QuadGT_Progs[prog].pitch_feedback;

  data[PITCH_INPUT_IDX]  = QuadGT_Progs[prog].pitch_input & BIT0;
  data[RES4_AMP_IDX]    |= (QuadGT_Progs[prog].res_amp[3] << 1) & BITS1to7;

  data[LFO_WAVEFORM_IDX] = QuadGT_Progs[prog].lfo_waveform & BIT0;
  data[EQ_PRESET_IDX]   |= (QuadGT_Progs[prog].eq_preset << 1) & BITS1to7;

  data[LFO_SPEED_IDX] = QuadGT_Progs[prog].lfo_speed;

  if (QuadGT_Progs[prog].config == CFG6_RESONATOR_DELAY_REVERB) 
  {
    data[RES_DECAY_IDX]     = QuadGT_Progs[prog].res_decay_all;
    data[RES_MIDI_GATE_IDX] = QuadGT_Progs[prog].res_midi_gate;

    data[RES1_PITCH_IDX] = QuadGT_Progs[prog].res_pitch[0];
    data[RES2_PITCH_IDX] = QuadGT_Progs[prog].res_pitch[1];
    data[RES3_PITCH_IDX] = QuadGT_Progs[prog].res_pitch[2];
    data[RES4_PITCH_IDX] = QuadGT_Progs[prog].res_pitch[3];
    data[RES5_PITCH_IDX] = QuadGT_Progs[prog].res_pitch[4];
  }
  else
  {
    data[LFO_DEPTH_IDX] = QuadGT_Progs[prog].lfo_depth;
    data[TRIGGER_FLANGE_IDX] = QuadGT_Progs[prog].trigger_flange;

    QuadGT_Encode_16Bit(QuadGT_Progs[prog].tap_delay[7], &data[TAP8_DELAY_IDX]);
    data[TAP8_VOLUME_IDX]  = QuadGT_Progs[prog].tap_volume[7];
    data[TAP8_PAN_IDX]     = QuadGT_Progs[prog].tap_pan[7];
    data[TAP8_FEEDBACK_IDX]= QuadGT_Progs[prog].tap_feedback[7];
  }

  //-------------------------------------------------------------------------
  // Delay Parameters (0x27 - 0x31)
  //-------------------------------------------------------------------------
  data[DELAY_MODE_IDX]   = QuadGT_Progs[prog].delay_mode;

  data[DELAY_INPUT_IDX]  = QuadGT_Progs[prog].delay_input & BIT0;
  data[RES5_AMP_IDX]    |= (QuadGT_Progs[prog].res_amp[4] << 1) & BITS1to7;

  data[DELAY_INPUT_MIX_IDX] = QuadGT_Progs[prog].delay_input_mix;

  if ((QuadGT_Progs[prog].delay_mode == 0) ||
      (QuadGT_Progs[prog].delay_mode == 1) ||
      (QuadGT_Progs[prog].delay_mode == 2))
  {
    QuadGT_Encode_16Bit(QuadGT_Progs[prog].delay_left, &data[DELAY_LEFT_IDX]);
    data[DELAY_LEFT_FEEDBACK_IDX] = QuadGT_Progs[prog].delay_left_feedback;

    QuadGT_Encode_16Bit(QuadGT_Progs[prog].delay_right, &data[DELAY_RIGHT_IDX]);
    data[DELAY_RIGHT_FEEDBACK_IDX] = QuadGT_Progs[prog].delay_right_feedback;
  }
  else if (QuadGT_Progs[prog].delay_mode == 3)
  {
    data[TAP2_PAN_IDX]     = QuadGT_Progs[prog].tap_pan[1];
    data[TAP2_FEEDBACK_IDX]= QuadGT_Progs[prog].tap_feedback[1];
    QuadGT_Encode_16Bit(QuadGT_Progs[prog].tap_delay[2], &data[TAP3_DELAY_IDX]);
    data[TAP3_VOLUME_IDX]  = QuadGT_Progs[prog].tap_volume[2];
    data[TAP3_PAN_IDX]     = QuadGT_Progs[prog].tap_pan[2];
  }

  //-------------------------------------------------------------------------
  // Reverb Parameters (0x32 - 0x43)
  //-------------------------------------------------------------------------

  //-------------------------------------------------------------------------
  // Config (0x44)
  //-------------------------------------------------------------------------
  data[CONFIG_IDX]    = QuadGT_Progs[prog].config;

  //-------------------------------------------------------------------------
  // Mix Parameters (0x45 - 
  //-------------------------------------------------------------------------
  data[PREPOST_EQ_IDX]    = QuadGT_Progs[prog].prepost_eq & BIT0;
  data[DIRECT_LEVEL_IDX] |= (QuadGT_Progs[prog].direct_level << 1) & BITS1to7;
  data[MASTER_EFFECTS_LEVEL_IDX] = QuadGT_Progs[prog].master_effects_level;

  if (QuadGT_Progs[prog].prepost_eq == 0) data[PREAMP_LEVEL_IDX]=QuadGT_Progs[prog].preamp_level;
  else                                    data[EQ_LEVEL_IDX]=QuadGT_Progs[prog].eq_level;

  if (QuadGT_Progs[prog].config==CFG1_LESLIE_DELAY_REVERB)
  {
    data[LESLIE_LEVEL_IDX] = QuadGT_Progs[prog].leslie_level;
  }
  else if (QuadGT_Progs[prog].config==CFG5_RINGMOD_DELAY_REVERB)
  {
    data[RING_MOD_LEVEL_IDX] = QuadGT_Progs[prog].ring_mod_level;
  }
  else
  {
    data[PITCH_LEVEL_IDX] = QuadGT_Progs[prog].pitch_level;
  }

  data[DELAY_LEVEL_IDX ] = QuadGT_Progs[prog].delay_level;
  if (QuadGT_Progs[prog].config!=CFG5_RINGMOD_DELAY_REVERB)
  {
    data[REVERB_LEVEL_IDX ] = QuadGT_Progs[prog].reverb_level;
  }
  else
  {
    data[RES1_AMP_IDX_B] = QuadGT_Progs[prog].res_amp[0]; 
  }

  //-------------------------------------------------------------------------
  // Modulation Parameters (0x50 - 0x67)
  //-------------------------------------------------------------------------
  data[MOD1_SOURCE_IDX]=QuadGT_Progs[prog].mod_source[0];
  data[MOD1_TARGET_IDX]=QuadGT_Progs[prog].mod_target[0];
  data[MOD1_AMP_IDX]=   QuadGT_Progs[prog].mod_amp[0]+99;
                         
  data[MOD2_SOURCE_IDX]=QuadGT_Progs[prog].mod_source[1];
  data[MOD2_TARGET_IDX]=QuadGT_Progs[prog].mod_target[1];
  data[MOD2_AMP_IDX]=   QuadGT_Progs[prog].mod_amp[1]+99;
                         
  data[MOD3_SOURCE_IDX]=QuadGT_Progs[prog].mod_source[2];
  data[MOD3_TARGET_IDX]=QuadGT_Progs[prog].mod_target[2];
  data[MOD3_AMP_IDX]=   QuadGT_Progs[prog].mod_amp[2]+99;
                         
  data[MOD4_SOURCE_IDX]=QuadGT_Progs[prog].mod_source[3];
  data[MOD4_TARGET_IDX]=QuadGT_Progs[prog].mod_target[3];
  data[MOD4_AMP_IDX]=   QuadGT_Progs[prog].mod_amp[3]+99;
                         
  data[MOD5_SOURCE_IDX]=QuadGT_Progs[prog].mod_source[4];
  data[MOD5_TARGET_IDX]=QuadGT_Progs[prog].mod_target[4];
  data[MOD4_AMP_IDX]=   QuadGT_Progs[prog].mod_amp[4]+99;
                         
  data[MOD6_SOURCE_IDX]=QuadGT_Progs[prog].mod_source[5];
  data[MOD6_TARGET_IDX]=QuadGT_Progs[prog].mod_target[5];
  data[MOD5_AMP_IDX]=   QuadGT_Progs[prog].mod_amp[5]+99;
                         
  data[MOD7_SOURCE_IDX]=QuadGT_Progs[prog].mod_source[6];
  data[MOD7_TARGET_IDX]=QuadGT_Progs[prog].mod_target[6];
  data[MOD7_AMP_IDX]=   QuadGT_Progs[prog].mod_amp[6]+99;
                         
  data[MOD8_SOURCE_IDX]=QuadGT_Progs[prog].mod_source[7];
  data[MOD8_TARGET_IDX]=QuadGT_Progs[prog].mod_target[7];
  data[MOD8_AMP_IDX]=   QuadGT_Progs[prog].mod_amp[7]+99;
  
  //-------------------------------------------------------------------------
  // Multi-tap Parameters (0x68 - 0x69)
  //-------------------------------------------------------------------------
  
  //-------------------------------------------------------------------------
  // Name (0x6A)
  //-------------------------------------------------------------------------
  memcpy(&data[NAME_IDX], QuadGT_Progs[prog].name, NAME_LENGTH);

  //-------------------------------------------------------------------------
  // More mix/ring mod/res/pan Parameters (0x78 - 0x7B)
  //-------------------------------------------------------------------------
  if (QuadGT_Progs[prog].config==CFG5_RINGMOD_DELAY_REVERB)
  {
    data[RING_MOD_OUTPUT_MIX_IDX]  = QuadGT_Progs[prog].ring_mod_out_mix+99;
    data[RING_MOD_DEL_REV_MIX_IDX] = QuadGT_Progs[prog].ring_mod_in_mix+99;
  }
  else
  {
    data[RES2_AMP_IDX_B] = QuadGT_Progs[prog].res_amp[1];
    data[RES3_AMP_IDX]   = QuadGT_Progs[prog].res_amp[2];
  }

  data[PAN_SPEED_IDX] = QuadGT_Progs[prog].pan_speed;
  data[PAN_DEPTH_IDX] = QuadGT_Progs[prog].pan_depth;
  
  //-------------------------------------------------------------------------
  // Pre-amp Parameters (0x7C - 0x7F)
  //-------------------------------------------------------------------------
  data[PREAMP_DIST_IDX]  = QuadGT_Progs[prog].dist & BITS0to3;
  data[PREAMP_COMP_IDX] |= (QuadGT_Progs[prog].comp << 4) & BITS4to6;
  data[EQ_MODE_IDX]     |= (QuadGT_Progs[prog].eq_mode<<7) & BIT7;

  data[MIX_MOD_IDX]      = (QuadGT_Progs[prog].mix_mod      << 6) & BITS6to7;
  data[EFFECT_LOOP_IDX] |= (QuadGT_Progs[prog].effect_loop  << 5) & BIT5;
  data[BASS_BOOST_IDX]  |= (QuadGT_Progs[prog].bass_boost   << 4) & BIT4;
  data[PREAMP_TONE_IDX] |= (QuadGT_Progs[prog].preamp_tone  << 2) & BITS2to3;
  data[CAB_SIM_IDX]     |= QuadGT_Progs[prog].cab_sim  & BITS0to1;

  data[PREAMP_OD_IDX]         = (QuadGT_Progs[prog].od << 5) & BITS5to7;
  data[PREAMP_GATE_IDX]      |= (QuadGT_Progs[prog].preamp_gate & BITS0to4);

  data[PREAMP_OUT_LEVEL_IDX] = QuadGT_Progs[prog].preamp_out_level;
  
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

  memcpy(QuadGT_Progs[prog].name, &data[NAME_IDX], NAME_LENGTH);
  QuadGT_Progs[prog].name[NAME_LENGTH]=0;

  //-------------------------------------------------------------------------
  // Decode modes and configurations first so we can use them later for decision making
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].config        = data[CONFIG_IDX];
  QuadGT_Progs[prog].eq_mode       = ((data[EQ_MODE_IDX]&BIT7)>>7);   // Eq Mode: Eq or Eq/Resonator
  QuadGT_Progs[prog].pitch_mode    = data[PITCH_MODE_IDX];
  QuadGT_Progs[prog].delay_mode    = data[DELAY_MODE_IDX];
  if (QuadGT_Progs[prog].config != CFG3_5BANDEQ_PITCH_DELAY)
  {
    QuadGT_Progs[prog].reverb_mode   = data[REVERB_MODE_IDX];
  }

  //-------------------------------------------------------------------------
  // Preamp Parameters (0x7C - 0x7F)
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].comp             = (data[PREAMP_COMP_IDX] & BITS4to6) >> 4;
  QuadGT_Progs[prog].od               = (data[PREAMP_OD_IDX]   & BITS5to7) >> 5;
  QuadGT_Progs[prog].dist             = (data[PREAMP_DIST_IDX] & BITS0to3);
  QuadGT_Progs[prog].preamp_tone      = (data[PREAMP_TONE_IDX] & BITS2to3) >> 2;
  QuadGT_Progs[prog].bass_boost       = (data[BASS_BOOST_IDX]  & BIT4) >> 4;
  QuadGT_Progs[prog].cab_sim          = (data[CAB_SIM_IDX]     & BITS0to1);
  QuadGT_Progs[prog].effect_loop      = (data[EFFECT_LOOP_IDX] & BIT5) >> 5;
  QuadGT_Progs[prog].preamp_gate      = (data[PREAMP_GATE_IDX] & BITS0to4);
  QuadGT_Progs[prog].preamp_out_level = data[PREAMP_OUT_LEVEL_IDX];

  //-------------------------------------------------------------------------
  // Eq Parameters
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].eq_preset = (data[EQ_PRESET_IDX]&BITS1to7)>>1;       // User or Preset 1-6

  // Resonator parameters, alternative one
  if ((QuadGT_Progs[prog].config == CFG0_EQ_PITCH_DELAY_REVERB) &&
      (QuadGT_Progs[prog].eq_mode == EQMODE1_EQ_PLUS_RESONATOR))
  {
    QuadGT_Progs[prog].res_tune[0] = data[RES1_TUNE_IDX]  ; 
    QuadGT_Progs[prog].res_decay[0] =data[RES1_DECAY_IDX] ; 
    QuadGT_Progs[prog].res_amp[0] =  data[RES1_AMP_IDX]   ; 
                                     
    QuadGT_Progs[prog].res_tune[1] = data[RES2_TUNE_IDX]  ; 
    QuadGT_Progs[prog].res_decay[1] =data[RES2_DECAY_IDX] ; 
    QuadGT_Progs[prog].res_amp[1] =  data[RES2_AMP_IDX]   ; 
  }
  // Graphic Eq
  else if (QuadGT_Progs[prog].config == CFG2_GEQ_DELAY)
  {
    QuadGT_Progs[prog].geq_16hz   = data[GEQ_16HZ_IDX]-14;
    QuadGT_Progs[prog].geq_32hz   = data[GEQ_32HZ_IDX]-14;
    QuadGT_Progs[prog].geq_62hz   = data[GEQ_62HZ_IDX]-14;

    QuadGT_Progs[prog].geq_4khz   = data[GEQ_4KHZ_IDX]-14;
    QuadGT_Progs[prog].geq_8khz   = data[GEQ_8KHZ_IDX]-14;
  }
  // 3 and 5 Band Eq
  else
  {
    QuadGT_Progs[prog].low_eq_freq   = QuadGT_Decode_16Bit(&data[LOW_EQ_FREQ_IDX]);
    QuadGT_Progs[prog].low_eq_amp    = QuadGT_Decode_16Bit(&data[LOW_EQ_AMP_IDX]);
    QuadGT_Progs[prog].high_eq_freq  = QuadGT_Decode_16Bit(&data[HIGH_EQ_FREQ_IDX]);
  }

  if (QuadGT_Progs[prog].config==CFG2_GEQ_DELAY)
  {
    QuadGT_Progs[prog].geq_126hz  = data[GEQ_126HZ_IDX]-14;
    QuadGT_Progs[prog].geq_250hz  = data[GEQ_250HZ_IDX]-14;
    QuadGT_Progs[prog].geq_500hz  = data[GEQ_500HZ_IDX]-14;
    QuadGT_Progs[prog].geq_1khz   = data[GEQ_1KHZ_IDX]-14;
    QuadGT_Progs[prog].geq_2khz   = data[GEQ_2KHZ_IDX]-14;

    QuadGT_Progs[prog].geq_16khz  = data[GEQ_16KHZ_IDX]-14;
  }
  else
  {
    QuadGT_Progs[prog].mid_eq_freq   = QuadGT_Decode_16Bit(&data[MID_EQ_FREQ_IDX]);
    QuadGT_Progs[prog].mid_eq_amp    = QuadGT_Decode_16Bit(&data[MID_EQ_AMP_IDX]);
    QuadGT_Progs[prog].mid_eq_q      = data[MID_EQ_BW_IDX];
    QuadGT_Progs[prog].high_eq_amp   = QuadGT_Decode_16Bit(&data[HIGH_EQ_AMP_IDX]);
  }

  // Type = 3 Eq and 5 Resonators
  if ((QuadGT_Progs[prog].config == CFG3_5BANDEQ_PITCH_DELAY) && 
      (QuadGT_Progs[prog].eq_mode == EQMODE1_EQ_PLUS_RESONATOR))
  {
    QuadGT_Progs[prog].res_tune[0]  = data[RES1_TUNE_IDX_B];
    QuadGT_Progs[prog].res_decay[0] =data[RES1_DECAY_IDX_B];
                                     
    QuadGT_Progs[prog].res_tune[1]  = data[RES2_TUNE_IDX_B];
    QuadGT_Progs[prog].res_decay[1] =data[RES2_DECAY_IDX_B];
                                     
    QuadGT_Progs[prog].res_tune[2]  = data[RES3_TUNE_IDX];
    QuadGT_Progs[prog].res_decay[2] =data[RES3_DECAY_IDX];
                                     
    QuadGT_Progs[prog].res_tune[3]  = data[RES4_TUNE_IDX];
    QuadGT_Progs[prog].res_decay[3] =data[RES4_DECAY_IDX];
                                     
    QuadGT_Progs[prog].res_tune[4]  = data[RES5_TUNE_IDX];
    QuadGT_Progs[prog].res_decay[4] =data[RES5_DECAY_IDX];
  }
  // Otherwise, assume 5 band Eq
  else
  {
    QuadGT_Progs[prog].low_mid_eq_freq   = QuadGT_Decode_16Bit(&data[LOW_MID_EQ_FREQ_IDX]);
    QuadGT_Progs[prog].low_mid_eq_amp    = QuadGT_Decode_16Bit(&data[LOW_MID_EQ_AMP_IDX]);
    QuadGT_Progs[prog].low_mid_eq_q      = data[LOW_MID_EQ_BW_IDX];

    QuadGT_Progs[prog].high_mid_eq_freq   = QuadGT_Decode_16Bit(&data[HIGH_MID_EQ_FREQ_IDX]);
    QuadGT_Progs[prog].high_mid_eq_amp    = QuadGT_Decode_16Bit(&data[HIGH_MID_EQ_AMP_IDX]);
    QuadGT_Progs[prog].high_mid_eq_q      = data[HIGH_MID_EQ_BW_IDX];
  }

  // Sampling or Tap/Leslie parameters
  if (QuadGT_Progs[prog].config == CFG7_SAMPLING) 
  {
    QuadGT_Progs[prog].sample_start            = data[SAMPLE_START_IDX]; 
    QuadGT_Progs[prog].sample_length           = data[SAMPLE_LENGTH_IDX]; 
    QuadGT_Progs[prog].sample_playback_mode    = data[SAMPLE_PLAYBACK_MODE_IDX]; 
    QuadGT_Progs[prog].sample_pitch            = data[SAMPLE_PITCH_IDX]; 
    QuadGT_Progs[prog].sample_rec_audio_trigger= data[SAMPLE_REC_AUDIO_TRIG_IDX]; 
    QuadGT_Progs[prog].sample_midi_trigger     = data[SAMPLE_MIDI_TRIG_IDX]; 
    QuadGT_Progs[prog].sample_midi_base_note   = data[SAMPLE_MIDI_BASE_NOTE_IDX]; 
    QuadGT_Progs[prog].sample_low_midi_note    = data[SAMPLE_LOW_MIDI_NOTE_IDX]; 
    QuadGT_Progs[prog].sample_high_midi_note   = data[SAMPLE_HIGH_MIDI_NOTE_IDX]; 
  }
  else
  {
    if (QuadGT_Progs[prog].config == CFG1_LESLIE_DELAY_REVERB)
    {
      QuadGT_Progs[prog].leslie_high_rotor_level = data[LESLIE_HIGH_ROTOR_LEVEL_IDX];
      QuadGT_Progs[prog].leslie_motor            = data[LESLIE_MOTOR_IDX];
      QuadGT_Progs[prog].leslie_speed            = data[LESLIE_SPEED_IDX];
    }
    else
    {
      QuadGT_Progs[prog].tap_delay[0] = QuadGT_Decode_16Bit_Split(&data[TAP1_DELAY_MSB_IDX],
                                                                &data[TAP1_DELAY_LSB_IDX]);
      QuadGT_Progs[prog].tap_delay[1] = QuadGT_Decode_16Bit(&data[TAP2_DELAY_IDX]);
    }

    QuadGT_Progs[prog].tap_volume[0] = data[TAP1_VOLUME_IDX];
    QuadGT_Progs[prog].tap_pan[0]    = data[TAP1_PAN_IDX];

    QuadGT_Progs[prog].detune_amount = data[DETUNE_AMOUNT_IDX];

    QuadGT_Progs[prog].tap_volume[1] = data[TAP2_VOLUME_IDX];
    QuadGT_Progs[prog].tap_feedback[2] = data[TAP3_FEEDBACK_IDX];

    if (QuadGT_Progs[prog].config == CFG3_5BANDEQ_PITCH_DELAY)
    {
      QuadGT_Progs[prog].tap_delay[3] = QuadGT_Decode_16Bit(&data[TAP4_DELAY_IDX]);
    }
  }

  if (QuadGT_Progs[prog].config == CFG1_LESLIE_DELAY_REVERB)
  {
    QuadGT_Progs[prog].leslie_seperation = data[LESLIE_SEPARATION_IDX];
  }
  else
  {
    QuadGT_Progs[prog].tap_feedback[0] = data[TAP1_FEEDBACK_IDX];
  }

  QuadGT_Progs[prog].tap_volume[3] = data[TAP4_VOLUME_IDX];

  if (QuadGT_Progs[prog].config == CFG3_5BANDEQ_PITCH_DELAY)
  {
    QuadGT_Progs[prog].tap_pan[3]      = data[TAP4_PAN_IDX];
    QuadGT_Progs[prog].tap_feedback[3] = data[TAP4_FEEDBACK_IDX];

    QuadGT_Progs[prog].tap_delay[4] = QuadGT_Decode_16Bit(&data[TAP5_DELAY_IDX]);
    QuadGT_Progs[prog].tap_volume[4] = data[TAP5_VOLUME_IDX];
    QuadGT_Progs[prog].tap_pan[4]      = data[TAP5_PAN_IDX];
    QuadGT_Progs[prog].tap_feedback[4] = data[TAP5_FEEDBACK_IDX];

    QuadGT_Progs[prog].tap_delay[5] = QuadGT_Decode_16Bit(&data[TAP6_DELAY_IDX]);
    QuadGT_Progs[prog].tap_volume[5] = data[TAP6_VOLUME_IDX];
    QuadGT_Progs[prog].tap_pan[5]      = data[TAP6_PAN_IDX];
    QuadGT_Progs[prog].tap_feedback[5] = data[TAP6_FEEDBACK_IDX];

    QuadGT_Progs[prog].tap_delay[6] = QuadGT_Decode_16Bit(&data[TAP7_DELAY_IDX]);
    QuadGT_Progs[prog].tap_volume[6] = data[TAP7_VOLUME_IDX];
    QuadGT_Progs[prog].tap_pan[6]      = data[TAP7_PAN_IDX];
  }
  else
  {
    //-------------------------------------------------------------------------
    // Reverb Parameters
    //-------------------------------------------------------------------------
    QuadGT_Progs[prog].reverb_input_1	        = data[REVERB_INPUT_1_IDX];
    QuadGT_Progs[prog].reverb_input_2	        = data[REVERB_INPUT_2_IDX];
    QuadGT_Progs[prog].reverb_input_mix	        = data[REVERB_INPUT_MIX_IDX]-99;
    QuadGT_Progs[prog].reverb_predelay	        = data[REVERB_PREDELAY_IDX];
    QuadGT_Progs[prog].reverb_predelay_mix	= data[REVERB_PREDELAY_MIX_IDX]-99;
    QuadGT_Progs[prog].reverb_decay	        = data[REVERB_DECAY_IDX];
    QuadGT_Progs[prog].reverb_diffusion	        = data[REVERB_DIFFUSION_IDX];
    QuadGT_Progs[prog].reverb_low_decay	        = data[REVERB_LOW_DECAY_IDX];
    QuadGT_Progs[prog].reverb_high_decay	= data[REVERB_HIGH_DECAY_IDX];
    QuadGT_Progs[prog].reverb_density	        = data[REVERB_DENSITY_IDX];
    QuadGT_Progs[prog].reverb_gate	        = data[REVERB_GATE_IDX];
    QuadGT_Progs[prog].reverb_gate_hold	        = data[REVERB_GATE_HOLD_IDX];
    QuadGT_Progs[prog].reverb_gate_release	= data[REVERB_GATE_RELEASE_IDX];
    QuadGT_Progs[prog].reverb_gated_level	= data[REVERB_GATED_LEVEL_IDX];

    QuadGT_Progs[prog].ring_mod_shift = QuadGT_Decode_16Bit(&data[RING_MOD_SHIFT_IDX]);
  }

  QuadGT_Progs[prog].multitap_master_feedback = data[MULTITAP_MASTER_FEEDBACK_IDX];
  QuadGT_Progs[prog].multitap_number          = data[MULTITAP_NUMBER_IDX];

  //-------------------------------------------------------------------------
  // Pitch Parameters
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].pitch_input   = data[PITCH_INPUT_IDX] & BIT0;
  QuadGT_Progs[prog].res_amp[3] = (data[RES4_AMP_IDX] && BITS1to7) >> 1;

  QuadGT_Progs[prog].lfo_waveform  = data[LFO_WAVEFORM_IDX] & BIT0;
  QuadGT_Progs[prog].lfo_speed     = data[LFO_SPEED_IDX];
  if (QuadGT_Progs[prog].config == CFG6_RESONATOR_DELAY_REVERB) 
  {
    QuadGT_Progs[prog].res_decay_all = data[RES_DECAY_IDX];
    QuadGT_Progs[prog].res_midi_gate = data[RES_MIDI_GATE_IDX];

    QuadGT_Progs[prog].res_pitch[0]  = data[RES1_PITCH_IDX];
    QuadGT_Progs[prog].res_pitch[1]  = data[RES2_PITCH_IDX];
    QuadGT_Progs[prog].res_pitch[2]  = data[RES3_PITCH_IDX];
    QuadGT_Progs[prog].res_pitch[3]  = data[RES4_PITCH_IDX];
    QuadGT_Progs[prog].res_pitch[4]  = data[RES5_PITCH_IDX];
  }
  else
  {
    QuadGT_Progs[prog].lfo_depth      = data[LFO_DEPTH_IDX];
    QuadGT_Progs[prog].trigger_flange = data[TRIGGER_FLANGE_IDX];

    QuadGT_Progs[prog].tap_delay[7] = QuadGT_Decode_16Bit(&data[TAP8_DELAY_IDX]);
    QuadGT_Progs[prog].tap_volume[7] = data[TAP8_VOLUME_IDX];
    QuadGT_Progs[prog].tap_pan[7]    = data[TAP8_PAN_IDX];
    QuadGT_Progs[prog].tap_feedback[7]= data[TAP8_FEEDBACK_IDX];
  }
  QuadGT_Progs[prog].pitch_feedback= data[PITCH_FEEDBACK_IDX];

  //-------------------------------------------------------------------------
  // Delay Parameters
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].delay_input=data[DELAY_INPUT_IDX]&BIT0;
  QuadGT_Progs[prog].delay_input_mix = data[DELAY_INPUT_MIX_IDX];

  if ((QuadGT_Progs[prog].delay_mode == 0) ||
      (QuadGT_Progs[prog].delay_mode == 1) ||
      (QuadGT_Progs[prog].delay_mode == 2))
  {
    QuadGT_Progs[prog].delay_left          = QuadGT_Decode_16Bit(&data[DELAY_LEFT_IDX]);
    QuadGT_Progs[prog].delay_left_feedback =data[DELAY_LEFT_FEEDBACK_IDX];
    QuadGT_Progs[prog].delay_right         = QuadGT_Decode_16Bit(&data[DELAY_RIGHT_IDX]);
    QuadGT_Progs[prog].delay_right_feedback=data[DELAY_RIGHT_FEEDBACK_IDX];
  }
  else if (QuadGT_Progs[prog].delay_mode == 3)
  {
    QuadGT_Progs[prog].tap_pan[1]      = data[TAP2_PAN_IDX];
    QuadGT_Progs[prog].tap_feedback[1] = data[TAP2_FEEDBACK_IDX];
    QuadGT_Progs[prog].tap_delay[2]    = QuadGT_Decode_16Bit(&data[TAP3_DELAY_IDX]);
    QuadGT_Progs[prog].tap_volume[2]   = data[TAP3_VOLUME_IDX];
    QuadGT_Progs[prog].tap_pan[2]      = data[TAP3_PAN_IDX];
  }

  //-------------------------------------------------------------------------
  // Mix Parameters
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].prepost_eq=data[PREPOST_EQ_IDX] & BIT0;
  QuadGT_Progs[prog].direct_level=(data[DIRECT_LEVEL_IDX ] & BITS1to7) >> 1;
  QuadGT_Progs[prog].master_effects_level=data[MASTER_EFFECTS_LEVEL_IDX ];

  if (QuadGT_Progs[prog].prepost_eq == 0) QuadGT_Progs[prog].preamp_level=data[PREAMP_LEVEL_IDX];
  else                                    QuadGT_Progs[prog].eq_level=data[EQ_LEVEL_IDX];

  if (QuadGT_Progs[prog].config==CFG1_LESLIE_DELAY_REVERB)
  {
    QuadGT_Progs[prog].leslie_level=data[LESLIE_LEVEL_IDX];
  }
  else if (QuadGT_Progs[prog].config==CFG5_RINGMOD_DELAY_REVERB)
  {
    QuadGT_Progs[prog].ring_mod_level=data[RING_MOD_LEVEL_IDX];
  }
  else
  {
    QuadGT_Progs[prog].pitch_level=data[PITCH_LEVEL_IDX];
  }

  QuadGT_Progs[prog].delay_level=data[DELAY_LEVEL_IDX ];
  if (QuadGT_Progs[prog].config!=CFG5_RINGMOD_DELAY_REVERB)
  {
    QuadGT_Progs[prog].reverb_level=data[REVERB_LEVEL_IDX ];
  }
  else
  {
    QuadGT_Progs[prog].res_amp[0] =  data[RES1_AMP_IDX_B]; 
  }

  QuadGT_Progs[prog].mix_mod=(data[MIX_MOD_IDX] & BITS6to7) >> 6;

  //-------------------------------------------------------------------------
  // Modulation Parameters
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].mod_source[0]=data[MOD1_SOURCE_IDX];
  QuadGT_Progs[prog].mod_target[0]=data[MOD1_TARGET_IDX];
  QuadGT_Progs[prog].mod_amp[0]   =data[MOD1_AMP_IDX]-99;

  QuadGT_Progs[prog].mod_source[1]=data[MOD2_SOURCE_IDX];
  QuadGT_Progs[prog].mod_target[1]=data[MOD2_TARGET_IDX];
  QuadGT_Progs[prog].mod_amp[1]   =data[MOD2_AMP_IDX]-99;

  QuadGT_Progs[prog].mod_source[2]=data[MOD3_SOURCE_IDX];
  QuadGT_Progs[prog].mod_target[2]=data[MOD3_TARGET_IDX];
  QuadGT_Progs[prog].mod_amp[2]   =data[MOD3_AMP_IDX]-99;

  QuadGT_Progs[prog].mod_source[3]=data[MOD4_SOURCE_IDX];
  QuadGT_Progs[prog].mod_target[3]=data[MOD4_TARGET_IDX];
  QuadGT_Progs[prog].mod_amp[3]   =data[MOD4_AMP_IDX]-99;

  QuadGT_Progs[prog].mod_source[4]=data[MOD5_SOURCE_IDX];
  QuadGT_Progs[prog].mod_target[4]=data[MOD5_TARGET_IDX];
  QuadGT_Progs[prog].mod_amp[4]   =data[MOD4_AMP_IDX]-99;

  QuadGT_Progs[prog].mod_source[5]=data[MOD6_SOURCE_IDX];
  QuadGT_Progs[prog].mod_target[5]=data[MOD6_TARGET_IDX];
  QuadGT_Progs[prog].mod_amp[5]   =data[MOD5_AMP_IDX]-99;

  QuadGT_Progs[prog].mod_source[6]=data[MOD7_SOURCE_IDX];
  QuadGT_Progs[prog].mod_target[6]=data[MOD7_TARGET_IDX];
  QuadGT_Progs[prog].mod_amp[6]   =data[MOD7_AMP_IDX]-99;

  QuadGT_Progs[prog].mod_source[7]=data[MOD8_SOURCE_IDX];
  QuadGT_Progs[prog].mod_target[7]=data[MOD8_TARGET_IDX];
  QuadGT_Progs[prog].mod_amp[7]   =data[MOD8_AMP_IDX]-99;

  if (QuadGT_Progs[prog].config==CFG5_RINGMOD_DELAY_REVERB)
  {
    QuadGT_Progs[prog].ring_mod_out_mix = data[RING_MOD_OUTPUT_MIX_IDX]-99;
    QuadGT_Progs[prog].ring_mod_in_mix= data[RING_MOD_DEL_REV_MIX_IDX]-99;
  }
  else
  {
    QuadGT_Progs[prog].res_amp[1] =  data[RES2_AMP_IDX_B]; 
    QuadGT_Progs[prog].res_amp[2] =  data[RES3_AMP_IDX]; 
  }
  QuadGT_Progs[prog].pan_speed = data[PAN_SPEED_IDX];
  QuadGT_Progs[prog].pan_depth = data[PAN_DEPTH_IDX];
  //-------------------------------------------------------------------------
  // Resonator Parameters
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].res_amp[4] = (data[RES5_AMP_IDX] & BITS1to7) >> 1;

  return 0;
}

//---------------------------------------------------------------------------
// Name        : QuadPatchWriteClick
// Description : Write the cyurrent patch to the QuadGT via MIDI
// Param 1     : Pointer object that generated the event
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
     FormError->ShowError(status, "sending SYSEX to Midi output device");
  }

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
    FormError->ShowError(ferror(bank_file),"opening bank file");
    return;
  }

  // TBD: Write a header

  // Write each patch
  for(prog=0; prog<QUAD_NUM_PATCH; prog++)
  {
    if (fwrite(&QuadGT_Progs[prog], sizeof(tQuadGT_Prog), 1, bank_file) != 1)
    {
      FormError->ShowError(ferror(bank_file),"writing bank file ");
      break;
    }
  }

  fclose(bank_file);

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
    FormError->ShowError(ferror(bank_file),"opening bank file");
    return;
  }

  // TBD: Read the header and check for validity
  
  for(prog=0; prog<QUAD_NUM_PATCH; prog++)
  {
    if (fread(&quadgt_patch[prog], sizeof(tQuadGT_Prog), 1, bank_file) != 1)
    {
      FormError->ShowError(ferror(bank_file),"reading bank file ");
      break;
    }
  }

  fclose(bank_file);
  if (prog >= QUAD_NUM_PATCH)
  {
    memcpy(QuadGT_Progs, quadgt_patch, sizeof(QuadGT_Progs));
  }

  prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  QuadGT_Redraw_Patch(prog);
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
    FormError->ShowError(ferror(sysex_file),"opening SysEx file");
    return;
  }

  // Determine size of the file
  fseek(sysex_file, 0, SEEK_END);
  size=ftell(sysex_file);
  rewind(sysex_file);

  data.buffer=(UInt8 *)malloc(sizeof(UInt8) * size);
  if (data.buffer == NULL)
  {
      FormError->ShowError(1,"allocating memory for SysEx file ");
  }
  else
  {
    if (fread(data.buffer, 1, size, sysex_file) != size)
    {
      FormError->ShowError(ferror(sysex_file),"reading SysEx file ");
    }
    else
    {
      data.length = size;
      FormDebug->Log(NULL, "Loaded SYSEX");
      QuadGT_Sysex_Process(data);
    }
  }

  fclose(sysex_file);
  free(data.buffer);

  prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  QuadGT_Redraw_Patch(prog);
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

void __fastcall TMainForm::VertBarCentChange(TObject *Sender)
{
  // TBD:???
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
     FormDebug->Log(NULL, "Sysex Process");
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

	   FormDebug->Log(NULL, "Sysex Code: "+AnsiString(code)+"   Prog: "+AnsiString(prog));

           if (code == *Sysex_Data_Dump)
           {
             // SysEx contains a single patch
             if (prog < QUAD_NUM_PATCH)
             {
	       FormDebug->Log(NULL, "RX: Sysex quad patch, bytes: "+AnsiString(sysex.length-offset));
               QuadGT_Decode_From_Sysex(sysex.buffer+offset,sysex.length-offset-1, quadgt, QUAD_PATCH_SIZE);
               QuadGT_Convert_QuadGT_To_Internal(prog, quadgt);
               QuadGT_Redraw_Patch(prog);
             }

             // Sysex contains a bank of patches
             else if (prog > QUAD_NUM_PATCH)
             {
	       FormDebug->Log(NULL, "RX: Sysex bank, bytes: "+AnsiString(sysex.length-offset));
               for (prog = 0; prog < QUAD_NUM_PATCH; prog++)
               {
                 QuadGT_Decode_From_Sysex(sysex.buffer+offset, SYSEX_PATCH_SIZE, quadgt, QUAD_PATCH_SIZE);
                 QuadGT_Convert_QuadGT_To_Internal(prog, quadgt);

                 offset+= SYSEX_PATCH_SIZE;
               }
               QuadGT_Redraw_Patch(0);
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
    FormError->ShowError(ferror(quadgt_file),"opening QuadGT file");
    return;
  }

  // Determine size of the file
  fseek(quadgt_file, 0, SEEK_END);
  size=ftell(quadgt_file);
  rewind(quadgt_file);

  data.buffer=(UInt8 *)malloc(sizeof(UInt8) * size);
  if (data.buffer == NULL)
  {
      FormError->ShowError(1,"allocating memory for QuadGT file ");
  }
  else
  {
    if (fread(data.buffer, 1, size, quadgt_file) != size)
    {
      FormError->ShowError(ferror(quadgt_file),"reading QuadGT file ");
    }
    else
    {
      data.length = size;
      FormDebug->Log(NULL, "Loaded QuadGT");

      for (prog = 0; prog < QUAD_NUM_PATCH; prog++)
      {
        QuadGT_Convert_QuadGT_To_Internal(prog, data.buffer+(prog*QUAD_PATCH_SIZE));
      }
    }
  }

  fclose(quadgt_file);
  free(data.buffer);

  QuadGT_Redraw_Patch(0);
}
//---------------------------------------------------------------------------
