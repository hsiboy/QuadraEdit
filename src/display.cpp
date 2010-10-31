#include "types.h"
#include "main.h"
#include "display.h"

//---------------------------------------------------------------------------
// Name        : RedrawHorizBarTextS8
// Description : Redraw a horizontal bar control and its associated text
// Parameters  : 
// Returns     : NONE.
// NOTE        : Currently unused, remove?
//---------------------------------------------------------------------------
void RedrawHorizBarTextS8(TTrackBar *bar, TEdit *text, SInt8 param)
{
  bar->Position=param;
  bar->Hint=AnsiString(param);
  text->Text=AnsiString(param);
}

//---------------------------------------------------------------------------
// Name        : RedrawVertBarTextS8
// Description : Redraw a vertical bar control and its associated text
// Parameters  : 
// Returns     : NONE.
// NOTE        : Currently unused, remove?
//---------------------------------------------------------------------------
void RedrawVertBarTextS8(TTrackBar *bar, TEdit *text, SInt8 param)
{
  bar->Position=param;
  bar->Hint=AnsiString(param);
  text->Text=AnsiString(param);
}

void RedrawVertBarTextU8(TTrackBar *bar, TEdit *text, UInt8 param, UInt8 offset)
{
  bar->Position=bar->Max-(param+offset);
  bar->Hint=AnsiString(param+offset);
  text->Text=AnsiString(param+offset);
}

void RedrawVertBarTextU16(TTrackBar *bar, TEdit *text, UInt16 param, UInt16 offset)
{
  bar->Position=bar->Max-(param+offset);
  bar->Hint=AnsiString(param+offset);
  text->Text=AnsiString(param+offset);
}

//---------------------------------------------------------------------------
// Name        : EqBarChange
// Description : Update the parameter, hint text and text field associated 
//               with a vertical Eq frequency bar.
// Parameter 1 : Pointer to bar object
// Parameter 2 : Pointer to text object for numeric value
// Parameter 3 : Pointer to parameter to update
// Returns     : NONE.
//---------------------------------------------------------------------------
void EqBarChange(TTrackBar *bar, TEdit *text, UInt16* param)
{
  *param= -1 * bar->Position + bar->Max;       // Convert to range: 0 to 560
  bar->Hint=AnsiString(bar->Position/-20.0);   // Display in range: -14 to 14
  text->Text=AnsiString(bar->Position/-20.0);  // Display value as text
}

void VertBarChange(TTrackBar *bar, UInt8* param)
{
  *param=(bar->Max - bar->Position);
  bar->Hint=AnsiString(bar->Max - bar->Position);
}

void VertBarChangeS8(TTrackBar *bar, TEdit *text, SInt8* param)
{
  *param=-1*bar->Position;
  bar->Hint=AnsiString(*param);
  if (text != NULL)
  {
    text->Text=AnsiString(*param);
  }
}
void VertBarChangeU8(TTrackBar *bar, TEdit *text, UInt8* param)
{
  *param=(bar->Max - bar->Position);
  bar->Hint=AnsiString(*param);
  if (text != NULL)
  {
    text->Text=AnsiString(*param);
  }
}

void VertBarChangeU16(TTrackBar *bar, TEdit *text, UInt16* param)
{
  *param=(bar->Max - bar->Position);
  bar->Hint=AnsiString(*param);
  if (text != NULL)
  {
    text->Text=AnsiString(*param);
  }
}
void VertBarChangeOffsetU8(TTrackBar *bar, TEdit *text, UInt8* param, UInt8 offset)
{
  *param=(bar->Max - bar->Position);
  bar->Hint=AnsiString(*param+offset);
  if (text != NULL)
  {
    text->Text=AnsiString(*param+offset);
  }
}

void HorizBarChange(TTrackBar *bar, TEdit *text, UInt16* param)
{
  *param= bar->Position;
  bar->Hint=AnsiString(*param);
  if (text != NULL)
  {
    text->Text=AnsiString(*param);
  }
}

void HorizBarChangeU8(TTrackBar *bar, TEdit *text, UInt8* param)
{
  *param= bar->Position;
  bar->Hint=AnsiString(*param);
  if (text != NULL)
  {
    text->Text=AnsiString(*param);
  }
}

void HorizBarChangeS8(TTrackBar *bar, TEdit *text, SInt8* param)
{
  *param= bar->Position;
  bar->Hint=AnsiString(*param);
  if (text != NULL)
  {
    text->Text=AnsiString(*param);
  }
}
