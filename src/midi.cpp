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

void CALLBACK Midi_In_Proc(HMIDIIN handle, UINT msg,
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

  status=midiInOpen(&Midi_In_Handle, device, (DWORD) Midi_In_Proc, 0, CALLBACK_FUNCTION);
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

void Midi_Sysex_Process(void)
{
   tBuffer buffer;
   UInt32 offset;
   UInt8 code,prog;

   offset=0;
   buffer = Queue_Pop();
   if (buffer.buffer != NULL)
   {
     if (memcmp(Sysex_Start, buffer.buffer+offset, sizeof(Sysex_Start))==0)
     {
       offset+=sizeof(Sysex_Start);
       if (memcmp(Sysex_Alesis, buffer.buffer+offset, sizeof(Sysex_Alesis))==0)
       {
         offset+=sizeof(Sysex_Alesis);
         if (memcmp(Sysex_QuadGT, buffer.buffer+offset, sizeof(Sysex_QuadGT))==0)
         {
           offset+=sizeof(Sysex_QuadGT);

	   code = *(buffer.buffer+offset);
	   offset+=1;
	   prog = *(buffer.buffer+offset);
	   offset+=1;

	   FormDebug->Log(NULL, "Code: "+AnsiString(code)+"  Program: "+AnsiString(prog)+"   Bytes: "+AnsiString(buffer.length-offset));
	 }
       }
     }
     free(buffer.buffer);
   }
}

void Midi_Test(void)
{
  UInt8 quad_data[]={0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 
                     0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81};
  UInt8 sysex_data[100];
  UInt32 count;

  count=QuadGT_Encode_To_Sysex(quad_data, sizeof(quad_data), sysex_data, sizeof(sysex_data));

  count=QuadGT_Decode_From_Sysex(sysex_data, count, quad_data, sizeof(quad_data));

  UInt8 sysex_data2[]={0x00, 0x32, 0x00, 0x11, 0x40, 0x1F, 0x20, 0x64, 0x00, 0x46, 0x03, 0x74, 0x00, 0x04, 0x30, 0x14, 0x00, 0x64, 0x0C, 0x40, 0x08, 0x60, 0x27, 0x08, 0x32, 0x00, 0x23, 0x00, 0x04, 0x6C, 0x02, 0x01, 0x00, 0x08, 0x45, 0x40, 0x00, 0x4A, 0x2A, 0x50, 0x00, 0x40, 0x20, 0x03, 0x60, 0x04, 0x02, 0x63, 0x00, 0x54, 0x22, 0x00, 0x0A, 0x44, 0x20, 0x00, 0x3F, 0x40, 0x60, 0x00, 0x00, 0x00, 0x4E, 0x37, 0x31, 0x4F, 0x01, 0x03, 0x61, 0x00, 0x10, 0x00, 0x00, 0x13, 0x60, 0x00, 0x03, 0x10, 0x09, 0x47, 0x00, 0x18, 0x60, 0x00, 0x01, 0x48, 0x30, 0x16, 0x0A, 0x04, 0x43, 0x00, 0x5A, 0x00, 0x30, 0x0B, 0x20, 0x44, 0x60, 0x44, 0x18, 0x38, 0x00, 0x20, 0x31, 0x40, 0x04, 0x06, 0x18, 0x00, 0x40, 0x63, 0x00, 0x08, 0x0C, 0x30, 0x01, 0x01, 0x46, 0x00, 0x00, 0x13, 0x2D, 0x17, 0x19, 0x01, 0x22, 0x75, 0x32, 0x59, 0x2D, 0x62, 0x01, 0x00, 0x40, 0x20, 0x10, 0x18, 0x6C, 0x31, 0x6B, 0x0C, 0x00, 0x00, 0x03, 0x58, 0x60};
  UInt8 quad_data2[100];
  count=QuadGT_Decode_From_Sysex(sysex_data2, sizeof(sysex_data2), quad_data2, sizeof(quad_data2));

  QuadGT_Display_Update(10, quad_data2);

}

//---------------------------------------------------------------------------
// Name      : quadgt.cpp
//---------------------------------------------------------------------------

static UInt8 Quad_Patch[QUAD_NUM_PATCH][QUAD_PATCH_SIZE];

void QuadGT_Display_Update(UInt8 program, UInt8 *quad_data)
{
  char prog_name[NAME_LENGTH+1];
  UInt8 config;

  /* Program Number */
  MainForm->QuadPatchNum->Text=AnsiString(program);

  /* Program name */
  memcpy(prog_name, &quad_data[NAME_IDX], NAME_LENGTH);
  prog_name[NAME_LENGTH]=0;
  MainForm->EditQuadPatchName->Text=AnsiString(prog_name);

 
  /* Configuration */
  config=quad_data[CONFIG_IDX];
  MainForm->QuadConfig->ItemIndex=config;

  QuadGT_Display_Update_Reverb(config, quad_data);

  QuadGT_Display_Update_Delay(config, quad_data);

  QuadGT_Display_Update_Pitch(config, quad_data);

  QuadGT_Display_Update_Eq(config, quad_data);

  QuadGT_Display_Update_Mix(config, quad_data);

  QuadGT_Display_Update_Mod(config, quad_data);

  QuadGT_Display_Update_Preamp(config, quad_data);
}

void QuadGT_Display_Update_Reverb(const UInt8 config, const UInt8 * const quad_data)
{
  UInt8 val;

  if (config==0)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=true;
    MainForm->ReverbInMix->Visible=true;
  }
  else if ((config==1) || (config==5) || (config==6))
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=true;
    MainForm->ReverbInMix->Visible=true;
    // TBD: Disable input 1, selection 3
  }
  else if ((config==2) || (config==7))
  {
    MainForm->QuadReverb->Visible=false;
  }
  else if (config==3)
  {
    // TBD
  }
  else if (config==4)
  {
    MainForm->QuadReverb->Visible=true;
    MainForm->ReverbInput1->Visible=FALSE;
    MainForm->ReverbInMix->Visible=FALSE;
  }

  MainForm->ReverbType->ItemIndex = quad_data[REVERB_MODE_IDX];

}

void QuadGT_Display_Update_Delay(const UInt8 config, const UInt8 * const quad_data)
{
  UInt8 mode;

  mode=quad_data[DELAY_MODE_IDX];
  if (mode ==0)
  {
    MainForm->DelayTap->Visible=FALSE;
  }
  else if (mode ==1)
  {
    MainForm->DelayTap->Visible=FALSE;
  }
}

void QuadGT_Display_Update_Pitch(const UInt8 config, const UInt8 * const quad_data)
{
  if ((config==0) || (config==3))
  {
    MainForm->QuadPitch->Visible=TRUE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
  }
  else if (config==1)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=TRUE;
    MainForm->QuadRingMod->Visible=FALSE;
  }
  else if ((config==2) || (config==6) || (config==7))
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
  }
  else if (config==4)
  {
    MainForm->QuadPitch->Visible=TRUE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;

    MainForm->PitchWave->Visible=FALSE;
    MainForm->PitchChorus->Visible=TRUE;
  }
  else if (config==5)
  {
    MainForm->QuadPitch->Visible=FALSE;
    MainForm->QuadLeslie->Visible=FALSE;
    MainForm->QuadRingMod->Visible=FALSE;
  }
}

void QuadGT_Display_Update_Eq(const UInt8 config, const UInt8 * const quad_data)
{
  // 3 Band Eq
  if ((config==0) || (config==4))
  {
    MainForm->PanelQuadEq3->Visible=TRUE;

    MainForm->EqFreq1->Position = quad_data[LOW_EQ_FREQ_IDX];
    MainForm->EqAmp1->Position = quad_data[LOW_EQ_AMP_IDX];

    MainForm->EqFreq2->Position = quad_data[MID_EQ_FREQ_IDX];
    MainForm->EqAmp2->Position = quad_data[MID_EQ_AMP_IDX];
    MainForm->EqQ2->Position = quad_data[MID_EQ_BW_IDX];

    MainForm->EqFreq3->Position = quad_data[HIGH_EQ_FREQ_IDX];
    MainForm->EqAmp3->Position = quad_data[HIGH_EQ_AMP_IDX];
  }
  else
  {
    MainForm->PanelQuadEq3->Visible=FALSE;
  }

  // 5 Band Eq
  if (config==3) MainForm->QuadEq5->Visible=TRUE;
  else MainForm->QuadEq5->Visible=FALSE;

  // Graphic Eq

  // Resonator
}

void QuadGT_Display_Update_Mix(const UInt8 config, const UInt8 * const quad_data)
{
  UInt8 val;

  // Pre or Post Eq
  val=quad_data[PREPOST_EQ_IDX] & BIT0;
  if (val == 0) 
  {
    MainForm->PrePostEq->ItemIndex=0;
  }
  else
  {
    MainForm->PrePostEq->ItemIndex=1;
  }

  // Direct level
  val=(quad_data[DIRECT_LEVEL_IDX ] & BITS1to7) >> 1;
  MainForm->MixDirect->Position=99-val;

  // Master Fx Level
  if (config != 7)
  {
    val=quad_data[MASTER_EFFECTS_LEVEL_IDX ];
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
    val=quad_data[PREAMP_LEVEL_IDX ];
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
  val=quad_data[PITCH_LEVEL_IDX ];
  MainForm->MixPitch->Position=99-val;

  // Delay Level
  val=quad_data[DELAY_LEVEL_IDX ];
  MainForm->MixDelay->Position=99-val;

  // Reverb Level
  val=quad_data[REVERB_LEVEL_IDX ];
  MainForm->MixReverb->Position=99-val;

  // Modulation (on/off)
  // TBD: Fix grouping so this doesnt effect pre/post eq
  val=(quad_data[MIX_MOD_IDX] & BITS6to7) >> 6;
  if (val == 0) MainForm->Modulation->Checked=FALSE;
  else MainForm->Modulation->Checked=TRUE;

  // Mod Depth
  // Mod Speed
 
  // Leslie Level

  // Eq Level
  if (MainForm->PrePostEq->ItemIndex==1)
  {
    val=quad_data[EQ_LEVEL_IDX ];
    MainForm->MixPreampEq->Position=99-val;
  }

  // Ring Level
  
  // Resonator Level
}

void QuadGT_Display_Update_Mod(const UInt8 config, const UInt8 * const quad_data)
{
  // Modulation
  switch (config)
  {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      MainForm->QuadModSource1->ItemIndex=quad_data[MOD1_SOURCE_IDX];
      MainForm->QuadModTarget1->ItemIndex=quad_data[MOD1_TARGET_IDX];
      MainForm->QuadModAmp1->Position=quad_data[MOD1_AMP_IDX]-99;
      break;

    case 7:
    default:
      break;
  }
}

void QuadGT_Display_Update_Preamp(const UInt8 config, const UInt8 * const quad_data)
{
  UInt8 val;

  // Compression (0-7)
  val=(quad_data[PREAMP_COMP_IDX] & BITS4to6) >> 4;
  MainForm->PreComp->Position=7-val;
  FormDebug->Log(NULL,"Compression: "+AnsiString(val));

  // Overdrive (0-7)
  val=(quad_data[PREAMP_OD_IDX] & BITS5to7) >> 5;
  MainForm->PreOd->Position=7-val;
  FormDebug->Log(NULL,"OD: "+AnsiString(val));

  // Distortion (0-8)
  val=(quad_data[PREAMP_DIST_IDX] & BITS0to3);
  MainForm->PreDist->Position=8-val;

  // Tone (0-2)
  val=(quad_data[PREAMP_TONE_IDX] & BITS2to3) >> 2;
  MainForm->PreTone->ItemIndex=val;

  // Bass Boost (0-1)
  // TBD: Make a check box (on/off)

  // Cab sim (0-2)
  // TBD: Make a radio button:  ???
  
  // Effect Loop (0-1)
  // TBD: Make a check box (in/out)
 
  // Noise Gate (0-17)
  val=(quad_data[PREAMP_GATE_IDX] & BITS0to4);
  MainForm->PreGate->Position=17-val;
  FormDebug->Log(NULL,"Gate: "+AnsiString(val));

  /* Preamp Output Level (0-99) */
  val=quad_data[PREAMP_OUT_LEVEL_IDX];
  MainForm->PreOutLevel->Position = 99-val;
  FormDebug->Log(NULL, "Preamp output level :"+AnsiString(val));

}
