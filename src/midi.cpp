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

#define MIDI_IN_BUF_SIZE  (20480)  // Make this big so we don't have to deal with
                                   // Sysex messages crossing multiple buffers
#define MIDI_IN_NUM_BUF    (1)     // TBD: Get rid of this entirely

//---------------------------------------------------------------------------
#pragma package(smart_init)

static boolean Midi_Open=FALSE;
static tQuadGT_Patch QuadGT_Patch[QUAD_NUM_PATCH];

typedef struct tQueue_Entry
{
  tBuffer payload;
  tQueue_Entry *next_ptr;
} tQueue_Entry;

typedef struct tQueue
{
  tQueue_Entry *ptr;
} tQueue;


const UInt8 Sysex_Start[]={0xF0};              // SysEx start 

const UInt8 Sysex_End[]={0xF7};                // SysEx End

// Midi handles
static HMIDIOUT Midi_Out_Handle;
static HMIDIIN Midi_In_Handle;

// Headers and associated buffers for Midi Input SYSEX messages
typedef struct tMidi_In
{
  MIDIHDR Hdr;
  UInt8   Buffer[MIDI_IN_BUF_SIZE];
} tMidi_In;

static tMidi_In Midi_In[MIDI_IN_NUM_BUF];

static tQueue Rx_Msg_Queue;

// Rx message counters
static int rx_sysex=0;
static int rx_msg=0;
static int rx_other=0;

void Midi_Init(void)
{
  Rx_Msg_Queue.ptr=NULL;

  FormDebug->Log(NULL, "MIDI Init done");
}

void Queue_Push(tBuffer buffer)
{
  tQueue_Entry *entry;
  tQueue_Entry *tail;

  // Create new queue entry
  entry = (tQueue_Entry *)malloc(sizeof(tQueue_Entry));
  if (entry == NULL)
  {
    FormError->ShowError(1,"no memory left to create queue entry");
    return;
  }

  // Create space for payload buffer
  entry->payload.buffer = (UInt8 *) malloc(buffer.length);
  if (entry->payload.buffer == NULL)
  {
    FormError->ShowError(2,"no memory left to queue data");
    free(entry);
    return;
  }

  // Copy buffer into queue entry payload
  memcpy(entry->payload.buffer, buffer.buffer, buffer.length);
  entry->payload.length=buffer.length;

  entry->next_ptr=NULL;

  // Push entry onto tail of queue
  if (Rx_Msg_Queue.ptr == NULL)
  {
    Rx_Msg_Queue.ptr = entry;
  }
  else
  {
    tail = Rx_Msg_Queue.ptr;
    while (tail->next_ptr != NULL) tail=tail->next_ptr;
    tail->next_ptr = entry;
  }

}

// Pop entry from head of queue
tBuffer Queue_Pop(void)
{
  tQueue_Entry * head;
  tBuffer payload;

  // Is queue empty?
  if (Rx_Msg_Queue.ptr == NULL) 
  {
    payload.buffer = NULL;
    payload.length = 0;
  }
  else
  {
    head = Rx_Msg_Queue.ptr;
    payload = head->payload;

    Rx_Msg_Queue.ptr = head->next_ptr;

    // Free the queue entry
    free(head);

  }

  // Return the payload, the caller must free payload.buffer when done
  return(payload);
}


void Midi_Get_Dev_Lists(TComboBox *in_list, TComboBox *out_list, TLabel * error_text)
{
  int devs,i;
  MMRESULT status;
  MIDIOUTCAPS caps;
  MIDIINCAPS incaps;

  error_text->Caption="";
  
  /**************************************************************************
  * Build the list of possible MIDI output devices
  **************************************************************************/
  devs = midiOutGetNumDevs();
  if (devs < 1) {
    error_text->Caption="No MIDI Output devices available";
    // TBD: Handle error
  }
  else {
    for (i=0; i<devs; i++)
    {
      status = midiOutGetDevCaps(i, &caps, sizeof(caps));
      if (status == MMSYSERR_NOERROR)
      {
        out_list->Items->Add(caps.szPname);
      }
      else
      {
        out_list->Items->Add("MIDI Out "+AnsiString(i+1)+" - ERROR");
      }
    }
  }

  out_list->ItemIndex=0;

  /**************************************************************************
  * Build the list of possible MIDI input devices
  **************************************************************************/
  devs = midiInGetNumDevs();
  if (devs < 1) {
        error_text->Caption="No MIDI Input devices available";
  }
  else {
    for (i=0; i<devs; i++)
    {
      status = midiInGetDevCaps(i, &incaps, sizeof(incaps));
      if (status == MMSYSERR_NOERROR)
      {
        in_list->Items->Add(incaps.szPname);
      }
      else
      {
        in_list->Items->Add("MIDI In "+AnsiString(i+1)+" - ERROR");
      }
    }
  }

  in_list->ItemIndex=0;
}

UInt8 Midi_Out_Open(int device)
{
  MMRESULT status;
  status=midiOutOpen(&Midi_Out_Handle, device, 0, 0, CALLBACK_NULL);

  if (status == MMSYSERR_NOERROR)
  {
    return 0;
  }
  else
  {
    // TBD: report open error?
    return 1;
  }

}

unsigned int Midi_Out_Dump_Req(UInt8 program)
{
  MIDIHDR out;
  UInt8 buffer[100];
  UInt8 buf_len=0;
  long int status;

  // Build message in buffer
  memcpy(buffer+buf_len, Sysex_Start, sizeof(Sysex_Start));
  buf_len+=sizeof(Sysex_Start);

  memcpy(buffer+buf_len, Sysex_Alesis, sizeof(Sysex_Alesis));
  buf_len+=sizeof(Sysex_Alesis);

  memcpy(buffer+buf_len, Sysex_QuadGT, sizeof(Sysex_QuadGT));
  buf_len+=sizeof(Sysex_QuadGT);

  memcpy(buffer+buf_len, Sysex_Dump_Req, sizeof(Sysex_Dump_Req));
  buf_len+=sizeof(Sysex_Dump_Req);

  buffer[buf_len] = program;
  buf_len+=1;

  memcpy(buffer+buf_len, Sysex_End, sizeof(Sysex_End));
  buf_len+=sizeof(Sysex_End);

  FormDebug->LogHex(NULL, "TX", buffer, buf_len);

  // Prepare header
  out.lpData = buffer;
  out.dwBufferLength = buf_len;
  out.dwBytesRecorded = buf_len;
  out.dwFlags = 0;

  // Prepare and send the MIDI message
  midiOutPrepareHeader(Midi_Out_Handle, &out, sizeof(out));
  status=midiOutLongMsg(Midi_Out_Handle, &out, sizeof(out));
  midiOutUnprepareHeader(Midi_Out_Handle, &out, sizeof(out));

  return(status);

}

unsigned int Midi_Out_Edit(UInt8 function, UInt8 page, UInt16 data)
{
  MIDIHDR out;
  UInt8 buffer[100];
  UInt8 buf_len=0;
  long int status;

  // Build message in buffer
  memcpy(buffer+buf_len, Sysex_Start, sizeof(Sysex_Start));
  buf_len+=sizeof(Sysex_Start);

  memcpy(buffer+buf_len, Sysex_Alesis, sizeof(Sysex_Alesis));
  buf_len+=sizeof(Sysex_Alesis);

  memcpy(buffer+buf_len, Sysex_QuadGT, sizeof(Sysex_QuadGT));
  buf_len+=sizeof(Sysex_QuadGT);

  memcpy(buffer+buf_len, Sysex_Edit, sizeof(Sysex_Edit));
  buf_len+=sizeof(Sysex_Edit);

  buffer[buf_len] = function;
  buf_len+=1;

  buffer[buf_len] = page;
  buf_len+=1;

  QuadGT_Encode_To_Sysex((UInt8*)&data,2,&buffer[buf_len],3);
  buf_len+=3;

  memcpy(buffer+buf_len, Sysex_End, sizeof(Sysex_End));
  buf_len+=sizeof(Sysex_End);

  FormDebug->LogHex(NULL, "TX", buffer, buf_len);

  // Prepare header
  out.lpData = buffer;
  out.dwBufferLength = buf_len;
  out.dwBytesRecorded = buf_len;
  out.dwFlags = 0;

  // Prepare and send the MIDI message
  midiOutPrepareHeader(Midi_Out_Handle, &out, sizeof(out));
  status=midiOutLongMsg(Midi_Out_Handle, &out, sizeof(out));
  midiOutUnprepareHeader(Midi_Out_Handle, &out, sizeof(out));

  return(status);

}
void Midi_Out_ShortMsg(const unsigned long message)
{
  midiOutShortMsg(Midi_Out_Handle, message);
}
//---------------------------------------------------------------------------
// Name        : Midi_In_Callback
// Description : Call back routine to process data received from Midi In.
//---------------------------------------------------------------------------
static void CALLBACK Midi_In_Callback(HMIDIIN handle, UINT msg,
                                  DWORD *instance, DWORD *p1, DWORD *p2)
{
  MIDIHDR *header_ptr;
  UInt32 status;
  tBuffer entry;

  switch (msg)
  {
    case MIM_LONGDATA:
      // Handle SYSEX data input
      header_ptr = (MIDIHDR *)p1;
      rx_sysex++;

      if (Midi_Open == TRUE)
      {
        entry.buffer=header_ptr->lpData;
        entry.length=header_ptr->dwBytesRecorded;
        Queue_Push(entry);

      }
      // Make the buffer free for next lot of SysEx data
      midiInUnprepareHeader(Midi_In_Handle, header_ptr, sizeof(MIDIHDR));

      // TBD: use proper index on Midi_In or only use a single buffer
      header_ptr->lpData = Midi_In[0].Buffer;
      header_ptr->dwBufferLength = sizeof(Midi_In[0].Buffer);
      header_ptr->dwBytesRecorded = 0;
      header_ptr->dwFlags = 0;
      status = midiInPrepareHeader(Midi_In_Handle, header_ptr, sizeof(MIDIHDR));
      if (status != 0) FormDebug->Log(NULL,"Proc prepare error");
      status=midiInAddBuffer(Midi_In_Handle, header_ptr, sizeof(MIDIHDR));
      if (status != 0) FormDebug->Log(NULL,"Proc buffer error");

      break;

    case MIM_DATA:
      rx_msg++;
      break;

    case MIM_ERROR:
    case MIM_CLOSE:
    default:
      rx_other++;
      break;
  }


}

unsigned int Midi_In_Open(int device)
{
  UInt8 i;
  MMRESULT status;

  status=midiInOpen(&Midi_In_Handle, device, (DWORD) Midi_In_Callback, 0, CALLBACK_FUNCTION);
  if (status != MMSYSERR_NOERROR)
  {
    return status;
  }

  // Setup MIDI in header
  for (i=0; i<MIDI_IN_NUM_BUF; i++)
  {
    Midi_In[i].Hdr.lpData = Midi_In[0].Buffer;
    Midi_In[i].Hdr.dwBufferLength = sizeof(Midi_In[0].Buffer);
    Midi_In[i].Hdr.dwBytesRecorded = 0;
    Midi_In[i].Hdr.dwFlags = 0;
    status = midiInPrepareHeader(Midi_In_Handle, &Midi_In[i].Hdr, sizeof(Midi_In[i].Hdr));
    if (status != MMSYSERR_NOERROR)
    {
      return status;
    }
  }

  // Add a buffer to the MIDI in header
  status=midiInAddBuffer(Midi_In_Handle, &Midi_In[0].Hdr, sizeof(Midi_In[0].Hdr));
  if (status != MMSYSERR_NOERROR)
  {
    // TBD: Report buffer error details
    return status;
  }

  midiInStart(Midi_In_Handle);

  Midi_Open=TRUE;
  return MMSYSERR_NOERROR;
}

void Midi_Get_Counts(int *msg_count_ptr, int *sysex_count_ptr, int *other_count_ptr)
{
  *msg_count_ptr=rx_msg;
  *sysex_count_ptr=rx_sysex;
  *other_count_ptr=rx_other;
}

unsigned int Midi_In_Close(void)
{
  unsigned int status;

  if (Midi_Open == TRUE)
  {
    Midi_Open=FALSE;
    midiInUnprepareHeader(Midi_In_Handle, &Midi_In[0].Hdr, sizeof(Midi_In[0].Hdr));

    status=midiInStop(Midi_In_Handle);
    if (status != MMSYSERR_NOERROR) return(status);

    status=midiInReset(Midi_In_Handle);
    if (status != MMSYSERR_NOERROR) return(status);

    status=midiInClose(Midi_In_Handle);
    if (status != MMSYSERR_NOERROR) return(status);


  }
  return(status);
}
unsigned int Midi_Out_Close(void)
{
  unsigned int status;
  status=midiOutClose(Midi_Out_Handle);
  return(status);
}

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

//---------------------------------------------------------------------------
// Name        : Midi_Sysex_Process
// Description : Process the Sysex data received
//---------------------------------------------------------------------------
void Midi_Sysex_Process(void)
{
   tBuffer sysex;
   UInt32 offset;
   UInt8 code,prog;
   UInt8 quadgt[QUAD_PATCH_SIZE];

   offset=0;
   sysex = Queue_Pop();
   if (sysex.buffer != NULL)
   {
     if (memcmp(Sysex_Start, sysex.buffer+offset, sizeof(Sysex_Start))==0)
     {
       offset+=sizeof(Sysex_Start);
       if (memcmp(Sysex_Alesis, sysex.buffer+offset, sizeof(Sysex_Alesis))==0)
       {
         offset+=sizeof(Sysex_Alesis);
         if (memcmp(Sysex_QuadGT, sysex.buffer+offset, sizeof(Sysex_QuadGT))==0)
         {
           offset+=sizeof(Sysex_QuadGT);

	   code = *(sysex.buffer+offset);
	   offset+=1;
	   prog = *(sysex.buffer+offset);
	   offset+=1;

	   FormDebug->Log(NULL, "Code: "+AnsiString(code)+"  Program: "+AnsiString(prog)+"   Bytes: "+AnsiString(sysex.length-offset));
           QuadGT_Decode_From_Sysex(sysex.buffer+offset,sysex.length-offset, quadgt, QUAD_PATCH_SIZE);

           QuadGT_Convert_Data_To_Internal(prog, sysex.buffer+offset);

           QuadGT_Display_Update_Patch(prog);
	 }
       }
     }
     free(sysex.buffer);
   }
}

//---------------------------------------------------------------------------
// Name      : quadgt.cpp
//---------------------------------------------------------------------------

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

void QuadGT_Display_Update_Patch(UInt8 program)
{
  UInt8 config;

  /* Program Number */
  MainForm->QuadPatchNum->Text=AnsiString(program);

  /* Program name */
  MainForm->EditQuadPatchName->Text=AnsiString(QuadGT_Patch[program].name);

 
  /* Configuration */
  MainForm->QuadConfig->ItemIndex=QuadGT_Patch[program].config;

  QuadGT_Display_Update_Reverb(program);

  QuadGT_Display_Update_Delay(program);

  QuadGT_Display_Update_Pitch(program);

  QuadGT_Display_Update_Eq(program);

  QuadGT_Display_Update_Mix(program);

  QuadGT_Display_Update_Mod(program);

  QuadGT_Display_Update_Preamp(program);

  QuadGT_Display_Update_Resonator(program);
}

void QuadGT_Display_Update_Reverb(const UInt8 program)
{
  UInt8 val;

  if (QuadGT_Patch[program].config==0)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=true;
    MainForm->ReverbInMix->Visible=true;
  }
  else if ((QuadGT_Patch[program].config==1) || (QuadGT_Patch[program].config==5) || (QuadGT_Patch[program].config==6))
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=true;
    MainForm->ReverbInMix->Visible=true;
    // TBD: Disable input 1, selection 3
  }
  else if ((QuadGT_Patch[program].config==2) || (QuadGT_Patch[program].config==7))
  {
    MainForm->QuadReverb->Visible=false;
  }
  else if (QuadGT_Patch[program].config==3)
  {
    MainForm->QuadReverb->Visible=false;
  }
  else if (QuadGT_Patch[program].config==4)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=FALSE;
    MainForm->ReverbInMix->Visible=FALSE;
  }

  MainForm->ReverbType->ItemIndex = QuadGT_Patch[program].reverb_mode;

}

void QuadGT_Display_Update_Delay(const UInt8 program)
{
  UInt8 mode;

  mode=QuadGT_Patch[program].delay_mode;
  if (mode ==0)
  {
    MainForm->DelayTap->Visible=FALSE;
  }
  else if (mode ==1)
  {
    MainForm->DelayTap->Visible=FALSE;
  }
}

void QuadGT_Display_Update_Pitch(const UInt8 program)
{
  if ((QuadGT_Patch[program].config==0) || (QuadGT_Patch[program].config==3))
  {
    MainForm->QuadPitch->Visible=TRUE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;
  }
  else if (QuadGT_Patch[program].config==1)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=TRUE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;
  }
  else if ((QuadGT_Patch[program].config==2) || (QuadGT_Patch[program].config==6) || (QuadGT_Patch[program].config==7))
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=FALSE;
  }
  else if (QuadGT_Patch[program].config==4)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
    MainForm->QuadChorus->Visible=TRUE;
  }
  else if (QuadGT_Patch[program].config==5)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=TRUE;
    MainForm->QuadChorus->Visible=FALSE;
  }
}

void QuadGT_Display_Update_Eq(const UInt8 program)
{
  // 3 Band Eq
  if ((QuadGT_Patch[program].config==0) || (QuadGT_Patch[program].config==4))
  {
    MainForm->PanelQuadEq3->Visible=TRUE;

    MainForm->EqFreq1->Position = QuadGT_Patch[program].low_eq_freq;
    MainForm->EqAmp1->Position = QuadGT_Patch[program].low_eq_amp;

    MainForm->EqFreq2->Position = QuadGT_Patch[program].mid_eq_freq;
    MainForm->EqAmp2->Position = QuadGT_Patch[program].mid_eq_amp;
    MainForm->EqQ2->Position = QuadGT_Patch[program].mid_eq_q;

    MainForm->EqFreq3->Position = QuadGT_Patch[program].high_eq_freq;
    MainForm->EqAmp3->Position = QuadGT_Patch[program].high_eq_amp;
  }
  else
  {
    MainForm->PanelQuadEq3->Visible=FALSE;
  }

  // 5 Band Eq
  if (QuadGT_Patch[program].config==3) 
  {
    MainForm->QuadEq5->Visible=TRUE;
  }
  else 
  {
    MainForm->QuadEq5->Visible=FALSE;
  }

  // Graphic Eq
  if (QuadGT_Patch[program].config==2) 
  {
    MainForm->QuadGraphEq->Visible=TRUE;
  }
  else 
  {
    MainForm->QuadGraphEq->Visible=FALSE;
  }

  // Resonator
}

void QuadGT_Display_Update_Mix(const UInt8 program)
{
  UInt8 val;

  // Pre or Post Eq
  val=QuadGT_Patch[program].prepost_eq & BIT0;
  if (val == 0) 
  {
    MainForm->PrePostEq->ItemIndex=0;
  }
  else
  {
    MainForm->PrePostEq->ItemIndex=1;
  }

  // Direct level
  val=QuadGT_Patch[program].direct_level;
  MainForm->MixDirect->Position=99-val;

  // Master Fx Level
  if (QuadGT_Patch[program].config != 7)
  {
    val=QuadGT_Patch[program].master_effects_level;
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
    val=QuadGT_Patch[program].preamp_level;
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
  val=QuadGT_Patch[program].pitch_level;
  MainForm->MixPitch->Position=99-val;

  // Delay Level
  val=QuadGT_Patch[program].delay_level;
  MainForm->MixDelay->Position=99-val;

  // Reverb Level
  val=QuadGT_Patch[program].reverb_level;
  MainForm->MixReverb->Position=99-val;

  // Modulation (on/off)
  // TBD: Fix grouping so this doesnt effect pre/post eq
  val=QuadGT_Patch[program].mix_mod;
  if (val == 0) MainForm->Modulation->Checked=FALSE;
  else MainForm->Modulation->Checked=TRUE;

  // Mod Depth
  // Mod Speed
 
  // Leslie Level

  // Eq Level
  if (MainForm->PrePostEq->ItemIndex==1)
  {
    val=QuadGT_Patch[program].eq_level;
    MainForm->MixPreampEq->Position=99-val;
  }

  // Ring Level
  
  // Resonator Level
}

void QuadGT_Display_Update_Mod(const UInt8 program)
{
  // Modulation
  switch (QuadGT_Patch[program].config)
  {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      MainForm->QuadModSource1->ItemIndex=QuadGT_Patch[program].mod1_source;
      MainForm->QuadModTarget1->ItemIndex=QuadGT_Patch[program].mod1_target;
      MainForm->QuadModAmp1->Position=QuadGT_Patch[program].mod1_amp-99;
      break;

    case 7:
    default:
      break;
  }
}

void QuadGT_Display_Update_Preamp(const UInt8 program)
{
  UInt8 val;
  UInt8 prog=(UInt8)StrToInt(MainForm->QuadPatchNum->Text);

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
  val=QuadGT_Patch[program].preamp_tone;
  MainForm->PreTone->ItemIndex=val;

  // Bass Boost (0-1)
  MainForm->BassBoost->Checked = QuadGT_Patch[program].bass_boost == 1 ? TRUE : FALSE;

  // Cab sim (0-2)
  MainForm->CabSim->ItemIndex=QuadGT_Patch[program].cab_sim;
  
  // Effect Loop (0-1)
  MainForm->EffectLoopIn->Checked = QuadGT_Patch[program].effect_loop == 1 ? TRUE : FALSE;

  // Noise Gate (0-17)
  val=(QuadGT_Patch[program].preamp_gate & BITS0to4);
  MainForm->PreGate->Position=17-val;
  FormDebug->Log(NULL,"Gate: "+AnsiString(val));

  /* Preamp Output Level (0-99) */
  val=QuadGT_Patch[program].preamp_out_level;
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
}

void QuadGT_Display_Update_Resonator(const UInt8 program)
{
  // Config 3 and Eq-Mode is 3Band+Resonator
  if ((QuadGT_Patch[program].config == 3) && (QuadGT_Patch[program].eq_mode == 1))
  {
    MainForm->QuadResonator->Visible=TRUE;
  }
  else if (QuadGT_Patch[program].config==6)
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

  QuadGT_Patch[prog].comp =  (data[PREAMP_COMP_IDX] & BITS4to6) >> 4;
  QuadGT_Patch[prog].od   =  (data[PREAMP_OD_IDX]   & BITS5to7) >> 5;
  QuadGT_Patch[prog].dist =  (data[PREAMP_DIST_IDX] & BITS0to3);

  // TBD: decode 16 bit values properly
  QuadGT_Patch[prog].low_eq_freq = data[LOW_EQ_FREQ_IDX];
  QuadGT_Patch[prog].low_eq_amp  = data[LOW_EQ_AMP_IDX];
  QuadGT_Patch[prog].mid_eq_freq = data[MID_EQ_FREQ_IDX];
  QuadGT_Patch[prog].mid_eq_amp  = data[MID_EQ_AMP_IDX];
  QuadGT_Patch[prog].mid_eq_q    = data[MID_EQ_BW_IDX];
  QuadGT_Patch[prog].high_eq_freq = data[HIGH_EQ_FREQ_IDX];
  QuadGT_Patch[prog].high_eq_amp   = data[HIGH_EQ_AMP_IDX];
  QuadGT_Patch[prog].eq_mode=((data[EQ_MODE_IDX]&BIT7)>>7);

  QuadGT_Patch[prog].reverb_mode = data[REVERB_MODE_IDX];

  QuadGT_Patch[prog].delay_mode=data[DELAY_MODE_IDX];

  QuadGT_Patch[prog].prepost_eq=data[PREPOST_EQ_IDX] & BIT0;
  QuadGT_Patch[prog].direct_level=(data[DIRECT_LEVEL_IDX ] & BITS1to7) >> 1;
  QuadGT_Patch[prog].master_effects_level=data[MASTER_EFFECTS_LEVEL_IDX ];
  QuadGT_Patch[prog].preamp_level=data[PREAMP_LEVEL_IDX ];
  QuadGT_Patch[prog].pitch_level=data[PITCH_LEVEL_IDX ];
  QuadGT_Patch[prog].delay_level=data[DELAY_LEVEL_IDX ];
  QuadGT_Patch[prog].reverb_level=data[REVERB_LEVEL_IDX ];
  QuadGT_Patch[prog].mix_mod=(data[MIX_MOD_IDX] & BITS6to7) >> 6;
  QuadGT_Patch[prog].eq_level=data[EQ_LEVEL_IDX ];

  QuadGT_Patch[prog].mod1_source=data[MOD1_SOURCE_IDX];
  QuadGT_Patch[prog].mod1_target=data[MOD1_TARGET_IDX];
  QuadGT_Patch[prog].mod1_amp=data[MOD1_AMP_IDX]-99;

  QuadGT_Patch[prog].preamp_tone=(data[PREAMP_TONE_IDX] & BITS2to3) >> 2;
  QuadGT_Patch[prog].preamp_gate=(data[PREAMP_GATE_IDX] & BITS0to4);
  QuadGT_Patch[prog].preamp_out_level=data[PREAMP_OUT_LEVEL_IDX];
  QuadGT_Patch[prog].effect_loop=data[EFFECT_LOOP_IDX];
  QuadGT_Patch[prog].bass_boost=data[BASS_BOOST_IDX];
  QuadGT_Patch[prog].cab_sim=data[CAB_SIM_IDX];

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

}
//---------------------------------------------------------------------------

