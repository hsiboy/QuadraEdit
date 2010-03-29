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
    TGroupBox *QuadEq5;
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
    TLabel *LeslieRingModReson;
    TLabel *Gate;
    TLabel *Out;
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
    TTrackBar *TrackBar18;
    TLabel *Label5;
    TTrackBar *TrackBar19;
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
    TTrackBar *TrackBar16;
    TTrackBar *TrackBar17;
    TTrackBar *TrackBar23;
    TLabel *Label18;
    TMaskEdit *MaskEdit1;
    TUpDown *UpDown2;
    TTrackBar *TrackBar24;
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
    TTrackBar *TrackBar28;
    TTrackBar *TrackBar29;
    TTrackBar *TrackBar30;
    TTrackBar *TrackBar31;
    TTrackBar *TrackBar32;
    TTrackBar *TrackBar33;
    TLabel *Label28;
    TLabel *Label29;
    TLabel *Label30;
    TLabel *Label31;
    TLabel *Label32;
    TLabel *Label33;
    TTrackBar *TrackBar34;
    TLabel *Label34;
    TTrackBar *TrackBar35;
    TLabel *Label35;
    TTrackBar *TrackBar36;
    TLabel *Label36;
    TTrackBar *TrackBar37;
    TLabel *Label37;
    TTrackBar *TrackBar38;
    TLabel *Label38;
    TRadioGroup *GraphicEqPreset;
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
    TButton *TEST;
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
    void __fastcall TESTClick(TObject *Sender);
    void __fastcall DelayModeClick(TObject *Sender);

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

