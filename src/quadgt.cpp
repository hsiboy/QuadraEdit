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

static tQuadGT_Patch QuadGT_Patch[QUAD_NUM_PATCH];

// Decode Midi SysEx data in 7bits to Quadraverb data in 8bits
UInt32 QuadGT_Decode_From_Sysex(UInt8 *in, UInt32 length, UInt8* out, UInt32 out_len)
{
  UInt8 oc;
  UInt32 i,j;
  UInt8 shift;

  // TBD: Safety checks on out_len
  // TBD: Ensure output is correct length
  FormDebug->LogHex(NULL,AnsiString(length)+" SYSEX",in,length);
  j=0;
  oc = 0;
  for (i=0; i<length; i++) {
     if (shift = i % 8) {
        oc = (oc << shift) + (in[i] >> (7-shift));
        out[j++]=oc;
     }
     oc = in[i];
  }
  FormDebug->LogHex(NULL,AnsiString(j)+" QUAD",out,j);
  
  return(j);
}

// Encode 8bit quadraverb data into low 7 bits for SysEx
UInt32 QuadGT_Encode_To_Sysex(UInt8 *in, UInt32 length, UInt8 * out, UInt32 out_len)
{
  UInt8 lc,cc;
  UInt32 i,j;
  UInt8 shift;

  // TBD: Safety checks on out_len
  FormDebug->LogHex(NULL,AnsiString(length)+" QUAD",in,length);
  i=0;
  j=0;
  lc=0;
  cc=in[0];
  while (i<length)
  {
    shift = j % 8;

    if (shift == 0)
    {
      out[j]= cc>>1;
      lc=cc;
      cc=in[++i];
    }
    else if (shift == 7)
    {
      out[j]= cc & 0x7F;
    }
    else
    {
      out[j]= (cc >> shift+1) + (lc<< 7-shift);
      lc=cc;
      cc=in[++i];
    }
    j++;
  }
  FormDebug->LogHex(NULL,AnsiString(j)+" SYSEX",out,j);
  return(j);
}


void QuadGT_Init(void)
{
  UInt8 patch;
  char patch_name[NAME_LENGTH+1];

  for (patch=0; patch<QUAD_NUM_PATCH; patch++)
  {
    memset(&QuadGT_Patch[patch], 0x00, sizeof(tQuadGT_Patch));
    sprintf(patch_name,"Patch-%2.2d",patch);
    strcpy(QuadGT_Patch[patch].name,patch_name);
  }

}

void QuadGT_Display_Update_Patch(UInt8 prog)
{
  UInt8 config;

  /* Program Number */
  MainForm->QuadPatchNum->Text=AnsiString(prog);

  /* Program name */
  MainForm->EditQuadPatchName->Text=AnsiString(QuadGT_Patch[prog].name);

  /* Configuration */
  MainForm->QuadConfig->ItemIndex=QuadGT_Patch[prog].config;

  QuadGT_Display_Update_Reverb(prog);
  QuadGT_Display_Update_Delay(prog);
  QuadGT_Display_Update_Pitch(prog);
  QuadGT_Display_Update_Eq(prog);
  QuadGT_Display_Update_Mix(prog);
  QuadGT_Display_Update_Mod(prog);
  QuadGT_Display_Update_Preamp(prog);
  QuadGT_Display_Update_Resonator(prog);
}

void QuadGT_Display_Update_Reverb(const UInt8 prog)
{
  UInt8 val;

  if (QuadGT_Patch[prog].config==0)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=true;
    MainForm->ReverbInMix->Visible=true;
  }
  else if ((QuadGT_Patch[prog].config==1) || (QuadGT_Patch[prog].config==5) || (QuadGT_Patch[prog].config==6))
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=true;
    MainForm->ReverbInMix->Visible=true;
    // TBD: Disable input 1, selection 3
  }
  else if ((QuadGT_Patch[prog].config==2) || (QuadGT_Patch[prog].config==7))
  {
    MainForm->QuadReverb->Visible=false;
  }
  else if (QuadGT_Patch[prog].config==3)
  {
    MainForm->QuadReverb->Visible=false;
  }
  else if (QuadGT_Patch[prog].config==4)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=FALSE;
    MainForm->ReverbInMix->Visible=FALSE;
  }

  MainForm->ReverbType->ItemIndex = QuadGT_Patch[prog].reverb_mode;

}

void QuadGT_Display_Update_Delay(const UInt8 prog)
{
  UInt8 mode;

  mode=QuadGT_Patch[prog].delay_mode;
  if (mode ==0)
  {
    MainForm->DelayTap->Visible=FALSE;
    MainForm->DelayRight->Visible=FALSE;
    MainForm->DelayRightFeedback->Visible=FALSE;
  }
  else if (mode ==1)
  {
    MainForm->DelayTap->Visible=FALSE;
    MainForm->DelayRight->Visible=TRUE;
    MainForm->DelayRightFeedback->Visible=TRUE;
  }
  else if (mode ==2)
  {
    MainForm->DelayTap->Visible=FALSE;
    MainForm->DelayRight->Visible=TRUE;
    MainForm->DelayRightFeedback->Visible=TRUE;
  }
  else if (mode ==3)
  {
    MainForm->DelayTap->Visible=TRUE;
  }
}

void QuadGT_Display_Update_Pitch(const UInt8 prog)
{
  if ((QuadGT_Patch[prog].config==0) || (QuadGT_Patch[prog].config==3))
  {
    MainForm->QuadPitch->Visible=TRUE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;
  }
  else if (QuadGT_Patch[prog].config==1)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=TRUE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;
  }
  else if ((QuadGT_Patch[prog].config==2) || (QuadGT_Patch[prog].config==6) || (QuadGT_Patch[prog].config==7))
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;
  }
  else if (QuadGT_Patch[prog].config==4)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=TRUE;
  }
  else if (QuadGT_Patch[prog].config==5)
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
  if ((QuadGT_Patch[prog].config==0) || (QuadGT_Patch[prog].config==4))
  {
    MainForm->PanelQuadEq3->Visible=TRUE;

    MainForm->EqFreq1->Position = QuadGT_Patch[prog].low_eq_freq;
    MainForm->EqAmp1->Position = QuadGT_Patch[prog].low_eq_amp;

    MainForm->EqFreq2->Position = QuadGT_Patch[prog].mid_eq_freq;
    MainForm->EqAmp2->Position = QuadGT_Patch[prog].mid_eq_amp;
    MainForm->EqQ2->Position = QuadGT_Patch[prog].mid_eq_q;

    MainForm->EqFreq3->Position = QuadGT_Patch[prog].high_eq_freq;
    MainForm->EqAmp3->Position = QuadGT_Patch[prog].high_eq_amp;
  }
  else
  {
    MainForm->PanelQuadEq3->Visible=FALSE;
  }

  // 5 Band Eq
  if (QuadGT_Patch[prog].config==3) 
  {
    MainForm->QuadEq5->Visible=TRUE;
  }
  else 
  {
    MainForm->QuadEq5->Visible=FALSE;
  }

  // Graphic Eq
  if (QuadGT_Patch[prog].config==2) 
  {
    MainForm->QuadGraphEq->Visible=TRUE;
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

  // Pre or Post Eq
  val=QuadGT_Patch[prog].prepost_eq & BIT0;
  if (val == 0) 
  {
    MainForm->PrePostEq->ItemIndex=0;
  }
  else
  {
    MainForm->PrePostEq->ItemIndex=1;
  }

  // Direct level
  val=QuadGT_Patch[prog].direct_level;
  MainForm->MixDirect->Position=99-val;

  // Master Fx Level
  if (QuadGT_Patch[prog].config != 7)
  {
    val=QuadGT_Patch[prog].master_effects_level;
    MainForm->MixMaster->Position=99-val;
    MainForm->MixMaster->Visible=TRUE;
  }
  else
  {
    MainForm->MixMaster->Visible=FALSE;
  }

  // Preamp Level
  if (MainForm->PrePostEq->ItemIndex==0)
  {
    val=QuadGT_Patch[prog].preamp_level;
    MainForm->MixPreampEq->Position=99-val;
    MainForm->MixPreampEq->Visible=TRUE;
    MainForm->PreAmp->Visible=TRUE;
  }
  else 
  {
    MainForm->MixPreampEq->Visible=FALSE;
    MainForm->PreAmp->Visible=FALSE;
  }

  // Pitch Level
  val=QuadGT_Patch[prog].pitch_level;
  MainForm->MixPitch->Position=99-val;

  // Delay Level
  val=QuadGT_Patch[prog].delay_level;
  MainForm->MixDelay->Position=99-val;

  // Reverb Level
  val=QuadGT_Patch[prog].reverb_level;
  MainForm->MixReverb->Position=99-val;

  // Modulation (on/off)
  val=QuadGT_Patch[prog].mix_mod;
  if (val == 0) MainForm->Modulation->Checked=FALSE;
  else MainForm->Modulation->Checked=TRUE;

  // Mod Depth
  // Mod Speed
 
  // Leslie Level

  // Eq Level
  if (MainForm->PrePostEq->ItemIndex==1)
  {
    val=QuadGT_Patch[prog].eq_level;
    MainForm->MixPreampEq->Position=99-val;
  }

  // Ring Level
  
  // Resonator Level
}

void QuadGT_Display_Update_Mod(const UInt8 prog)
{
  // Modulation
  switch (QuadGT_Patch[prog].config)
  {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      MainForm->QuadModSource1->ItemIndex=QuadGT_Patch[prog].mod_source[0];
      MainForm->QuadModTarget1->ItemIndex=QuadGT_Patch[prog].mod_target[0];
      MainForm->QuadModAmp1->Position=QuadGT_Patch[prog].mod_amp[0]-99;
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
  MainForm->PreComp->Position=MainForm->PreComp->Max-QuadGT_Patch[prog].comp;
  MainForm->BarChange(MainForm->PreComp);
  FormDebug->Log(NULL,"  Compression: "+AnsiString(QuadGT_Patch[prog].comp));

  // Overdrive (0-7)
  MainForm->PreOd->Position=MainForm->PreOd->Max-QuadGT_Patch[prog].od;
  FormDebug->Log(NULL,"  OD: "+AnsiString(QuadGT_Patch[prog].od));
  MainForm->BarChange(MainForm->PreOd);

  // Distortion (0-8)
  MainForm->PreDist->Position=MainForm->PreDist->Max-QuadGT_Patch[prog].dist;
  FormDebug->Log(NULL,"  Dist: "+AnsiString(QuadGT_Patch[prog].dist));
  MainForm->BarChange(MainForm->PreDist);

  // Tone (0-2)
  val=QuadGT_Patch[prog].preamp_tone;
  MainForm->PreTone->ItemIndex=val;

  // Bass Boost (0-1)
  MainForm->BassBoost->Checked = QuadGT_Patch[prog].bass_boost == 1 ? TRUE : FALSE;

  // Cab sim (0-2)
  MainForm->CabSim->ItemIndex=QuadGT_Patch[prog].cab_sim;
  
  // Effect Loop (0-1)
  MainForm->EffectLoopIn->Checked = QuadGT_Patch[prog].effect_loop == 1 ? TRUE : FALSE;

  // Noise Gate (0-17)
  val=(QuadGT_Patch[prog].preamp_gate & BITS0to4);
  MainForm->PreGate->Position=17-val;
  FormDebug->Log(NULL,"Gate: "+AnsiString(val));

  /* Preamp Output Level (0-99) */
  val=QuadGT_Patch[prog].preamp_out_level;
  MainForm->PreOutLevel->Position = 99-val;
  FormDebug->Log(NULL, "Preamp output level :"+AnsiString(val));

}

void QuadtGT_Param_Change(TObject * Sender)
{
  UInt8 prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  FormDebug->Log(NULL,"Change: "+AnsiString(prog));

  if (Sender == MainForm->QuadConfig)
  {
    QuadGT_Patch[prog].config=MainForm->QuadConfig->ItemIndex;
    QuadGT_Display_Update_Patch(prog);
  }
  if (Sender == MainForm->PreComp)
  {
    QuadGT_Patch[prog].comp= (MainForm->PreComp->Max - MainForm->PreComp->Position);
    MainForm->BarChange(Sender);
    FormDebug->Log(NULL,"  Set Compression: "+AnsiString(QuadGT_Patch[prog].comp));
  }
  if (Sender == MainForm->PreOd)
  {
    QuadGT_Patch[prog].od=(MainForm->PreOd->Max - MainForm->PreOd->Position);
    MainForm->BarChange(Sender);
    FormDebug->Log(NULL,"  Set OD: "+AnsiString(QuadGT_Patch[prog].od));
  }
  if (Sender == MainForm->PreDist)
  {
    QuadGT_Patch[prog].dist= (MainForm->PreDist->Max - MainForm->PreDist->Position);
    MainForm->BarChange(Sender);
    FormDebug->Log(NULL,"  Set DIST: "+AnsiString(QuadGT_Patch[prog].dist));
  }
  if (Sender == MainForm->EditQuadPatchName)
  {
    strncpy(QuadGT_Patch[prog].name,MainForm->EditQuadPatchName->Text.c_str(),NAME_LENGTH);
  }
}

void __fastcall TMainForm::DelayModeClick(TObject *Sender)
{
  UInt8 prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  QuadGT_Patch[prog].delay_mode=MainForm->DelayMode->ItemIndex;
  QuadGT_Display_Update_Delay(prog);
}
//---------------------------------------------------------------------------

void QuadGT_Display_Update_Resonator(const UInt8 prog)
{
  if (QuadGT_Patch[prog].config==CFG0_EQ_PITCH_DELAY_REVERB)
  {
    MainForm->QuadResonator->Visible=TRUE;
  }
  if (QuadGT_Patch[prog].config==CFG4_3BANDEQ_REVERB)
  {
    MainForm->QuadResonator->Visible=TRUE;
  }
  // Config 3 and Eq-Mode is 3Band+Resonator
  else if ((QuadGT_Patch[prog].config == CFG3_5BANDEQ_PITCH_DELAY) && (QuadGT_Patch[prog].eq_mode == 1))
  {
    MainForm->QuadResonator->Visible=TRUE;
  }
  else if (QuadGT_Patch[prog].config==CFG6_RESONATOR_DELAY_REVERB)
  {
    MainForm->QuadResonator->Visible=TRUE;
  }
  else
  {
    MainForm->QuadResonator->Visible=FALSE;
  }
}

//---------------------------------------------------------------------------
// Name     : QuadGT_Convert_Data_To_Internal
// Description : Convert raw Quadraverb GT data to internal data structure
//---------------------------------------------------------------------------
UInt32 QuadGT_Convert_Data_To_Internal(UInt8 prog, UInt8* data)
{
  if (prog >= QUAD_NUM_PATCH) return 1;

  QuadGT_Patch[prog].config= data[CONFIG_IDX];
  memcpy(QuadGT_Patch[prog].name, &data[NAME_IDX], NAME_LENGTH);
  QuadGT_Patch[prog].name[NAME_LENGTH]=0;


  //-------------------------------------------------------------------------
  // Preamp Parameters
  //-------------------------------------------------------------------------
  QuadGT_Patch[prog].comp             =  (data[PREAMP_COMP_IDX] & BITS4to6) >> 4;
  QuadGT_Patch[prog].od               =  (data[PREAMP_OD_IDX]   & BITS5to7) >> 5;
  QuadGT_Patch[prog].dist             =  (data[PREAMP_DIST_IDX] & BITS0to3);
  QuadGT_Patch[prog].preamp_tone      = (data[PREAMP_TONE_IDX] & BITS2to3) >> 2;
  QuadGT_Patch[prog].bass_boost       = data[BASS_BOOST_IDX];
  QuadGT_Patch[prog].cab_sim          = data[CAB_SIM_IDX];
  QuadGT_Patch[prog].effect_loop      = data[EFFECT_LOOP_IDX];
  QuadGT_Patch[prog].preamp_gate      = (data[PREAMP_GATE_IDX] & BITS0to4);
  QuadGT_Patch[prog].preamp_out_level = data[PREAMP_OUT_LEVEL_IDX];

  //-------------------------------------------------------------------------
  // Eq Parameters
  //-------------------------------------------------------------------------
  QuadGT_Patch[prog].eq_mode=((data[EQ_MODE_IDX]&BIT7)>>7);                    // TBD: Is this preset/user?

  // 3 and 5 Band Eq
  if ( (QuadGT_Patch[prog].config==0) || (QuadGT_Patch[prog].config==4) ||
       (QuadGT_Patch[prog].config==3))
  {
    QuadGT_Patch[prog].low_eq_freq   = EXTRACT_16BIT(data[LOW_EQ_FREQ_IDX]);
    QuadGT_Patch[prog].low_eq_amp    = EXTRACT_16BIT(data[LOW_EQ_AMP_IDX]);
    QuadGT_Patch[prog].mid_eq_freq   = EXTRACT_16BIT(data[MID_EQ_FREQ_IDX]);
    QuadGT_Patch[prog].mid_eq_amp    = EXTRACT_16BIT(data[MID_EQ_AMP_IDX]);
    QuadGT_Patch[prog].mid_eq_q      = data[MID_EQ_BW_IDX];
    QuadGT_Patch[prog].high_eq_freq  = EXTRACT_16BIT(data[HIGH_EQ_FREQ_IDX]);
    QuadGT_Patch[prog].high_eq_amp   = EXTRACT_16BIT(data[HIGH_EQ_AMP_IDX]);
  }

  // 5 Band Eq only
  if (QuadGT_Patch[prog].config==3)
  {
    // TBD
    QuadGT_Patch[prog].low_mid_eq_freq   = EXTRACT_16BIT(data[LOW_MID_EQ_FREQ_IDX]);
    QuadGT_Patch[prog].low_mid_eq_amp    = EXTRACT_16BIT(data[LOW_MID_EQ_AMP_IDX]);
    QuadGT_Patch[prog].low_mid_eq_q      = data[LOW_MID_EQ_BW_IDX];

    QuadGT_Patch[prog].high_mid_eq_freq   = EXTRACT_16BIT(data[HIGH_MID_EQ_FREQ_IDX]);
    QuadGT_Patch[prog].high_mid_eq_amp    = EXTRACT_16BIT(data[HIGH_MID_EQ_AMP_IDX]);
    QuadGT_Patch[prog].high_mid_eq_q      = data[HIGH_MID_EQ_BW_IDX];
  }

  // Graphic Eq
  if (QuadGT_Patch[prog].config==2)
  {
    QuadGT_Patch[prog].geq_16hz   = data[GEQ_16HZ_IDX];
    QuadGT_Patch[prog].geq_32hz   = data[GEQ_32HZ_IDX];
    QuadGT_Patch[prog].geq_62hz   = data[GEQ_62HZ_IDX];
    QuadGT_Patch[prog].geq_126hz  = data[GEQ_126HZ_IDX];
    QuadGT_Patch[prog].geq_250hz  = data[GEQ_250HZ_IDX];
    QuadGT_Patch[prog].geq_1khz   = data[GEQ_1KHZ_IDX];
    QuadGT_Patch[prog].geq_2khz   = data[GEQ_2KHZ_IDX];
    QuadGT_Patch[prog].geq_4khz   = data[GEQ_4KHZ_IDX];
    QuadGT_Patch[prog].geq_8khz   = data[GEQ_8KHZ_IDX];
    QuadGT_Patch[prog].geq_16khz  = data[GEQ_16KHZ_IDX];
  }


  //-------------------------------------------------------------------------
  // Reverb Parameters
  //-------------------------------------------------------------------------
  QuadGT_Patch[prog].reverb_mode = data[REVERB_MODE_IDX];

  //-------------------------------------------------------------------------
  // Delay Parameters
  //-------------------------------------------------------------------------
  QuadGT_Patch[prog].delay_mode=data[DELAY_MODE_IDX];
  QuadGT_Patch[prog].delay=data[DELAY_IDX];
  QuadGT_Patch[prog].delay_feedback=data[DELAY_FEEDBACK_IDX];
  QuadGT_Patch[prog].delay_left=data[DELAY_LEFT_IDX];
  QuadGT_Patch[prog].delay_left_feedback=data[DELAY_LEFT_FEEDBACK_IDX];
  QuadGT_Patch[prog].delay_right=data[DELAY_RIGHT_IDX];
  QuadGT_Patch[prog].delay_right_feedback=data[DELAY_RIGHT_FEEDBACK_IDX];

  //-------------------------------------------------------------------------
  // Mix Parameters
  //-------------------------------------------------------------------------
  QuadGT_Patch[prog].prepost_eq=data[PREPOST_EQ_IDX] & BIT0;
  QuadGT_Patch[prog].direct_level=(data[DIRECT_LEVEL_IDX ] & BITS1to7) >> 1;
  QuadGT_Patch[prog].master_effects_level=data[MASTER_EFFECTS_LEVEL_IDX ];

  if (QuadGT_Patch[prog].prepost_eq == 0) QuadGT_Patch[prog].preamp_level=data[PREAMP_LEVEL_IDX];
  else                                    QuadGT_Patch[prog].eq_level=data[EQ_LEVEL_IDX];

  if (QuadGT_Patch[prog].config==1)
  {
    QuadGT_Patch[prog].leslie_level=data[LESLIE_LEVEL_IDX];
  }
  else if (QuadGT_Patch[prog].config==5)
  {
    QuadGT_Patch[prog].ring_mod_level=data[RING_MOD_LEVEL_IDX];
  }
  else
  {
    QuadGT_Patch[prog].pitch_level=data[PITCH_LEVEL_IDX];
  }

  QuadGT_Patch[prog].delay_level=data[DELAY_LEVEL_IDX ];
  QuadGT_Patch[prog].reverb_level=data[REVERB_LEVEL_IDX ];
  QuadGT_Patch[prog].mix_mod=(data[MIX_MOD_IDX] & BITS6to7) >> 6;
  // TBD: MOD_DEPTH
  // TBD: MOD_SPEED

  //-------------------------------------------------------------------------
  // Modulation Parameters
  //-------------------------------------------------------------------------
  QuadGT_Patch[prog].mod_source[0]=data[MOD1_SOURCE_IDX];
  QuadGT_Patch[prog].mod_target[0]=data[MOD1_TARGET_IDX];
  QuadGT_Patch[prog].mod_amp[0]=data[MOD1_AMP_IDX];

  //-------------------------------------------------------------------------
  // Resonator Parameters
  //-------------------------------------------------------------------------
  //TBD: Determine which set of parameters are used in which mode

  return 0;
}

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
    if (fwrite(&QuadGT_Patch[prog], sizeof(tQuadGT_Patch), 1, bank_file) != 1)
    {
      FormError->ShowError(ferror(bank_file),"writing bank file ");
      break;
    }
  }

  fclose(bank_file);

}

void __fastcall TMainForm::QuadBankLoadClick(TObject *Sender)
{
  UInt8 prog;
  FILE *bank_file;

  bank_file = fopen("quadgt.bnk","rb");
  if (bank_file == NULL)
  {
    FormError->ShowError(ferror(bank_file),"opening bank file");
    return;
  }
  for(prog=0; prog<QUAD_NUM_PATCH; prog++)
  {
    if (fread(&QuadGT_Patch[prog], sizeof(tQuadGT_Patch), 1, bank_file) != 1)
    {
      FormError->ShowError(ferror(bank_file),"reading bank file ");
      break;
    }
  }

  fclose(bank_file);

  prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);
  QuadGT_Display_Update_Patch(prog);
}
//---------------------------------------------------------------------------

