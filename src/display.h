
void RedrawHorizBarTextS8(TTrackBar *bar, TEdit *text, SInt8 param);

void RedrawVertBarTextS8(TTrackBar *bar, TEdit *text, SInt8 param);
void RedrawVertBarTextU8(TTrackBar *bar, TEdit *text, UInt8 param, UInt8 offset);
void RedrawVertBarTextU16(TTrackBar *bar, TEdit *text, UInt16 param, UInt16 offset);

void EqBarChange(TTrackBar *bar, TEdit *text, UInt16* param);

void VertBarChange(TTrackBar *bar, UInt8* param);
void VertBarChangeS8(TTrackBar *bar, TEdit *text, SInt8* param);
void VertBarChangeU8(TTrackBar *bar, TEdit *text, UInt8* param);
void VertBarChangeU16(TTrackBar *bar, TEdit *text, UInt16* param);
void VertBarChangeOffsetU8(TTrackBar *bar, TEdit *text, UInt8* param, UInt8 offset);

void HorizBarChange(TTrackBar *bar, TEdit *text, UInt16* param);
void HorizBarChangeU8(TTrackBar *bar, TEdit *text, UInt8* param);
void HorizBarChangeS8(TTrackBar *bar, TEdit *text, SInt8* param);
