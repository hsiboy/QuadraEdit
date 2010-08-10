//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "types.h"
#include "error.h"
#include "debug.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormError *FormError;
//---------------------------------------------------------------------------
// Name        : 
// Description : 
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
__fastcall TFormError::TFormError(TComponent* Owner)
    : TForm(Owner)
{
}

//---------------------------------------------------------------------------
// Name        : 
// Description : 
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void __fastcall TFormError::FormHide(TObject *Sender)
{
 ModalResult=1;
}

//---------------------------------------------------------------------------
// Name        : 
// Description : 
// Parameters  : 
// Returns     : NONE.
//---------------------------------------------------------------------------
void __fastcall TFormError::ButtonOkClick(TObject *Sender)
{
  FormHide(Sender);
}

//---------------------------------------------------------------------------
// Name        : ShowErrorCode
// Description : Display an error dialog with an error code and description
// Parameter 1 : Numeric error code
// Parameter 2 : Error description string
// Returns     : NONE.
//---------------------------------------------------------------------------
void __fastcall TFormError::ShowErrorCode(UInt32 code, AnsiString string)
{
  LabelError->Caption="Error ["+AnsiString(code)+"]: "+string;
  ShowModal();
}

//---------------------------------------------------------------------------
// Name        : ShowError
// Description : Display an error dialog with an error description
// Parameter 1 : Error description string
// Returns     : NONE.
//---------------------------------------------------------------------------
void __fastcall TFormError::ShowError(AnsiString string)
{
  LabelError->Caption="Error: "+string;
  ShowModal();
}
