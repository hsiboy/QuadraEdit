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

#define DATA_EMPTY (0x00000000)

//---------------------------------------------------------------------------
#pragma package(smart_init)

static boolean Midi_Open=FALSE;

typedef struct tSysex_Queue_Entry
{
  tBuffer payload;
  tSysex_Queue_Entry *next_ptr;
} tSysex_Queue_Entry;

typedef struct tSysex_Queue
{
  tSysex_Queue_Entry *ptr;
} tSysex_Queue;

typedef struct tData_Queue_Entry
{
  tData              payload;
  tData_Queue_Entry *next_ptr;
} tData_Queue_Entry;

typedef struct tData_Queue
{
  tData_Queue_Entry *ptr;
} tData_Queue;


// Midi handles
static HMIDIOUT Midi_Out_Handle;
static HMIDIIN Midi_In_Handle;

// Headers and associated buffers for Midi Input SYSEX messages
typedef struct tMidi_In
{
  MIDIHDR Hdr;
  UInt8   Buffer[MIDI_IN_BUF_SIZE];
} tMidi_In;

static tMidi_In Midi_In;

static tSysex_Queue Sysex_Queue;
static tData_Queue Data_Queue;

// Rx message counters
static int rx_sysex=0;
static int rx_msg=0;
static int rx_other=0;

static TStringList *in_device;   // List of Input Midi Devices
static TStringList *out_device;  // List of Output Midi Devices

static void CALLBACK Midi_In_Callback(HMIDIIN handle, UINT msg,
                                      DWORD *instance, DWORD *p1, DWORD *p2);
//---------------------------------------------------------------------------
// Name        : Midi_Init
// Description : Initialise the MIDI input/output system
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void Midi_Init(void)
{
  Sysex_Queue.ptr=NULL;

  in_device = new(TStringList);
  out_device = new(TStringList);

  FormDebug->Log(NULL, "MIDI Init done");
}

//---------------------------------------------------------------------------
// Name        : Sysex_Queue_Push
// Description : Push SYSEX data onto end of the queue for later processing
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void Sysex_Queue_Push(tBuffer buffer)
{
  tSysex_Queue_Entry *entry;
  tSysex_Queue_Entry *tail;

  // Create new queue entry
  entry = (tSysex_Queue_Entry *)malloc(sizeof(tSysex_Queue_Entry));
  if (entry == NULL)
  {
    FormError->ShowErrorCode(1,"no memory left to create SYSEX queue entry");
    return;
  }

  // Create space for payload buffer
  entry->payload.buffer = (UInt8 *) malloc(buffer.length);
  if (entry->payload.buffer == NULL)
  {
    FormError->ShowErrorCode(2,"no memory left to queue SYSEX data");
    free(entry);
    return;
  }

  // Copy buffer into queue entry payload
  memcpy(entry->payload.buffer, buffer.buffer, buffer.length);
  entry->payload.length=buffer.length;

  entry->next_ptr=NULL;

  // Push entry onto tail of queue
  if (Sysex_Queue.ptr == NULL)
  {
    Sysex_Queue.ptr = entry;
  }
  else
  {
    tail = Sysex_Queue.ptr;
    while (tail->next_ptr != NULL) tail=tail->next_ptr;
    tail->next_ptr = entry;
  }

}

//---------------------------------------------------------------------------
// Name        : Sysex_Queue_Pop
// Description : Pop an entry from the head of the queue
// Parameters  : 
// Returns     : Item poped from queue.
//---------------------------------------------------------------------------
tBuffer Sysex_Queue_Pop(void)
{
  tSysex_Queue_Entry * head;
  tBuffer payload;

  // Is queue empty?
  if (Sysex_Queue.ptr == NULL) 
  {
    payload.buffer = NULL;
    payload.length = 0;
  }
  else
  {
    head = Sysex_Queue.ptr;
    payload = head->payload;

    Sysex_Queue.ptr = head->next_ptr;

    // Free the queue entry
    free(head);

  }

  // Return the payload, the caller must free payload.buffer when done
  return(payload);
}

//---------------------------------------------------------------------------
// Name        : Data_Queue_Push
// Description : Push Data data onto end of the queue for later processing
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void Data_Queue_Push(tData data)
{
  tData_Queue_Entry *entry;
  tData_Queue_Entry *tail;

  // Create new queue entry
  entry = (tData_Queue_Entry *)malloc(sizeof(tData_Queue_Entry));
  if (entry == NULL)
  {
    FormError->ShowErrorCode(1,"no memory left to create Data queue entry");
    return;
  }

  entry->payload=data;
  entry->next_ptr=NULL;

  // Push entry onto tail of queue
  if (Data_Queue.ptr == NULL)
  {
    Data_Queue.ptr = entry;
  }
  else
  {
    tail = Data_Queue.ptr;
    while (tail->next_ptr != NULL) tail=tail->next_ptr;
    tail->next_ptr = entry;
  }

}

//---------------------------------------------------------------------------
// Name        : Data_Queue_Pop
// Description : Pop an entry from the head of the queue
// Parameters  : 
// Returns     : Item poped from queue.
//---------------------------------------------------------------------------
tData Data_Queue_Pop(void)
{
  tData_Queue_Entry * head;
  tData payload;

  // Is queue empty?
  if (Data_Queue.ptr == NULL) 
  {
    payload.p1 = NULL;
  }
  else
  {
    head = Data_Queue.ptr;
    payload = head->payload;

    Data_Queue.ptr = head->next_ptr;

    // Free the queue entry
    free(head);

  }

  return(payload);
}


//---------------------------------------------------------------------------
// Name        : 
// Description : 
// Parameters  : 
// Returns     : Number of Midi IO devices available.
//---------------------------------------------------------------------------
int Midi_Get_IO_Dev_List(TComboBox *list)
{
  int devs,i;
  MMRESULT status;
  MIDIOUTCAPS caps;
  MIDIINCAPS incaps;

  /**************************************************************************
  * Build the list of possible MIDI output devices
  **************************************************************************/
  out_device->Clear();
  devs = midiOutGetNumDevs();
  if (devs >= 1) {
    for (i=0; i<devs; i++)
    {
      status = midiOutGetDevCaps(i, &caps, sizeof(caps));
      if (status == MMSYSERR_NOERROR)
      {
        out_device->Add(caps.szPname);
      }
      else
      {
        out_device->Add("MIDI Out "+AnsiString(i+1)+" - ERROR");
      }
    }
  }

  /**************************************************************************
  * Build the list of possible MIDI input devices
  **************************************************************************/
  in_device->Clear();
  devs = midiInGetNumDevs();
  if (devs >= 1) {
    for (i=0; i<devs; i++)
    {
      status = midiInGetDevCaps(i, &incaps, sizeof(incaps));
      if (status == MMSYSERR_NOERROR)
      {
        in_device->Add(incaps.szPname);
      }
      else
      {
        in_device->Add("MIDI In "+AnsiString(i+1)+" - ERROR");
      }
    }
  }

  /**************************************************************************
  * Build the list of possible MIDI input and output devices
  **************************************************************************/
  list->Items->Clear();
  for (i=0; i<out_device->Count; i++)
  {
    if (in_device->IndexOf(out_device->Strings[i]) >= 0)
    {
      list->Items->Add(out_device->Strings[i]);
      FormDebug->Log(NULL, "Device "+AnsiString(out_device->Strings[i]));
    }
  }
  list->ItemIndex=0;

  FormDebug->Log(NULL, "Devices "+AnsiString(list->Items->Count));
  return(list->Items->Count);

}

//---------------------------------------------------------------------------
// Name        : 
// Description : 
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
UInt8 Midi_IO_Open(AnsiString device_name)
{
  MMRESULT status=MMSYSERR_NOERROR;
  int in,out;
  UInt8 i;

  in = in_device->IndexOf(device_name);
  out = out_device->IndexOf(device_name);

  FormDebug->Log(NULL, "Opening "+AnsiString(device_name));

  if ((in < 0) && (out < 0))
  {
    FormDebug->Log(NULL, "***ERROR: Open, no MIDI device available");
    status = 1;
  }

  if (status == MMSYSERR_NOERROR)
  {
    status=midiOutOpen(&Midi_Out_Handle, out, 0, 0, CALLBACK_NULL);
    if (status != MMSYSERR_NOERROR)
    {
      FormDebug->Log(NULL, "***ERROR: Can't open MIDI output device");
      midiOutClose(Midi_Out_Handle);
      status = 1;
    }
  }

  if (status == MMSYSERR_NOERROR)
  {
    status=midiInOpen(&Midi_In_Handle, in, (DWORD) Midi_In_Callback, 0, CALLBACK_FUNCTION);
  }

  if (status == MMSYSERR_NOERROR)
  {

    // Setup MIDI in header
      Midi_In.Hdr.lpData = Midi_In.Buffer;
      Midi_In.Hdr.dwBufferLength = sizeof(Midi_In.Buffer);
      Midi_In.Hdr.dwBytesRecorded = 0;
      Midi_In.Hdr.dwFlags = 0;
      status = midiInPrepareHeader(Midi_In_Handle, &Midi_In.Hdr, sizeof(Midi_In.Hdr));
   }

  if (status == MMSYSERR_NOERROR)
  {
    // Add a buffer to the MIDI in header
    status=midiInAddBuffer(Midi_In_Handle, &Midi_In.Hdr, sizeof(Midi_In.Hdr));
  }

  if (status == MMSYSERR_NOERROR)
  {
    midiInStart(Midi_In_Handle);
    Midi_Open=TRUE;
  }

  if (status != MMSYSERR_NOERROR)
  {
    FormDebug->Log(NULL, "***ERROR: Can't open MIDI input device");
    midiOutClose(Midi_In_Handle);
    midiOutClose(Midi_Out_Handle);
    status = 1;
  }

  FormDebug->Log(NULL, "OPEN: "+AnsiString(status));
  return status;
}
//---------------------------------------------------------------------------
// Name        : Midi_Out_Dump
// Description : Send a dump sysex message to the device (i.e. send a program
//               to the device)
// Param 1     : Program number to send
// Param 2     : Data to send (does not include Sysex headers)
// Param 3     : Size of data to send
//---------------------------------------------------------------------------
UInt32 Midi_Out_Dump(UInt8 program, UInt8 *data, UInt16 size)
{
  MIDIHDR out;
  UInt8 buffer[500];
  UInt8 buf_len=0;
  long int status;

  // Build message in buffer
  memcpy(buffer+buf_len, Sysex_Start, sizeof(Sysex_Start));
  buf_len+=sizeof(Sysex_Start);

  memcpy(buffer+buf_len, Sysex_Alesis, sizeof(Sysex_Alesis));
  buf_len+=sizeof(Sysex_Alesis);

  memcpy(buffer+buf_len, Sysex_QuadGT, sizeof(Sysex_QuadGT));
  buf_len+=sizeof(Sysex_QuadGT);

  memcpy(buffer+buf_len, Sysex_Data_Dump, sizeof(Sysex_Dump_Req));
  buf_len+=sizeof(Sysex_Dump_Req);

  buffer[buf_len] = program;
  buf_len+=1;

  memcpy(buffer+buf_len, data, size);
  buf_len+=size;

  memcpy(buffer+buf_len, Sysex_End, sizeof(Sysex_End));
  buf_len+=sizeof(Sysex_End);

  FormDebug->LogHex(NULL, "TX "+AnsiString(buf_len)+": ", buffer, buf_len);

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
//---------------------------------------------------------------------------
// Name        : Midi_Out_Dump_Req
// Description : Send a dump request to an Alesis Quadraverb via Midi
// Parameters  : Program to be requested 
// Returns     : NONE.
//---------------------------------------------------------------------------
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

//---------------------------------------------------------------------------
// Name        : Midi_Out_Edit
// Description : Output a Midi Edit command
// Parameter 1 : function - Function code to send Midi device
// Parameter 2 : page - Page number code to send Midi device
// Parameter 3 : data - Data word to send Midi device
// Returns     : NONE.
//---------------------------------------------------------------------------
unsigned int Midi_Out_Edit(UInt8 function, UInt8 page, UInt16 data)
{
  MIDIHDR out;
  UInt8 buffer[100];
  UInt8 buf_len=0;
  long int status;
  UInt8 quadgt[3];   // Two bytes of data converted to Quadraverb format
  UInt32 len;

  FormDebug->Log(NULL, "Sending Midi Edit func: "+AnsiString(function) + " page: " + AnsiString(page) + " data: " + AnsiString(data));
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

  // TBD: Converting program number to 3 bytes of 7bit Sysex data is not working
  QuadGT_Encode_16Bit(data, quadgt);
  FormDebug->Log(NULL, "Encode 16bit: "+AnsiString(data) + " becomes: " + AnsiString(quadgt[0]) + " " + AnsiString(quadgt[1]) );
  len=QuadGT_Encode_To_Sysex(quadgt,2,&buffer[buf_len],3);
  FormDebug->Log(NULL, "Result: "+AnsiString(len) + " bytes");
  //*(buffer+buf_len+0)=10>>1;   // DEBUG: Force to program 10, manually converted to 3 bytes of 7bit SYSEX data
  //*(buffer+buf_len+1)=0;
  //*(buffer+buf_len+2)=0;
  FormDebug->Log(NULL, "Sysex: "+AnsiString(*(buffer+buf_len+0)) + " " +AnsiString(*(buffer+buf_len+1)) + " " +AnsiString(*(buffer+buf_len+2)) );
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
//---------------------------------------------------------------------------
// Name        : Midi_Out_ShortMsg`
// Description : Output a Midi short message
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void Midi_Out_ShortMsg(const unsigned long message)
{
  midiOutShortMsg(Midi_Out_Handle, message);
}
//---------------------------------------------------------------------------
// Name        : Midi_In_Callback
// Description : Call back routine to process data received from Midi In.
//---------------------------------------------------------------------------
static void CALLBACK Midi_In_Callback(HMIDIIN handle, UINT msg,
                                      DWORD *instance, DWORD *p1, DWORD *p2)  // TBD: These aren't pointers!
{
  MIDIHDR *header_ptr;
  UInt32 status;
  tBuffer entry;
  tData data;

  switch (msg)
  {
    case MIM_LONGDATA:
      FormDebug->Log(NULL,"Rx MIDI Sysex");
      // Handle SYSEX data input
      header_ptr = (MIDIHDR *)p1;
      rx_sysex++;

      if (Midi_Open == TRUE)
      {
        entry.buffer=header_ptr->lpData;
        entry.length=header_ptr->dwBytesRecorded;
        Sysex_Queue_Push(entry);

      }

      if (header_ptr->dwBytesRecorded != 0) 
      {
        status=midiInAddBuffer(Midi_In_Handle, header_ptr, sizeof(MIDIHDR));
        if (status != 0) FormDebug->Log(NULL,"Proc buffer error");
      }

      break;

    case MIM_DATA:
      rx_msg++;
      if (Midi_Open == TRUE)
      {
        data.p1=(DWORD)p1;
        Data_Queue_Push(data);
      }

      break;

    case MIM_ERROR:
      FormDebug->Log(NULL,"Rx MIDI Error - ignored");
      break;

    case MIM_CLOSE:
      FormDebug->Log(NULL,"Rx MIDI close - ignored");
      break;

    default:
      FormDebug->Log(NULL,"Rx MIDI other - ignored");
      rx_other++;
      break;
  }


}

void Midi_Get_Counts(int *msg_count_ptr, int *sysex_count_ptr, int *other_count_ptr)
{
  *msg_count_ptr=rx_msg;
  *sysex_count_ptr=rx_sysex;
  *other_count_ptr=rx_other;
}

//---------------------------------------------------------------------------
// Name        : 
// Description : 
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
unsigned int Midi_IO_Close(void)
{
  unsigned int status=MMSYSERR_NOERROR;

  if (Midi_Open == TRUE)
  {
    Midi_Open=FALSE;

    midiOutClose(Midi_Out_Handle);

    midiInUnprepareHeader(Midi_In_Handle, &Midi_In.Hdr, sizeof(Midi_In.Hdr));

    status=midiInStop(Midi_In_Handle);
    if (status != MMSYSERR_NOERROR) return(status);

    status=midiInReset(Midi_In_Handle);
    if (status != MMSYSERR_NOERROR) return(status);

    status=midiInClose(Midi_In_Handle);
    if (status != MMSYSERR_NOERROR) return(status);


  }
  return(status);
}

//---------------------------------------------------------------------------
// Name        : Midi_Sysex_Process
// Description : Process any Midi Sysex messages received and placed in the
//               queue.
// Note        : Called from a timer.
//---------------------------------------------------------------------------
void Midi_Sysex_Process(void)
{
   tBuffer sysex;
   tData data;
   UInt8 code,prog;
   UInt8 quadgt[QUAD_PATCH_SIZE];

   // Process Sysex messages
   sysex = Sysex_Queue_Pop();
   if (sysex.buffer != NULL)
   {
     QuadGT_Sysex_Process(sysex);
     free(sysex.buffer);
   }

   // Process Data messages
   data = Data_Queue_Pop();
   if (data.p1 != NULL)
   {
     QuadGT_Data_Process(data);
   }
}

