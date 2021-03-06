//---------------------------------------------------------------------------
#ifndef midiH
#define midiH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
// Literal defintions

#define QUADGT_SCRATCH (100)    // Patch used as a scratch buffer
#define QUADGT_ALL     (101)    // All patches

//---------------------------------------------------------------------------
// Type definitions

// Contents of a SYSEX message
typedef struct tBuffer
{
    UInt8 * buffer;
    UInt32 length;
} tBuffer;

// Contents of a data message
typedef struct tData
{
  DWORD              p1;
  // Don't bother with p2 - just a timestamp
} tData;

//---------------------------------------------------------------------------
// Constant definitions
const UInt8 Sysex_Start[]={0xF0};              // SysEx start 
const UInt8 Sysex_End[]={0xF7};                // SysEx End

//---------------------------------------------------------------------------
// Variable definitions

//---------------------------------------------------------------------------
// Prototype definitions
void Midi_Init(void);
void Queue_Push(tBuffer buffer);
tBuffer Queue_Pop(void);

int Midi_Get_IO_Dev_List(TComboBox *list);

UInt8 Midi_IO_Open(AnsiString device_name);
UInt32 Midi_Out_Dump(UInt8 program, UInt8 *data, UInt16 size);
unsigned int Midi_Out_Dump_Req(UInt8 program);
unsigned int Midi_Out_Edit(UInt8 function, UInt8 page, UInt16 data);
void Midi_Out_ShortMsg(const unsigned long message);

unsigned int Midi_In_Open(int device);

void Midi_Get_Counts(int *msg_count_ptr, int *sysex_count_ptr, int *other_count_ptr);
unsigned int Midi_IO_Close(void);
void Midi_Sysex_Process(void);
#endif
