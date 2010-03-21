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
class Tmidiform : public TForm
{
__published:	// IDE-managed Components
    TPanel *Midi;
    TButton *Ok;
    TLabel *LabelMidi;
private:	// User declarations
public:		// User declarations
    __fastcall Tmidiform(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE Tmidiform *midiform;
//---------------------------------------------------------------------------
void Midi_Get_Dev_Lists(TComboBox *in_list,TComboBox *out_list, TLabel * error_text);

UInt8 Midi_Out_Open(int device);
unsigned int Midi_Out_Dump_Req(UInt8 program);
void Midi_Out_ShortMsg(const unsigned long message);

unsigned int Midi_In_Open(int device);

void Midi_Get_Counts(int *msg_count_ptr, int *sysex_count_ptr, int *other_count_ptr);
unsigned int Midi_Out_Close(void);
unsigned int Midi_In_Close(void);
void decode_quad(UInt8 *buffer, UInt32 length);
#endif
