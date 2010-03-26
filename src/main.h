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
    TPanel *PanelQuadEq3;
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
    TTrackBar *EqFreq1;
    TTrackBar *EqFreq2;
    TTrackBar *EqQ2;
    TTrackBar *EqFreq3;
    TTrackBar *EqAmp3;
    TTrackBar *MixDirect;
    TTrackBar *MixMaster;
    TRadioGroup *PrePostEq;
    TTrackBar *MixPreampEq;
    TTrackBar *MixPitch;
    TTrackBar *MixDelay;
    TTrackBar *MixReverb;
    TTrackBar *ModDepth;
    TTrackBar *ModSpeed;
    TTrackBar *MixLeslieRingModReson;
    TRadioButton *DelayMultiTap;
    TTrackBar *PreComp;
    TTrackBar *PreOd;
    TTrackBar *PreDist;
    TTrackBar *TrackBar20;
    TTrackBar *TrackBar21;
    TTrackBar *TrackBar22;
    TTrackBar *PreGate;
    TTrackBar *PreOutLevel;

    TTimer *TimerProcess;
    TButton *QuadProgWrite;
    TButton *QuadMidiWrite;
    TButton *Test;
    TComboBox *QuadModSource1;
    TComboBox *QuadModTarget1;
    TTrackBar *QuadModAmp1;
    TPanel *Panel1;
    TCheckBox *Modulation;
    TRadioGroup *PreTone;
    TLabel *Direct;
    TLabel *Master;
    TLabel *PreAmp;
    TLabel *Pitch;
    TLabel *Delay;
    TLabel *Reverb;
    TLabel *Depth;
    TLabel *Speed;
    TPanel *PanelQuadEq5;
    TTrackBar *TrackBar1;
    TTrackBar *TrackBar2;
    TTrackBar *TrackBar4;
    TTrackBar *TrackBar5;
    TTrackBar *TrackBar6;
    TTrackBar *TrackBar7;
    TTrackBar *TrackBar8;
    TTrackBar *TrackBar9;
    TTrackBar *TrackBar10;
    TTrackBar *TrackBar11;
    TTrackBar *TrackBar12;
    TTrackBar *TrackBar13;
    TTrackBar *TrackBar14;
    TLabel *Comp;
    TLabel *OD;
    TLabel *Dist;
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
    void __fastcall ButtonMidiDevCloseClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    
    void __fastcall TimerProcessTimer(TObject *Sender);
    void __fastcall QuadBankReadClick(TObject *Sender);
    void __fastcall QuadProgWriteClick(TObject *Sender);
    void __fastcall QuadMidiWriteClick(TObject *Sender);
    void __fastcall TestClick(TObject *Sender);
    void __fastcall BarChange(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TMainForm(TComponent* Owner);
    void __fastcall Init(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif

