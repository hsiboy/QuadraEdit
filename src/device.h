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
    TLabel *LabelOutDevName;
    TComboBox *ComboBoxOutDevs;
    TLabel *LabelInDevName;
    TComboBox *ComboBoxInDevs;
    TLabel *LabelMidiDevError;
    TButton *ButtonMidiDevOpen;
    TButton *ButtonMidiDevClose;
private:	// User declarations
public:		// User declarations
    __fastcall TFormDevice(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormDevice *FormDevice;
//---------------------------------------------------------------------------
#endif
