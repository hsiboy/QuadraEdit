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
#include <Mask.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
    TMainMenu *MainMenu1;
    TMenuItem *MenuFile;
    TMenuItem *MenuFileExit;
    TMenuItem *MenuHelp;
    TMenuItem *MenuHelpAbout;
    TPanel *PanelQuad;
    TGroupBox *QuadReverb;
    TGroupBox *PanelQuadDelay;
    TGroupBox *QuadPitch;
    TGroupBox *PanelQuadEq3;
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
    TTimer *TimerMidiCounts;
    TGroupBox *PanelQuadMix;
    TGroupBox *PanelQuadMod;
    TGroupBox *PanelQuadPreamp;
    TRadioGroup *ReverbType;
    TRadioGroup *DelayMode;
    TRadioGroup *PitchMode;
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
    TTrackBar *PreComp;
    TTrackBar *PreOd;
    TTrackBar *PreDist;
    TTrackBar *PreGate;
    TTrackBar *PreOutLevel;

    TTimer *TimerProcess;
    TButton *QuadProgWrite;
    TButton *QuadMidiWrite;
    TComboBox *ModTarget;
    TTrackBar *ModAmp;
    TPanel *Panel1;
    TRadioGroup *PreTone;
    TLabel *Direct;
    TLabel *Master;
    TLabel *PreAmpEq;
    TLabel *Pitch;
    TLabel *Delay;
    TLabel *Reverb;
    TLabel *Depth;
    TLabel *Speed;
    TGroupBox *QuadEq5;
    TTrackBar *Eq5Amp1;
    TTrackBar *Eq5Amp2;
    TTrackBar *Eq5Freq1;
    TTrackBar *Eq5Freq2;
    TTrackBar *Eq5Q2;
    TTrackBar *Eq5Freq3;
    TTrackBar *Eq5Amp3;
    TTrackBar *Eq5Q3;
    TTrackBar *Eq5Freq4;
    TTrackBar *Eq5Amp4;
    TTrackBar *Eq5Q4;
    TTrackBar *Eq5Freq5;
    TTrackBar *Eq5Amp5;
    TLabel *Comp;
    TLabel *OD;
    TLabel *Dist;
    TLabel *LeslieRingModReson;
    TLabel *Gate;
    TLabel *PreOutLabel;
    TGroupBox *Bass;
    TCheckBox *BassBoost;
    TRadioGroup *CabSim;
    TGroupBox *EffectLoop;
    TCheckBox *EffectLoopIn;
    TRadioGroup *ReverbInput1;
    TRadioGroup *ReverbInput2;
    TTrackBar *ReverbInMix;
    TLabel *Label1;
    TTrackBar *ReverbPreDelay;
    TLabel *Label2;
    TTrackBar *ReverbPreMix;
    TLabel *Label3;
    TTrackBar *ReverbDecay;
    TLabel *Label4;
    TTrackBar *ReverbDiffusion;
    TLabel *Label5;
    TTrackBar *ReverbDensity;
    TLabel *Label6;
    TRadioGroup *QuadConfig;
    TGroupBox *ReverbGate;
    TCheckBox *ReverbGateOn;
    TTrackBar *ReverbGateHold;
    TTrackBar *ReverbGateRelease;
    TLabel *Hold;
    TLabel *Release;
    TTrackBar *ReverbGateLevel;
    TLabel *Level;
    TTrackBar *ReverbLowDecay;
    TTrackBar *ReverbHighDecay;
    TLabel *Label7;
    TLabel *Label8;
    TUpDown *UpDown1;
    TMaskEdit *ModNumber;
    TLabel *Label9;
    TRadioGroup *DelayInput;
    TTrackBar *DelayInMix;
    TLabel *Label10;
    TTrackBar *DelayDelay;
    TTrackBar *DelayFeedback;
    TLabel *Label11;
    TLabel *Label12;
    TTrackBar *DelayRight;
    TLabel *Label13;
    TTrackBar *DelayRightFeedback;
    TLabel *Label14;
    TGroupBox *DelayTap;
    TLabel *Label15;
    TLabel *Label16;
    TLabel *Label17;
    TTrackBar *TapDelay;
    TTrackBar *TapVol;
    TTrackBar *TapPan;
    TLabel *Label18;
    TMaskEdit *TapNumber;
    TUpDown *UpDown2;
    TTrackBar *TapFeedback;
    TLabel *Label19;
    TRadioGroup *PitchInput;
    TRadioGroup *PitchWave;
    TTrackBar *PitchSpeed;
    TTrackBar *PitchDepth;
    TTrackBar *PitchFeedback;
    TLabel *Label20;
    TLabel *Label21;
    TLabel *Label22;
    TGroupBox *QuadLeslie;
    TRadioGroup *LeslieWave;
    TTrackBar *LeslieSpeed;
    TTrackBar *LeslieDepth;
    TLabel *Label23;
    TLabel *Label24;
    TGroupBox *QuadRingMod;
    TTrackBar *TrackBar25;
    TLabel *Label25;
    TTrackBar *TrackBar26;
    TLabel *Label26;
    TTrackBar *TrackBar27;
    TLabel *Label27;
    TMaskEdit *QuadPatchNum;
    TGroupBox *QuadGraphEq;
    TTrackBar *GEQ_16Hz;
    TTrackBar *GEQ_32Hz;
    TTrackBar *GEQ_62Hz;
    TTrackBar *GEQ_126Hz;
    TTrackBar *GEQ_250Hz;
    TTrackBar *GEQ_500Hz;
    TLabel *Label28;
    TLabel *Label29;
    TLabel *Label30;
    TLabel *Label31;
    TLabel *Label32;
    TLabel *Label33;
    TTrackBar *GEQ_1KHz;
    TLabel *Label34;
    TTrackBar *GEQ_2KHz;
    TLabel *Label35;
    TTrackBar *GEQ_4KHz;
    TLabel *Label36;
    TTrackBar *GEQ_8KHz;
    TLabel *Label37;
    TTrackBar *GEQ_16KHz;
    TLabel *Label38;
    TRadioGroup *GEQPreset;
    TGroupBox *QuadResonator;
    TMenuItem *Device;
    TMenuItem *Open;
    TMenuItem *Close;
    TGroupBox *QuadChorus;
    TLabel *Label39;
    TLabel *Label40;
    TTrackBar *ChorusSpeed;
    TTrackBar *ChorusDepth;
    TRadioGroup *ChorusOnOff;
    TRadioGroup *ChorusWave;
    TButton *RawQuadGtBankLoad;
    TEdit *Eq5Amp1Val;
    TEdit *Eq5Amp2Val;
    TEdit *Eq5Amp3Val;
    TEdit *Eq5Amp4Val;
    TEdit *Eq5Amp5Val;
    TEdit *Eq3Amp1Val;
    TEdit *Eq3Amp2Val;
    TEdit *Eq3Amp3Val;
    TRadioGroup *Eq3Mode;
    TLabel *ResLabel;
    TMaskEdit *ResNumber;
    TUpDown *ResUpDown;
    TTrackBar *ResTune;
    TTrackBar *ResDecay;
    TLabel *Label41;
    TLabel *Label42;
    TLabel *Label43;
    TTrackBar *ResAmp;
    TRadioGroup *Eq5Mode;
    TOpenDialog *SysexOpenDialog;
    TButton *SysexBankLoad;
    TOpenDialog *QuadGtOpenDialog;
    TOpenDialog *InternalOpenDialog;
    TButton *SysexBankRead;
    TButton *SysexBankWrite;
    TEdit *Eq5Q2Val;
    TEdit *Eq5Q3Val;
    TEdit *Eq5Q4Val;
    TEdit *Eq5Freq1Val;
    TEdit *Eq5Freq2Val;
    TEdit *Eq5Freq3Val;
    TEdit *Eq5Freq4Val;
    TEdit *Eq5Freq5Val;
    TEdit *EqQ2Val;
    TEdit *PreCompVal;
    TEdit *PreOdVal;
    TEdit *PreDistVal;
    TEdit *PreGateVal;
    TEdit *PreOutVal;
    TEdit *EqFreq1Val;
    TEdit *EqFreq2Val;
    TEdit *EqFreq3Val;
    TEdit *ResTuneVal;
    TEdit *ResDecayVal;
    TEdit *ResAmpVal;
    TEdit *MixDirectVal;
    TEdit *MixMasterVal;
    TEdit *MixPreampEqVal;
    TEdit *MixPitchVal;
    TEdit *MixDelayVal;
    TEdit *MixReverbVal;
    TEdit *MixLeslieRingModResonVal;
    TEdit *ModDepthVal;
    TEdit *ModSpeedVal;
    TEdit *ModAmpVal;
    TEdit *PitchFeedbackVal;
    TEdit *PitchDepthVal;
    TEdit *PitchSpeedVal;
    TRadioButton *MixModNone;
    TRadioButton *MixModTrem;
    TRadioButton *MixModPan;
    TMaskEdit *ModSource;
    TUpDown *UpDownModSource;
    TLabel *Label44;
    TEdit *ChorusDepthVal;
    TEdit *ChorusSpeedVal;
    TEdit *ReverbInMixVal;
    TEdit *ReverbPreMixVal;
    TEdit *ReverbPreDelayVal;
    TEdit *ReverbDensityVal;
    TEdit *ReverbDiffusionVal;
    TEdit *ReverbDecayVal;
    TEdit *ReverbGateHoldVal;
    TEdit *ReverbGateReleaseVal;
    TEdit *ReverbGateLevelVal;
    TEdit *ReverbLowDecayVal;
    TEdit *ReverbHighDecayVal;
    void __fastcall MenuHelpAboutClick(TObject *Sender);
    void __fastcall UpDownQuadPatchClick(TObject *Sender,
          TUDBtnType Button);
    
    void __fastcall TimerMidiCountsTimer(TObject *Sender);
    
    void __fastcall DeviceOpenClick(TObject *Sender);
    void __fastcall MenuFileExitClick(TObject *Sender);
    void __fastcall QuadParamChange(TObject *Sender);
    void __fastcall RadioDelayClick(TObject *Sender);
    void __fastcall RadioPitchClick(TObject *Sender);
    void __fastcall RadioReverbClick(TObject *Sender);
    void __fastcall QuadPatchReadClick(TObject *Sender);
    void __fastcall DeviceCloseClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    
    void __fastcall TimerProcessTimer(TObject *Sender);
    void __fastcall QuadBankReadClick(TObject *Sender);
    void __fastcall QuadProgWriteClick(TObject *Sender);
    void __fastcall QuadMidiWriteClick(TObject *Sender);
    void __fastcall BarChange(TObject *Sender);

    void __fastcall QuadPatchNumKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);

    void __fastcall QuadPatchNumExit(TObject *Sender);
    void __fastcall QuadBankSaveClick(TObject *Sender);
    void __fastcall QuadBankLoadClick(TObject *Sender);
    void __fastcall QuadGtBankLoadClick(TObject *Sender);
    void __fastcall DelayModeClick(TObject *Sender);

    void __fastcall QuadPatchAuditionClick(TObject *Sender);
    void __fastcall VertBarCentChange(TObject *Sender);
    
    void __fastcall ResNumberKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall ResNumberExit(TObject *Sender);
    void __fastcall SysexBankLoadClick(TObject *Sender);
    
    void __fastcall QuadPatchWriteClick(TObject *Sender);
    void __fastcall SysexBankReadClick(TObject *Sender);
    void __fastcall SysexBankWriteClick(TObject *Sender);
    
    
private:	// User declarations
public:		// User declarations
    __fastcall TMainForm(TComponent* Owner);
    void __fastcall EnableIO(void);
    void __fastcall DisableIO(void);
    void __fastcall Init(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif

