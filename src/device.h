//---------------------------------------------------------------------------
#ifndef deviceH
#define deviceH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TFormDevice : public TForm
{
__published:	// IDE-managed Components
    TLabel *LabelInDevName;
    TComboBox *ComboBoxInDevs;
    TLabel *LabelMidiDevError;
    TButton *ButtonMidiDevOpen;
    TButton *ButtonMidiDevCancel;
    void __fastcall ButtonMidiDevOpenClick(TObject *Sender);
    void __fastcall ButtonMidiDevCancelClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TFormDevice(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormDevice *FormDevice;
//---------------------------------------------------------------------------
#endif
