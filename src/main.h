//---------------------------------------------------------------------------
#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
    TButton *ButtonPlay;
    TButton *ButtonSilence;
    TButton *ButtonInstrument;
    TEdit *EditSilenceNote;
    TEdit *EditInstrument;
    TEdit *EditPlayNote;
    TPanel *PanelDev;
    TLabel *LabelOutDevName;
    TPanel *PanelOutDev;
    TComboBox *ComboBoxOutDevs;
    TMainMenu *MainMenu1;
    TMenuItem *MenuFile;
    TMenuItem *MenuFileExit;
    TMenuItem *MenuHelp;
    TMenuItem *MenuHelpAbout;
    TPanel *PanelPlay;
    TPanel *PanelQuad;
    TPanel *PanelQuadConfig;
    TPanel *PanelQuadReverb;
    TPanel *PanelQuadDelay;
    TPanel *PanelQuadPitch;
    TPanel *PanelQuadEq;
    TEdit *QuadPatchNum;
    TUpDown *UpDownQuadPatch;
    TLabel *LabelQuadPatch;
    TButton *QuadPatchRead;
    TButton *QuadPatchWrite;
    TButton *QuadPatchLoad;
    TButton *QuadPatchSave;
    TButton *QuadBankRead;
    TButton *QuadBankWrite;
    TButton *QuadBankLoad;
    TButton *QuadBankSave;
    TEdit *EditQuadPatchName;
    TButton *ButtonMidiDevOpen;
    TLabel *LabelMidiDevError;
    TButton *ButtonMidiDevClose;
    TLabel *LabelInDevName;
    TComboBox *ComboBoxInDevs;
    TTimer *TimerMidiCounts;
    TLabel *Debug;
    TLabel *Debug2;

    TRadioGroup *RadioGroupConfig;
    TRadioButton *Config0;
    TRadioButton *Config1;
    TRadioButton *Config2;
    TRadioButton *Config3;
    TRadioButton *Config4;
    TRadioButton *Config5;
    TRadioButton *Config6;
    TRadioButton *Config7;
    TPanel *PanelQuadMix;
    TPanel *PanelQuadMod;
    TPanel *PanelQuadPreamp;
    TRadioGroup *RadioGroupReverb;
    TRadioButton *ReverbPlate;
    TRadioButton *ReverbRoom;
    TRadioButton *ReverbChamber;
    TRadioButton *ReverbHall;
    TRadioButton *ReverbReverse;
    TRadioGroup *RadioGroupDelay;
    TRadioButton *DelayMono;
    TRadioButton *DelayStereo;
    TRadioButton *DelayPingPong;
    TRadioGroup *RadioGroupPitch;
    TRadioButton *PitchMonoChorus;
    TButton *QuadPatchAudition;
    TTrackBar *EqAmp1;
    TTrackBar *EqAmp2;
    TTrackBar *EqQ1;
    TTrackBar *EqFreq1;
    TTrackBar *TrackBar1;
    TTrackBar *TrackBar2;
    TTrackBar *TrackBar3;
    TTrackBar *EqAmp3;
    TTrackBar *TrackBar5;
    TTrackBar *TrackBar6;
    TTrackBar *EqAmp4;
    TTrackBar *TrackBar8;
    TTrackBar *TrackBar9;
    TTrackBar *EqAmp5;
    TTrackBar *TrackBar11;
    TTrackBar *MixDirect;
    TTrackBar *MixMaster;
    TRadioGroup *RadioGroup1;
    TRadioButton *MixPreEq;
    TRadioButton *MixPostEq;
    TTrackBar *TrackBar4;
    TTrackBar *TrackBar7;
    TTrackBar *TrackBar10;
    TTrackBar *TrackBar12;
    TTrackBar *TrackBar13;
    TTrackBar *TrackBar14;
    TTrackBar *TrackBar15;
    TRadioButton *DelayMultiTap;
    TTrackBar *PreComp;
    TTrackBar *PreOd;
    TTrackBar *PreDist;
    TTrackBar *PreTone;
    TTrackBar *TrackBar20;
    TTrackBar *TrackBar21;
    TTrackBar *TrackBar22;
    TTrackBar *PreGate;
    TTrackBar *PreOutLevel;
    void __fastcall ButtonPlayClick(TObject *Sender);
    void __fastcall ButtonSilenceClick(TObject *Sender);
    void __fastcall ButtonInstrumentClick(TObject *Sender);
    void __fastcall MenuHelpAboutClick(TObject *Sender);
    void __fastcall QuadPatchNumChange(TObject *Sender);
    void __fastcall UpDownQuadPatchClick(TObject *Sender,
          TUDBtnType Button);
    
    void __fastcall TimerMidiCountsTimer(TObject *Sender);
    
    void __fastcall ButtonMidiDevOpenClick(TObject *Sender);
    void __fastcall MenuFileExitClick(TObject *Sender);
    void __fastcall RadioConfigClick(TObject *Sender);
    void __fastcall RadioDelayClick(TObject *Sender);
    void __fastcall RadioPitchClick(TObject *Sender);
    void __fastcall RadioReverbClick(TObject *Sender);
    void __fastcall QuadPatchReadClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
 
