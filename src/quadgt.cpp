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

void save_patch(UInt8 *data, UInt32 length, char *fname)
{
  FILE *handle;
  int i;
  handle= fopen(fname,"w");

  for (i=0; i<length; i++)
  {
    fprintf(handle,"%2.2X ",*(data+i));
  }
  fclose(handle);
}

// Decode Midi SysEx data in 7bits to Quadraverb data in 8bits
UInt32 QuadGT_Decode_From_Sysex(UInt8 *in, UInt32 length, UInt8* out, UInt32 out_len)
{
  UInt8 oc;
  UInt32 i,j;
  UInt8 shift;

  save_patch(in,length,"patch_in.syx");
  // TBD: Ensure output is correct length

  //FormDebug->Log(NULL,"----------------------------------");
  //FormDebug->Log(NULL,"Decode to Quad");
  //FormDebug->LogHex(NULL,AnsiString(length)+" SYSEX",in,length);
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
  //FormDebug->LogHex(NULL,AnsiString(j)+" QUAD",out,j);
  
  return(j);
}

// Encode 8bit quadraverb data into low 7 bits for SysEx
UInt32 QuadGT_Encode_To_Sysex(UInt8 *in, UInt32 length, UInt8 * out, UInt32 out_len)
{
  UInt8 lc,cc;
  UInt32 i,j;
  UInt8 shift;

  //FormDebug->Log(NULL,"----------------------------------");
  //FormDebug->Log(NULL,"Encode to SYSEX");
  //FormDebug->LogHex(NULL,AnsiString(length)+" QUAD",in,length);
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
    if (j >= out_len) return(0);
  }

  // TBD: Resolve issue with last byte being encoded wrongly
 
  //FormDebug->LogHex(NULL,AnsiString(j)+" SYSEX",out,j);
  return(j);
}


void QuadGT_Init(void)
{
  UInt8 patch;
  char patch_name[NAME_LENGTH+1];

  for (patch=0; patch<QUAD_NUM_PATCH; patch++)
  {
    // Set all parameters to 0
    memset(&QuadGT_Progs[patch], 0x00, sizeof(tQuadGT_Prog));

    // Create a program name
    sprintf(patch_name,"Patch-%2.2d",patch);
    strcpy(QuadGT_Progs[patch].name,patch_name);
  }

}

void QuadGT_Display_Update_Patch(UInt8 prog)
{
  UInt8 config;

  /* Program Number */
  MainForm->QuadPatchNum->Text=AnsiString(prog);

  /* Program name */
  MainForm->EditQuadPatchName->Text=AnsiString(QuadGT_Progs[prog].name);

  /* Configuration */
  MainForm->QuadConfig->ItemIndex=QuadGT_Progs[prog].config;

  QuadGT_Display_Update_Reverb(prog);
  QuadGT_Display_Update_Delay(prog);
  QuadGT_Display_Update_Pitch(prog);
  QuadGT_Display_Update_Eq(prog);
  QuadGT_Display_Update_Mix(prog);
  QuadGT_Display_Update_Mod(prog);
  QuadGT_Display_Update_Preamp(prog);
  QuadGT_Display_Update_Resonator(prog);
}

//---------------------------------------------------------------------------
// Name        : QuadGT_Display_Update_Reverb
// Description : Update the on screen display of the reverb parameters for
//               the given program based on the data in QuadGT_Progs
// Parameter 1 : Program number for which display is to be updated
// Returns     : NONE.
//---------------------------------------------------------------------------
void QuadGT_Display_Update_Reverb(const UInt8 prog)
{
  UInt8 val;

  if (QuadGT_Progs[prog].config==0)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=true;
    MainForm->ReverbInMix->Visible=true;
  }
  else if ((QuadGT_Progs[prog].config==1) || (QuadGT_Progs[prog].config==5) || (QuadGT_Progs[prog].config==6))
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=true;
    MainForm->ReverbInMix->Visible=true;
    // TBD: Disable input 1, selection 3
  }
  else if ((QuadGT_Progs[prog].config==2) || (QuadGT_Progs[prog].config==7))
  {
    MainForm->QuadReverb->Visible=false;
  }
  else if (QuadGT_Progs[prog].config==3)
  {
    MainForm->QuadReverb->Visible=false;
  }
  else if (QuadGT_Progs[prog].config==4)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=FALSE;
    MainForm->ReverbInMix->Visible=FALSE;
  }

  if (MainForm->QuadReverb->Visible == true)
  {
    MainForm->ReverbType->ItemIndex = QuadGT_Progs[prog].reverb_mode;

    // TBD: Scale translate all these for screen display

    MainForm->ReverbInput1->ItemIndex = QuadGT_Progs[prog].reverb_input_1;
    MainForm->ReverbInput2->ItemIndex = QuadGT_Progs[prog].reverb_input_2;
    MainForm->ReverbInMix->Position = QuadGT_Progs[prog].reverb_input_mix;

    MainForm->ReverbPreDelay->Position = QuadGT_Progs[prog].reverb_predelay;
    MainForm->ReverbPreMix->Position = QuadGT_Progs[prog].reverb_predelay_mix;

    MainForm->ReverbDecay->Position = QuadGT_Progs[prog].reverb_decay;
    MainForm->ReverbDiffusion->Position = QuadGT_Progs[prog].reverb_diffusion;

    MainForm->ReverbLowDecay->Position = QuadGT_Progs[prog].reverb_low_decay;
    MainForm->ReverbHighDecay->Position = QuadGT_Progs[prog].reverb_high_decay;

    MainForm->ReverbDensity->Position = QuadGT_Progs[prog].reverb_density;

    MainForm->ReverbGateOn->Checked = (QuadGT_Progs[prog].reverb_gate==1) ? TRUE : FALSE;
    MainForm->ReverbGateHold->Position = QuadGT_Progs[prog].reverb_gate_hold;
    MainForm->ReverbGateRelease->Position = QuadGT_Progs[prog].reverb_gate_release;
    MainForm->ReverbGateLevel->Position = QuadGT_Progs[prog].reverb_gated_level;
  }
}

void QuadGT_Display_Update_Delay(const UInt8 prog)
{
  UInt8 mode;

  mode=QuadGT_Progs[prog].delay_mode;
  MainForm->DelayMode->ItemIndex=mode;
  MainForm->DelayInput->ItemIndex=QuadGT_Progs[prog].delay_input;

  // Mono
  if (mode ==0)
  {
    MainForm->DelayTap->Visible=FALSE;
    MainForm->DelayRight->Visible=FALSE;
    MainForm->DelayRightFeedback->Visible=FALSE;

    MainForm->DelayDelay->Position=QuadGT_Progs[prog].delay;
    MainForm->DelayFeedback->Position=QuadGT_Progs[prog].delay_feedback;

  }

  // Stereo
  else if (mode ==1)
  {
    MainForm->DelayTap->Visible=FALSE;
    MainForm->DelayRight->Visible=TRUE;
    MainForm->DelayRightFeedback->Visible=TRUE;

    MainForm->DelayDelay->Position=QuadGT_Progs[prog].delay_left;
    MainForm->DelayFeedback->Position=QuadGT_Progs[prog].delay_left_feedback;
    MainForm->DelayRight->Position=QuadGT_Progs[prog].delay_right;
    MainForm->DelayRightFeedback->Position=QuadGT_Progs[prog].delay_right_feedback;
  }

  // Ping Pong
  else if (mode ==2)
  {
    MainForm->DelayTap->Visible=FALSE;
    MainForm->DelayRight->Visible=TRUE;
    MainForm->DelayRightFeedback->Visible=TRUE;
  }

  // Multitap
  else if (mode ==3)
  {
    MainForm->DelayTap->Visible=TRUE;
  }
}

void QuadGT_Display_Update_Pitch(const UInt8 prog)
{
  if ((QuadGT_Progs[prog].config==CFG0_EQ_PITCH_DELAY_REVERB) ||
      (QuadGT_Progs[prog].config==CFG3_5BANDEQ_PITCH_DELAY))
  {
    MainForm->QuadPitch->Visible=TRUE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;

    MainForm->PitchMode->ItemIndex = QuadGT_Progs[prog].pitch_mode;
    MainForm->PitchInput->ItemIndex = QuadGT_Progs[prog].pitch_input;
    MainForm->PitchWave->ItemIndex = QuadGT_Progs[prog].lfo_waveform;
    MainForm->PitchSpeed->Position = QuadGT_Progs[prog].lfo_speed;
    MainForm->PitchDepth->Position = QuadGT_Progs[prog].lfo_depth;
    MainForm->PitchFeedback->Position = QuadGT_Progs[prog].pitch_feedback;
  }
  else if (QuadGT_Progs[prog].config==CFG1_LESLIE_DELAY_REVERB)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=TRUE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;
  }
  else if ((QuadGT_Progs[prog].config==CFG2_GEQ_DELAY) || 
           (QuadGT_Progs[prog].config==CFG6_RESONATOR_DELAY_REVERB) || 
           (QuadGT_Progs[prog].config==CFG7_SAMPLING))
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;
  }
  else if (QuadGT_Progs[prog].config==CFG4_3BANDEQ_REVERB)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=TRUE;
  }
  else if (QuadGT_Progs[prog].config==CFG5_RINGMOD_DELAY_REVERB)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=TRUE;
    MainForm->QuadChorus->Visible=FALSE;
  }

}

void QuadGT_Display_Update_Eq(const UInt8 prog)
{
  // 3 Band Eq
  if ((QuadGT_Progs[prog].config==0) || (QuadGT_Progs[prog].config==4))
  {
    MainForm->PanelQuadEq3->Visible=TRUE;

    MainForm->EqFreq1->Position = QuadGT_Progs[prog].low_eq_freq;
    MainForm->EqAmp1->Position = -1 * (QuadGT_Progs[prog].low_eq_amp - MainForm->EqAmp1->Max);
    MainForm->Eq3Amp1Val->Text = AnsiString(((double)(QuadGT_Progs[prog].low_eq_amp - MainForm->EqAmp1->Max))/20.0);

    MainForm->EqFreq2->Position = QuadGT_Progs[prog].mid_eq_freq;
    MainForm->EqAmp2->Position = -1 * (QuadGT_Progs[prog].mid_eq_amp - MainForm->EqAmp2->Max);
    MainForm->Eq3Amp2Val->Text = AnsiString(((double)(QuadGT_Progs[prog].mid_eq_amp - MainForm->EqAmp2->Max))/20.0);
    MainForm->EqQ2->Position = QuadGT_Progs[prog].mid_eq_q;

    MainForm->EqFreq3->Position = QuadGT_Progs[prog].high_eq_freq;
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
  if (QuadGT_Progs[prog].config==3) 
  {
    MainForm->QuadEq5->Visible=TRUE;

    MainForm->Eq5Freq1->Position = QuadGT_Progs[prog].low_eq_freq;
    MainForm->Eq5Amp1->Position = -1 * (QuadGT_Progs[prog].low_eq_amp - MainForm->Eq5Amp1->Max);
    MainForm->Eq5Amp1Val->Text = AnsiString(((double)(QuadGT_Progs[prog].low_eq_amp - MainForm->Eq5Amp1->Max))/20.0);

    MainForm->Eq5Freq2->Position = QuadGT_Progs[prog].low_mid_eq_freq;
    MainForm->Eq5Amp2->Position = -1 * (QuadGT_Progs[prog].low_mid_eq_amp - MainForm->Eq5Amp2->Max);
    MainForm->Eq5Amp2Val->Text = AnsiString((double)(QuadGT_Progs[prog].low_mid_eq_amp - MainForm->Eq5Amp2->Max)/20.0);
    MainForm->Eq5Q2->Position = QuadGT_Progs[prog].low_mid_eq_q;

    MainForm->Eq5Freq3->Position = QuadGT_Progs[prog].mid_eq_freq;
    MainForm->Eq5Amp3->Position = -1 * (QuadGT_Progs[prog].mid_eq_amp - MainForm->Eq5Amp3->Max);
    MainForm->Eq5Amp3Val->Text = AnsiString((double)(QuadGT_Progs[prog].mid_eq_amp - MainForm->Eq5Amp3->Max)/20.0);
    MainForm->Eq5Q3->Position = QuadGT_Progs[prog].mid_eq_q;

    MainForm->Eq5Freq4->Position = QuadGT_Progs[prog].high_mid_eq_freq;
    MainForm->Eq5Amp4->Position = -1 * (QuadGT_Progs[prog].high_mid_eq_amp - MainForm->Eq5Amp4->Max);
    MainForm->Eq5Amp4Val->Text = AnsiString((double)(QuadGT_Progs[prog].high_mid_eq_amp - MainForm->Eq5Amp4->Max)/20.0);
    MainForm->Eq5Q4->Position = QuadGT_Progs[prog].high_mid_eq_q;

    MainForm->Eq5Freq5->Position = QuadGT_Progs[prog].high_eq_freq;
    MainForm->Eq5Amp5->Position = -1 * (QuadGT_Progs[prog].high_eq_amp - MainForm->Eq5Amp5->Max);
    MainForm->Eq5Amp5Val->Text = AnsiString((double)(QuadGT_Progs[prog].high_eq_amp - MainForm->Eq5Amp5->Max)/20.0);
  }
  else 
  {
    MainForm->QuadEq5->Visible=FALSE;
  }

  // Graphic Eq
  if (QuadGT_Progs[prog].config==2) 
  {
    MainForm->QuadGraphEq->Visible=TRUE;

    MainForm->GEQPreset->ItemIndex =QuadGT_Progs[prog].geq_preset;
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

void QuadGT_Display_Update_Mix(const UInt8 prog)
{
  UInt8 val;

  // Direct level
  MainForm->MixDirect->Position=MainForm->MixDirect->Max - QuadGT_Progs[prog].direct_level;

  // Master Fx Level
  if (QuadGT_Progs[prog].config != 7)
  {
    val=QuadGT_Progs[prog].master_effects_level;
    MainForm->MixMaster->Position=99-val;
    MainForm->MixMaster->Visible=TRUE;
  }
  else
  {
    MainForm->MixMaster->Visible=FALSE;
  }

  // Pre or Post Eq
  MainForm->PrePostEq->ItemIndex=QuadGT_Progs[prog].prepost_eq;
 
  QuadGT_Display_Update_PrePostEq(prog);

  // Pitch Level
  if ( (QuadGT_Progs[prog].config == CFG0_EQ_PITCH_DELAY_REVERB) ||
       (QuadGT_Progs[prog].config == CFG3_5BANDEQ_PITCH_DELAY))
  {
    val=QuadGT_Progs[prog].pitch_level;
    MainForm->MixPitch->Position=99-val;

    MainForm->MixPitch->Visible=TRUE;
  }
  else
  {
    MainForm->MixPitch->Visible=FALSE;
  }

  // Delay Level
  val=QuadGT_Progs[prog].delay_level;
  MainForm->MixDelay->Position=99-val;

  // Reverb Level
  val=QuadGT_Progs[prog].reverb_level;
  MainForm->MixReverb->Position=99-val;

  // Modulation (on/off)
  val=QuadGT_Progs[prog].mix_mod;
  if (val == 0) MainForm->Modulation->Checked=FALSE;
  else MainForm->Modulation->Checked=TRUE;

  // Mod Depth
  // Mod Speed
 
  // Leslie Level

  // Leslie Level
  if (QuadGT_Progs[prog].config == CFG1_LESLIE_DELAY_REVERB)
  {
    MainForm->MixLeslieRingModReson->Visible=TRUE;
    MainForm->LeslieRingModReson->Caption="Leslie";
    MainForm->LeslieRingModReson->Visible=TRUE;
  }

  // Ring Level
  else if (QuadGT_Progs[prog].config == CFG5_RINGMOD_DELAY_REVERB)
  {
    MainForm->MixLeslieRingModReson->Visible=TRUE;
    MainForm->LeslieRingModReson->Caption="RingMod";
    MainForm->LeslieRingModReson->Visible=TRUE;
  }
  
  // Resonator Level
  else if (QuadGT_Progs[prog].config == CFG6_RESONATOR_DELAY_REVERB)
  {
    MainForm->MixLeslieRingModReson->Visible=TRUE;
    MainForm->LeslieRingModReson->Caption="Reson.";
    MainForm->LeslieRingModReson->Visible=TRUE;
  }

  else
  {
    MainForm->MixLeslieRingModReson->Visible=FALSE;
    MainForm->LeslieRingModReson->Visible=FALSE;
  }
}

void QuadGT_Display_Update_Mod(const UInt8 prog)
{
  // Modulation
  switch (QuadGT_Progs[prog].config)
  {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      MainForm->QuadModSource1->ItemIndex=QuadGT_Progs[prog].mod_source[0];
      MainForm->QuadModTarget1->ItemIndex=QuadGT_Progs[prog].mod_target[0];
      MainForm->QuadModAmp1->Position=QuadGT_Progs[prog].mod_amp[0]-99;
      break;

    case 7:
    default:
      break;
  }
}

void QuadGT_Display_Update_Preamp(const UInt8 prog)
{
  UInt8 val;

  // Compression (0-7)
  MainForm->PreComp->Position=MainForm->PreComp->Max-QuadGT_Progs[prog].comp;
  MainForm->PreComp->Position = MainForm->PreComp->Max - QuadGT_Progs[prog].comp;

  // Overdrive (0-7)
  MainForm->PreOd->Position=MainForm->PreOd->Max-QuadGT_Progs[prog].od;
  MainForm->PreOd->Position = MainForm->PreOd->Max - QuadGT_Progs[prog].od;

  // Distortion (0-8)
  MainForm->PreDist->Position=MainForm->PreDist->Max-QuadGT_Progs[prog].dist;
  MainForm->PreDist->Position = MainForm->PreDist->Max - QuadGT_Progs[prog].dist;

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

  /* Preamp Output Level (0-99) */
  if (MainForm->QuadConfig->ItemIndex == CFG7_SAMPLING)
  {
    MainForm->PreOutLevel->Visible = FALSE;
  }
  else
  {
    MainForm->PreOutLevel->Position = MainForm->PreOutLevel->Max-QuadGT_Progs[prog].preamp_out_level;
    MainForm->PreOutLevel->Visible = TRUE;
  }

}

void EqBarChange(TTrackBar *bar, TEdit *text, UInt16* param)
{
  *param= -1 * bar->Position + bar->Max;       // Convert to range: 0 to 560
  bar->Hint=AnsiString(bar->Position/-20.0);   // Display in range: -14 to 14
  text->Text=AnsiString(bar->Position/-20.0);
}

void VertBarChange(TTrackBar *bar, UInt8* param)
{
  *param=(bar->Max - bar->Position);
  bar->Hint=AnsiString(bar->Max - bar->Position);
}

void QuadGT_Display_Update_PrePostEq(UInt8 prog)
{
  // Preamp Level
  if (MainForm->PrePostEq->ItemIndex==0)
  {
    MainForm->MixPreampEq->Position=99-QuadGT_Progs[prog].preamp_level;
    MainForm->MixPreampEq->Visible=TRUE;
    MainForm->PreAmpEq->Visible=TRUE;
    MainForm->PreAmpEq->Caption="Preamp";
  }
  // Eq Level
  else
  {
    MainForm->MixPreampEq->Position=99-QuadGT_Progs[prog].eq_level;
    MainForm->MixPreampEq->Visible=TRUE;
    MainForm->PreAmpEq->Visible=TRUE;
    MainForm->PreAmpEq->Caption=" Eq ";
  }
}

void __fastcall TMainForm::QuadParamChange(TObject *Sender)
{
  UInt8 prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  //FormDebug->Log(NULL,"Change: "+AnsiString(prog));

  if (Sender == MainForm->QuadConfig)
  {
    QuadGT_Progs[prog].config=MainForm->QuadConfig->ItemIndex;
    QuadGT_Display_Update_Patch(prog);
  }
  else if (Sender == MainForm->PreComp) VertBarChange((TTrackBar *)Sender, &QuadGT_Progs[prog].comp);
  else if (Sender == MainForm->PreOd) VertBarChange((TTrackBar *)Sender, &QuadGT_Progs[prog].od);
  else if (Sender == MainForm->PreDist) VertBarChange((TTrackBar *)Sender, &QuadGT_Progs[prog].dist);
  else if (Sender == MainForm->EditQuadPatchName)
  {
    strncpy(QuadGT_Progs[prog].name,MainForm->EditQuadPatchName->Text.c_str(),NAME_LENGTH);
  }

  // 3 Band Eq
  // TBD: EqFreq1 EqFreq2 EqQ2 EqFreq3
  else if (Sender == MainForm->Eq3Mode)
  {
    QuadGT_Progs[prog].eq_mode = MainForm->Eq3Mode->ItemIndex;
    QuadGT_Display_Update_Eq(prog);
    QuadGT_Display_Update_Resonator(prog);
  }
  else if (Sender == MainForm->EqAmp1)
  {
    QuadGT_Progs[prog].low_eq_amp = -1 * MainForm->EqAmp1->Position + MainForm->EqAmp1->Max;
    MainForm->Eq3Amp1Val->Text = AnsiString(((double)(QuadGT_Progs[prog].low_eq_amp - MainForm->EqAmp1->Max))/20.0);
  }
  else if (Sender == MainForm->EqAmp2)
  {
    QuadGT_Progs[prog].mid_eq_amp = -1 * MainForm->EqAmp2->Position + MainForm->EqAmp2->Max;
    MainForm->Eq3Amp2Val->Text = AnsiString(((double)(QuadGT_Progs[prog].mid_eq_amp - MainForm->EqAmp2->Max))/20.0);
  }
  else if (Sender == MainForm->EqAmp3)
  {
    QuadGT_Progs[prog].high_eq_amp = -1 * MainForm->EqAmp3->Position + MainForm->EqAmp3->Max;
    MainForm->Eq3Amp3Val->Text = AnsiString(((double)(QuadGT_Progs[prog].high_eq_amp - MainForm->EqAmp3->Max))/20.0);
  }

  // 5 Band Eq
  else if (Sender == MainForm->Eq5Mode)
  {
    QuadGT_Progs[prog].eq_mode = MainForm->Eq5Mode->ItemIndex;
    QuadGT_Display_Update_Eq(prog);
    QuadGT_Display_Update_Resonator(prog);
  }
  else if (Sender == MainForm->Eq5Amp1) EqBarChange(MainForm->Eq5Amp1, MainForm->Eq5Amp1Val, &QuadGT_Progs[prog].low_eq_amp);
  else if (Sender == MainForm->Eq5Amp2) EqBarChange(MainForm->Eq5Amp2, MainForm->Eq5Amp2Val, &QuadGT_Progs[prog].low_mid_eq_amp);
  else if (Sender == MainForm->Eq5Amp3) EqBarChange(MainForm->Eq5Amp3, MainForm->Eq5Amp3Val, &QuadGT_Progs[prog].mid_eq_amp);
  else if (Sender == MainForm->Eq5Amp4) EqBarChange(MainForm->Eq5Amp4, MainForm->Eq5Amp4Val, &QuadGT_Progs[prog].high_mid_eq_amp);
  else if (Sender == MainForm->Eq5Amp5) EqBarChange(MainForm->Eq5Amp5, MainForm->Eq5Amp5Val, &QuadGT_Progs[prog].high_eq_amp);

  // Mix parameters
  else if (Sender == MainForm->MixDirect) VertBarChange((TTrackBar *)Sender, &QuadGT_Progs[prog].direct_level);
  else if (Sender == MainForm->MixMaster) VertBarChange((TTrackBar *)Sender, &QuadGT_Progs[prog].master_effects_level);
  else if (Sender == MainForm->MixPitch) VertBarChange((TTrackBar *)Sender, &QuadGT_Progs[prog].pitch_level);
  else if (Sender == MainForm->PrePostEq) 
  {
    QuadGT_Progs[prog].prepost_eq=MainForm->PrePostEq->ItemIndex;
    QuadGT_Display_Update_PrePostEq(prog);
  }
  else if (Sender == MainForm->MixPreampEq) 
  {
    if (QuadGT_Progs[prog].prepost_eq==0) VertBarChange((TTrackBar *)Sender, &QuadGT_Progs[prog].preamp_level);
    else                                  VertBarChange((TTrackBar *)Sender, &QuadGT_Progs[prog].eq_level);
  }
  else if (Sender == MainForm->MixDelay) VertBarChange((TTrackBar *)Sender, &QuadGT_Progs[prog].delay_level);
  else if (Sender == MainForm->MixReverb) VertBarChange((TTrackBar *)Sender, &QuadGT_Progs[prog].reverb_level);
  else if (Sender == MainForm->Modulation) QuadGT_Progs[prog].mix_mod=MainForm->Modulation->Checked;
}

void __fastcall TMainForm::DelayModeClick(TObject *Sender)
{
  UInt8 prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  QuadGT_Progs[prog].delay_mode=MainForm->DelayMode->ItemIndex;
  QuadGT_Display_Update_Delay(prog);
}
//---------------------------------------------------------------------------

void QuadGT_Display_Update_Resonator(const UInt8 prog)
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

  save_patch(data,QUAD_PATCH_SIZE,"patch_out.qgt");

  //-------------------------------------------------------------------------
  // Eq/Res/Sample/Tap1 Parameters (0x00 - 0x19)
  //-------------------------------------------------------------------------
  // TBD: Select Resonator * or Eq
  QuadGT_Encode_16Bit(QuadGT_Progs[prog].low_eq_freq, &data[LOW_EQ_FREQ_IDX]);
  QuadGT_Encode_16Bit(QuadGT_Progs[prog].low_eq_amp, &data[LOW_EQ_AMP_IDX]);

  QuadGT_Encode_16Bit(QuadGT_Progs[prog].mid_eq_freq, &data[MID_EQ_FREQ_IDX]);
  data[MID_EQ_BW_IDX] = QuadGT_Progs[prog].mid_eq_q;
  QuadGT_Encode_16Bit(QuadGT_Progs[prog].mid_eq_amp, &data[MID_EQ_AMP_IDX]);

  // TBD: Select Resonator * or Eq
  QuadGT_Encode_16Bit(QuadGT_Progs[prog].high_eq_freq, &data[HIGH_EQ_FREQ_IDX]);

  QuadGT_Encode_16Bit(QuadGT_Progs[prog].high_eq_amp, &data[HIGH_EQ_AMP_IDX]);

  // TBD: Select Leslie or Tap 1
  // TBD: Encode

  // TBD: Select Resonator ** or Eq
  QuadGT_Encode_16Bit(QuadGT_Progs[prog].low_mid_eq_freq, &data[LOW_MID_EQ_FREQ_IDX]);
  data[LOW_MID_EQ_BW_IDX] = QuadGT_Progs[prog].low_mid_eq_q;
  QuadGT_Encode_16Bit(QuadGT_Progs[prog].low_mid_eq_amp, &data[LOW_MID_EQ_AMP_IDX]);
  QuadGT_Encode_16Bit(QuadGT_Progs[prog].high_mid_eq_freq, &data[HIGH_MID_EQ_FREQ_IDX]);
  data[HIGH_MID_EQ_BW_IDX] = QuadGT_Progs[prog].high_mid_eq_q;
  QuadGT_Encode_16Bit(QuadGT_Progs[prog].high_mid_eq_amp, &data[HIGH_MID_EQ_AMP_IDX]);

  // Select Sampling or Tap 1
  // Note: Tap 1 Delay is a split parameter
  if (QuadGT_Progs[prog].config == CFG7_SAMPLING) 
  {
    data[SAMPLE_LENGTH_IDX] = QuadGT_Progs[prog].sample_length;
  }
  else 
  {
    //data[TAP1_DELAY_LSB_IDX]   = TBD;
    //data[TAP1_DELAY_MSB_IDX]   = TBD;
  }


  //-------------------------------------------------------------------------
  // Pitch Parameters (0x1A - 
  //-------------------------------------------------------------------------
  data[PITCH_MODE_IDX]   = QuadGT_Progs[prog].pitch_mode;

  data[PITCH_INPUT_IDX]  = QuadGT_Progs[prog].pitch_input & BIT0;
  data[RES4_AMP_IDX]    |= (QuadGT_Progs[prog].res_amp[3] << 1) & BITS1to7;

  data[LFO_WAVEFORM_IDX] = QuadGT_Progs[prog].lfo_waveform & BIT0;
  data[EQ_PRESET_IDX]   |= (QuadGT_Progs[prog].geq_preset << 1) & BITS1to7;

  data[LFO_SPEED_IDX] = QuadGT_Progs[prog].lfo_speed;

  data[LFO_DEPTH_IDX] = QuadGT_Progs[prog].lfo_depth;

  //-------------------------------------------------------------------------
  // Delay Parameters (0x27 - 0x31)
  //-------------------------------------------------------------------------
  data[DELAY_MODE_IDX]   = QuadGT_Progs[prog].delay_mode;

  data[DELAY_INPUT_IDX]  = QuadGT_Progs[prog].delay_input & BIT0;
  data[RES5_AMP_IDX]    |= (QuadGT_Progs[prog].res_amp[4] << 1) & BITS1to7;

  data[DELAY_INPUT_MIX_IDX] = QuadGT_Progs[prog].delay_input_mix;

  QuadGT_Encode_16Bit(QuadGT_Progs[prog].delay, &data[DELAY_IDX]);

  //-------------------------------------------------------------------------
  // Reverb Parameters (0x32 - 0x43)
  //-------------------------------------------------------------------------
  data[REVERB_MODE_IDX]   = QuadGT_Progs[prog].reverb_mode;

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

  if (QuadGT_Progs[prog].config==1)
  {
    data[LESLIE_LEVEL_IDX] = QuadGT_Progs[prog].leslie_level;
  }
  else if (QuadGT_Progs[prog].config==5)
  {
    data[RING_MOD_LEVEL_IDX] = QuadGT_Progs[prog].ring_mod_level;
  }
  else
  {
    data[PITCH_LEVEL_IDX] = QuadGT_Progs[prog].pitch_level;
  }

  data[DELAY_LEVEL_IDX ] = QuadGT_Progs[prog].delay_level;
  data[REVERB_LEVEL_IDX ] = QuadGT_Progs[prog].reverb_level;

  //-------------------------------------------------------------------------
  // Modulation Parameters (0x50 - 0x67)
  //-------------------------------------------------------------------------
  
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
  data[RING_MOD_OUTPUT_MIX_IDX]  = QuadGT_Progs[prog].ring_mod_output_mix;
  data[RING_MOD_DEL_REV_MIX_IDX]  = QuadGT_Progs[prog].ring_mod_del_rev_mix;
  data[PAN_SPEED_IDX]  = QuadGT_Progs[prog].pan_speed;
  data[PAN_DEPTH_IDX]  = QuadGT_Progs[prog].pan_depth;
  
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
  
  return 0;
}


//---------------------------------------------------------------------------
// Name     : QuadGT_Convert_QuadGT_To_Internal
// Description : Convert raw Quadraverb GT data to internal data structure
//---------------------------------------------------------------------------
UInt32 QuadGT_Convert_QuadGT_To_Internal(UInt8 prog, UInt8* data)
{
  if (prog >= QUAD_NUM_PATCH) return 1;

  save_patch(data,QUAD_PATCH_SIZE,"patch_in.qgt");

  QuadGT_Progs[prog].config= data[CONFIG_IDX];
  memcpy(QuadGT_Progs[prog].name, &data[NAME_IDX], NAME_LENGTH);
  QuadGT_Progs[prog].name[NAME_LENGTH]=0;


  //-------------------------------------------------------------------------
  // Preamp Parameters (0x7C - 0x7F)
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].comp             = (data[PREAMP_COMP_IDX] & BITS4to6) >> 4;
  QuadGT_Progs[prog].od               = (data[PREAMP_OD_IDX]   & BITS5to7) >> 5;
  QuadGT_Progs[prog].dist             = (data[PREAMP_DIST_IDX] & BITS0to3);
  QuadGT_Progs[prog].preamp_tone      = (data[PREAMP_TONE_IDX] & BITS2to3) >> 2;
  QuadGT_Progs[prog].bass_boost       = data[BASS_BOOST_IDX];
  QuadGT_Progs[prog].cab_sim          = data[CAB_SIM_IDX];
  QuadGT_Progs[prog].effect_loop      = data[EFFECT_LOOP_IDX];
  QuadGT_Progs[prog].preamp_gate      = (data[PREAMP_GATE_IDX] & BITS0to4);
  QuadGT_Progs[prog].preamp_out_level = data[PREAMP_OUT_LEVEL_IDX];

  //-------------------------------------------------------------------------
  // Eq Parameters
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].eq_mode=((data[EQ_MODE_IDX]&BIT7)>>7);                // Eq Mode: Eq or Eq/Resonator
  QuadGT_Progs[prog].geq_preset = (data[EQ_PRESET_IDX]&BITS1to7)>>1;       // User or Preset 1-6

  // 3 and 5 Band Eq
  if ( (QuadGT_Progs[prog].config==0) || (QuadGT_Progs[prog].config==4) ||
       (QuadGT_Progs[prog].config==3))
  {
    QuadGT_Progs[prog].low_eq_freq   = QuadGT_Decode_16Bit(&data[LOW_EQ_FREQ_IDX]);
    FormDebug->Log(NULL, "LOW FREQ="+AnsiString(QuadGT_Progs[prog].low_eq_freq)+"Hz");
    QuadGT_Progs[prog].low_eq_amp    = QuadGT_Decode_16Bit(&data[LOW_EQ_AMP_IDX]);
    QuadGT_Progs[prog].mid_eq_freq   = QuadGT_Decode_16Bit(&data[MID_EQ_FREQ_IDX]);
    FormDebug->Log(NULL, "MID FREQ="+AnsiString(QuadGT_Progs[prog].mid_eq_freq)+"Hz");
    QuadGT_Progs[prog].mid_eq_amp    = QuadGT_Decode_16Bit(&data[MID_EQ_AMP_IDX]);
    QuadGT_Progs[prog].mid_eq_q      = data[MID_EQ_BW_IDX];
    QuadGT_Progs[prog].high_eq_freq  = QuadGT_Decode_16Bit(&data[HIGH_EQ_FREQ_IDX]);
    FormDebug->Log(NULL, "HIGH FREQ="+AnsiString(QuadGT_Progs[prog].high_eq_freq)+"Hz");
    QuadGT_Progs[prog].high_eq_amp   = QuadGT_Decode_16Bit(&data[HIGH_EQ_AMP_IDX]);
  }

  // 5 Band Eq only
  //if (QuadGT_Progs[prog].config==3)
  {
    QuadGT_Progs[prog].low_mid_eq_freq   = QuadGT_Decode_16Bit(&data[LOW_MID_EQ_FREQ_IDX]);
    FormDebug->Log(NULL, "LOW MID FREQ="+AnsiString(QuadGT_Progs[prog].low_mid_eq_freq)+"Hz");
    QuadGT_Progs[prog].low_mid_eq_amp    = QuadGT_Decode_16Bit(&data[LOW_MID_EQ_AMP_IDX]);
    FormDebug->Log(NULL, "LOW MID AMP="+AnsiString(QuadGT_Progs[prog].low_mid_eq_amp));
    QuadGT_Progs[prog].low_mid_eq_q      = data[LOW_MID_EQ_BW_IDX];
    FormDebug->Log(NULL, "LOW MID Q="+AnsiString(QuadGT_Progs[prog].low_mid_eq_q));

    QuadGT_Progs[prog].high_mid_eq_freq   = QuadGT_Decode_16Bit(&data[HIGH_MID_EQ_FREQ_IDX]);
    FormDebug->Log(NULL, "HIGH MID FREQ="+AnsiString(QuadGT_Progs[prog].high_mid_eq_freq)+"Hz");
    QuadGT_Progs[prog].high_mid_eq_amp    = QuadGT_Decode_16Bit(&data[HIGH_MID_EQ_AMP_IDX]);
    QuadGT_Progs[prog].high_mid_eq_q      = data[HIGH_MID_EQ_BW_IDX];
  }

  // Graphic Eq
  if (QuadGT_Progs[prog].config==2)
  {
    QuadGT_Progs[prog].geq_16hz   = data[GEQ_16HZ_IDX]-14;
    QuadGT_Progs[prog].geq_32hz   = data[GEQ_32HZ_IDX]-14;
    QuadGT_Progs[prog].geq_62hz   = data[GEQ_62HZ_IDX]-14;
    QuadGT_Progs[prog].geq_126hz  = data[GEQ_126HZ_IDX]-14;
    QuadGT_Progs[prog].geq_250hz  = data[GEQ_250HZ_IDX]-14;
    QuadGT_Progs[prog].geq_500hz  = data[GEQ_500HZ_IDX]-14;
    QuadGT_Progs[prog].geq_1khz   = data[GEQ_1KHZ_IDX]-14;
    QuadGT_Progs[prog].geq_2khz   = data[GEQ_2KHZ_IDX]-14;
    QuadGT_Progs[prog].geq_4khz   = data[GEQ_4KHZ_IDX]-14;
    QuadGT_Progs[prog].geq_8khz   = data[GEQ_8KHZ_IDX]-14;
    QuadGT_Progs[prog].geq_16khz  = data[GEQ_16KHZ_IDX]-14;
  }


  //-------------------------------------------------------------------------
  // Pitch Parameters
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].pitch_mode    = data[PITCH_MODE_IDX];
  QuadGT_Progs[prog].pitch_input   = data[PITCH_INPUT_IDX] & BIT0;
  QuadGT_Progs[prog].lfo_waveform  = data[LFO_WAVEFORM_IDX] & BIT0;
  QuadGT_Progs[prog].lfo_speed     = data[LFO_SPEED_IDX];
  QuadGT_Progs[prog].lfo_depth     = data[LFO_DEPTH_IDX];
  QuadGT_Progs[prog].pitch_feedback= data[PITCH_FEEDBACK_IDX];

  //-------------------------------------------------------------------------
  // Delay Parameters
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].delay_mode=data[DELAY_MODE_IDX];
  QuadGT_Progs[prog].delay_input=data[DELAY_INPUT_IDX]&BIT0;
  QuadGT_Progs[prog].delay_input_mix = data[DELAY_INPUT_MIX_IDX];
  QuadGT_Progs[prog].delay=data[DELAY_IDX];
  QuadGT_Progs[prog].delay_feedback=data[DELAY_FEEDBACK_IDX];
  QuadGT_Progs[prog].delay_left=data[DELAY_LEFT_IDX];
  QuadGT_Progs[prog].delay_left_feedback=data[DELAY_LEFT_FEEDBACK_IDX];
  QuadGT_Progs[prog].delay_right=data[DELAY_RIGHT_IDX];
  QuadGT_Progs[prog].delay_right_feedback=data[DELAY_RIGHT_FEEDBACK_IDX];

  //-------------------------------------------------------------------------
  // Reverb Parameters
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].reverb_mode = data[REVERB_MODE_IDX];
  QuadGT_Progs[prog].reverb_input_1	        = data[REVERB_INPUT_1_IDX];
  QuadGT_Progs[prog].reverb_input_2	        = data[REVERB_INPUT_2_IDX];
  QuadGT_Progs[prog].reverb_input_mix	        = data[REVERB_INPUT_MIX_IDX];
  QuadGT_Progs[prog].reverb_predelay	        = data[REVERB_PREDELAY_IDX];
  QuadGT_Progs[prog].reverb_predelay_mix	= data[REVERB_PREDELAY_MIX_IDX];
  QuadGT_Progs[prog].reverb_decay	        = data[REVERB_DECAY_IDX];
  QuadGT_Progs[prog].reverb_diffusion	        = data[REVERB_DIFFUSION_IDX];
  QuadGT_Progs[prog].reverb_low_decay	        = data[REVERB_LOW_DECAY_IDX];
  QuadGT_Progs[prog].reverb_high_decay	        = data[REVERB_HIGH_DECAY_IDX];
  QuadGT_Progs[prog].reverb_density	        = data[REVERB_DENSITY_IDX];
  QuadGT_Progs[prog].reverb_gate	        = data[REVERB_GATE_IDX];
  QuadGT_Progs[prog].reverb_gate_hold	        = data[REVERB_GATE_HOLD_IDX];
  QuadGT_Progs[prog].reverb_gate_release	= data[REVERB_GATE_RELEASE_IDX];
  QuadGT_Progs[prog].reverb_gated_level	        = data[REVERB_GATED_LEVEL_IDX];

  //-------------------------------------------------------------------------
  // Mix Parameters
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].prepost_eq=data[PREPOST_EQ_IDX] & BIT0;
  QuadGT_Progs[prog].direct_level=(data[DIRECT_LEVEL_IDX ] & BITS1to7) >> 1;
  QuadGT_Progs[prog].master_effects_level=data[MASTER_EFFECTS_LEVEL_IDX ];

  if (QuadGT_Progs[prog].prepost_eq == 0) QuadGT_Progs[prog].preamp_level=data[PREAMP_LEVEL_IDX];
  else                                    QuadGT_Progs[prog].eq_level=data[EQ_LEVEL_IDX];

  if (QuadGT_Progs[prog].config==1)
  {
    QuadGT_Progs[prog].leslie_level=data[LESLIE_LEVEL_IDX];
  }
  else if (QuadGT_Progs[prog].config==5)
  {
    QuadGT_Progs[prog].ring_mod_level=data[RING_MOD_LEVEL_IDX];
  }
  else
  {
    QuadGT_Progs[prog].pitch_level=data[PITCH_LEVEL_IDX];
  }

  QuadGT_Progs[prog].delay_level=data[DELAY_LEVEL_IDX ];
  QuadGT_Progs[prog].reverb_level=data[REVERB_LEVEL_IDX ];
  QuadGT_Progs[prog].mix_mod=(data[MIX_MOD_IDX] & BITS6to7) >> 6;
  // TBD: MOD_DEPTH what is IDX?  Same as LFO?
  // TBD: MOD_SPEED what is IDX?  Same as LFO?

  //-------------------------------------------------------------------------
  // Modulation Parameters
  //-------------------------------------------------------------------------
  QuadGT_Progs[prog].mod_source[0]=data[MOD1_SOURCE_IDX];
  QuadGT_Progs[prog].mod_target[0]=data[MOD1_TARGET_IDX];
  QuadGT_Progs[prog].mod_amp[0]   =data[MOD1_AMP_IDX];

  QuadGT_Progs[prog].mod_source[1]=data[MOD2_SOURCE_IDX];
  QuadGT_Progs[prog].mod_target[1]=data[MOD2_TARGET_IDX];
  QuadGT_Progs[prog].mod_amp[1]   =data[MOD2_AMP_IDX];

  //-------------------------------------------------------------------------
  // Resonator Parameters
  //-------------------------------------------------------------------------
  //TBD: Determine which set of parameters are used in which mode
  QuadGT_Progs[prog].res_amp[4] = (data[RES5_AMP_IDX] & BITS1to7) >> 1;

  return 0;
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

  bank_file = fopen("quadgt.bnk","wb");
  if (bank_file == NULL)
  {
    FormError->ShowError(ferror(bank_file),"opening bank file");
    return;
  }

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

  bank_file = fopen("quadgt.bnk","rb");
  if (bank_file == NULL)
  {
    FormError->ShowError(ferror(bank_file),"opening bank file");
    return;
  }
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
  QuadGT_Display_Update_Patch(prog);
}

//---------------------------------------------------------------------------
// Name        : 
// Description : Load an entire bank of programs from a raw binary dump of a 
//               complete SYSEX on disk.
// Parameter 1 : 
// Parameter 2 : 
// Parameters  : NONE.
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
  QuadGT_Display_Update_Patch(prog);
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
// Name        : QuadPatchWriteClick
// Description : Sends the current program to the QuadGT 
// Param 1     : Pointer object that generated the event
//---------------------------------------------------------------------------
void __fastcall TMainForm::QuadPatchWriteClick(TObject *Sender)
{
  UInt8 prog;
  long int status;
  UInt8 quadgt[QUAD_PATCH_SIZE];
  UInt8 sysex[200];
  UInt32 sysex_size;

  prog = (UInt8) StrToInt(QuadPatchNum->Text);

  // Convert internal format to QuadGT format
  QuadGT_Convert_Data_From_Internal(prog, quadgt);

  // Convert QuadGT format to Sysex
  sysex_size=QuadGT_Encode_To_Sysex(quadgt, QUAD_PATCH_SIZE, sysex, sizeof(sysex));

  save_patch(sysex, sysex_size, "patch_out.syx");
}
//---------------------------------------------------------------------------
UInt16 QuadGT_Decode_16Bit(UInt8 *data)
{
  //FormDebug->Log(NULL, "Decode "+AnsiString(*data)+", "+AnsiString(*(data+1)));
  return( (UInt16) *(data + 1)  + ((UInt16) *(data)<<8));
}

void QuadGT_Encode_16Bit(const UInt16 word, UInt8 *data)
{
  *data= (UInt8)(word>>8); 
  *(data+1)=(UInt8)(word & 0xff);
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
UInt32 QuadGT_Sysex_Process(tBuffer sysex)
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
       if (memcmp(Sysex_Alesis, sysex.buffer+offset, sizeof(Sysex_Alesis))==0)
       {
         offset+=sizeof(Sysex_Alesis);
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
             if (prog < QUAD_NUM_PATCH)
             {
	       FormDebug->Log(NULL, "RX: Sysex quad patch, bytes: "+AnsiString(sysex.length-offset));
               QuadGT_Decode_From_Sysex(sysex.buffer+offset,sysex.length-offset-1, quadgt, QUAD_PATCH_SIZE);
               QuadGT_Convert_QuadGT_To_Internal(prog, quadgt);
               QuadGT_Display_Update_Patch(prog);
             }
             else if (prog > QUAD_NUM_PATCH)
             {
	       FormDebug->Log(NULL, "RX: Sysex bank, bytes: "+AnsiString(sysex.length-offset));
               for (prog = 0; prog < QUAD_NUM_PATCH; prog++)
               {
                 QuadGT_Decode_From_Sysex(sysex.buffer+offset,147, quadgt, QUAD_PATCH_SIZE);
                 QuadGT_Convert_QuadGT_To_Internal(prog, quadgt);

                 offset+=147;
               }
               QuadGT_Display_Update_Patch(0);
             }
             else
             {
	       FormDebug->Log(NULL, "Data Dump Program: "+AnsiString(prog)+"   Bytes: "+AnsiString(sysex.length-offset));
             }
           }
           // TBD: Handle other program types
	 }
         else FormDebug->Log(NULL, "Not for Alesis QuadGT\n");
       }
       else FormDebug->Log(NULL, "Not for Alesis\n");
     }
     else FormDebug->Log(NULL, "No Sysex start\n");
   }
}

void __fastcall TMainForm::TESTClick(TObject *Sender)
{
  UInt8 prog;
  FILE *quadgt_file;
  tBuffer data;
  UInt32 size;

  if (!MainForm->SysexOpenDialog->Execute()) return;

  quadgt_file = fopen(MainForm->SysexOpenDialog->Files->Strings[0].c_str(),"rb");
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

  QuadGT_Display_Update_Patch(0);
}
//---------------------------------------------------------------------------
