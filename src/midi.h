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

typedef struct tBuffer
{
    UInt8 * buffer;
    UInt32 length;
} tBuffer;

//---------------------------------------------------------------------------
// Variable definitions

//---------------------------------------------------------------------------
// Prototype definitions
void Midi_Init(void);
void Queue_Push(tBuffer buffer);
tBuffer Queue_Pop(void);

void Midi_Get_Dev_Lists(TComboBox *in_list,TComboBox *out_list, TLabel * error_text);

UInt8 Midi_Out_Open(int device);
unsigned int Midi_Out_Dump_Req(UInt8 program);
unsigned int Midi_Out_Edit(UInt8 function, UInt8 page, UInt16 data);
void Midi_Out_ShortMsg(const unsigned long message);

unsigned int Midi_In_Open(int device);

void Midi_Get_Counts(int *msg_count_ptr, int *sysex_count_ptr, int *other_count_ptr);
unsigned int Midi_Out_Close(void);
unsigned int Midi_In_Close(void);
void Midi_Sysex_Process(void);

void Midi_Test(void);
#endif
