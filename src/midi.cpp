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
