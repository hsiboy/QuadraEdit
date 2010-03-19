#include <vcl.h>
#include <mmsystem.h>                       // we need this
#pragma hdrstop

#include "types.h"
#include "midi.h"
#include "error.h"

#define MIDI_IN_BUF_SIZE  (2048)
          
//---------------------------------------------------------------------------
#pragma package(smart_init)
//#pragma resource "*.dfm"
Tmidiform *midiform;
//---------------------------------------------------------------------------
__fastcall Tmidiform::Tmidiform(TComponent* Owner)
    : TForm(Owner)
{
}

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
static MIDIHDR Midi_In_1;
static UInt8 Midi_In_1_Buffer[MIDI_IN_BUF_SIZE];

// Rx message counters
static int rx_sysex=0;
static int rx_msg=100;
static int rx_other=0;

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


  switch (msg)
  {
    case MIM_LONGDATA:
      // TBD: Handle SYSEX data input
      header_ptr = (MIDIHDR *)p1;
      rx_sysex++;

      // Make the buffer free for next lot of SysEx data
      header_ptr->dwBytesRecorded = 0;
      //midiInAddBuffer(Midi_In_Handle, &Midi_In_1, sizeof(Midi_In_1));
      midiInPrepareHeader(Midi_In_Handle, &Midi_In_1, sizeof(Midi_In_1));
      // TBD: Is this Ok?  (Should not be calling system fucntions in a callback)
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
  MMRESULT status;

  status=midiInOpen(&Midi_In_Handle, device, (DWORD) Midi_In_Proc, 0, CALLBACK_FUNCTION);
  if (status != MMSYSERR_NOERROR)
  {
    // TBD: Report open error details
    return status;
  }

  // TBD: Setup more than one MIDI in header/buffer

  // Setup MIDI in header
  Midi_In_1.lpData = Midi_In_1_Buffer;
  Midi_In_1.dwBufferLength = sizeof(Midi_In_1_Buffer);
  Midi_In_1.dwBytesRecorded = 0;
  Midi_In_1.dwFlags = 0;
  status = midiInPrepareHeader(Midi_In_Handle, &Midi_In_1, sizeof(Midi_In_1));
  if (status != MMSYSERR_NOERROR)
  {
    // TBD: Report header error details
    return status;
  }

  // Add a buffer to the MIDI in header
  status=midiInAddBuffer(Midi_In_Handle, &Midi_In_1, sizeof(Midi_In_1));
  if (status != MMSYSERR_NOERROR)
  {
    // TBD: Report buffer error details
    return status;
  }

  midiInStart(Midi_In_Handle);

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
  midiInStop(Midi_In_Handle);
  //status=midiInReset(Midi_In_Handle);
  //if (status != MMSYSERR_NOERROR) return(status);
  status=midiInClose(Midi_In_Handle);
  return(status);
}
unsigned int Midi_Out_Close(void)
{
  unsigned int status;
  status=midiOutClose(Midi_Out_Handle);
  return(status);
}
