#include <vcl.h>
#include <mmsystem.h>                       // we need this

#include <stdlib.h>
#include <stdio.h>

#pragma hdrstop

#include "types.h"
#include "midi.h"
#include "error.h"
#include "debug.h"

#define MIDI_IN_BUF_SIZE  (2048)
#define MIDI_IN_NUM_BUF    (1)


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


const UInt8 Sysex_Hdr[]={0xF0,              // SysEx
                         0x00,0x00,0x0E,    // Alesis Manufacturer Id
                         0x07};             // Quadraverb GT

const UInt8 Sysex_Edit[]={0x01};                // Command to edit a Quadraverb function
const UInt8 Sysex_Data_Dump[]={0x02};           // Command to send a program data dump
const UInt8 Sysex_Dump_Req[]={0x03};            // Command to request a dump

const UInt8 Sysex_End[]={0xF7};             // SysEx End

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
static int rx_msg=100;
static int rx_other=0;

void Midi_Init(void)
{
  int i,j;
  tBuffer dummy;
  tBuffer * ptr;

  Rx_Msg_Queue.ptr=NULL;


  for (i=0; i<12; i++)
  {
    dummy.buffer = (UInt8 *)malloc(i+1);
    if (dummy.buffer == NULL) return;
    for (j=0; j<=i; j++)
    {
      *(dummy.buffer+j)=i+(2*j);
    }
    dummy.length=i+1;
    Queue_Push(dummy);
  }
  FormDebug->Log(NULL, "Queue push test done");

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
    //tbd
    return;
  }

  // Create space for payload buffer
  entry->payload.buffer = (UInt8 *) malloc(buffer.length);
  if (entry->payload.buffer == NULL)
  {
    // TBD
    free(entry);
    return;
  }

  // Copy buffer into queue entry payload
  memcpy(entry->payload.buffer, \
         buffer.buffer,  \
         buffer.length);
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
  memcpy(buffer, Sysex_Hdr, sizeof(Sysex_Hdr));
  buf_len+=sizeof(Sysex_Hdr);

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

      FormDebug->Log(NULL,"RX: "+AnsiString((UInt32)header_ptr->dwBytesRecorded));

      if (Midi_Open == TRUE)
      {
        entry.buffer=header_ptr->lpData;
        entry.length=header_ptr->dwBytesRecorded;
        Queue_Push(entry);
        // Process the data

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
void decode_quad(UInt8 *buffer, UInt32 length)
{
  UInt8 oc;
  UInt8 out[1000];
  UInt32 i,j;
  UInt8 shift;

  FormDebug->LogHex(NULL,"To Decode :",buffer,length);
  j=0;
  oc = 0;
  for (i=0; i<length; i++) {
     if (shift = i % 8) {
        oc = (oc << shift) + (buffer[i] >> (7-shift));
        out[j++]=oc;
     }
     oc = buffer[i];
  }
  FormDebug->LogHex(NULL,"Decoded :",out,j);
}

void process(void)
{
   tBuffer buffer;

   buffer = Queue_Pop();
   if (buffer.buffer != NULL)
   {
     FormDebug->LogHex(NULL, "RX: ",  buffer.buffer, buffer.length );
     free(buffer.buffer);
   }
}
