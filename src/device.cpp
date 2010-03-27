//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "device.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormDevice *FormDevice;
//---------------------------------------------------------------------------
__fastcall TFormDevice::TFormDevice(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
