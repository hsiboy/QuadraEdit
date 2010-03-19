//---------------------------------------------------------------------------
#ifndef midiH
#define midiH
//---------------------------------------------------------------------------
#endif

void Midi_Get_Dev_Lists(TComboBox *in_list,TComboBox *out_list, TLabel * error_text);

UInt8 Midi_Out_Open(int device);
void Midi_Out_Dump_Req(UInt8 program);
void Midi_Out_ShortMsg(const unsigned long message);

UInt8 Midi_In_Open(int device);

void Midi_Get_Counts(int *msg_count_ptr, int *sysex_count_ptr);
