//---------------------------------------------------------------------------
// Literals
//---------------------------------------------------------------------------
#define BIT0         (0x01)
#define BIT1         (0x02)
#define BIT2         (0x04)
#define BIT3         (0x08)
#define BIT4         (0x10)
#define BIT5         (0x20)
#define BIT6         (0x40)
#define BIT7         (0x80)

// Two bits
#define BITS0to1     (0x03)
#define BITS1to2     (0x06)
#define BITS2to3     (0x0C)
#define BITS3to4     (0x18)
#define BITS4to5     (0x30)
#define BITS5to6     (0x60)
#define BITS6to7     (0xC0)

// Three bits
#define BITS0to2     (0x07)
#define BITS1to3     (0x0E)
#define BITS2to4     (0x1C)
#define BITS3to5     (0x38)
#define BITS4to6     (0x70)
#define BITS5to7     (0xE0)

// Four bits
#define BITS0to3     (0x0F)
#define BITS1to4     (0x1E)
#define BITS2to5     (0x3C)
#define BITS3to6     (0x78)
#define BITS4to7     (0xF0)

// Five bits
#define BITS0to4     (0x1F)

// Six bits
#define BITS0to5     (0x3F)
#define BITS1to6     (0x7E)
#define BITS2to7     (0xFC)

// Seven bits
#define BITS0to6     (0x7F)
#define BITS1to7     (0xFE)

#define QUAD_PATCH_SIZE   (128)   // Number of bytes per patch in QuadGT (8 bit) format
#define QUAD_NUM_PATCH    (100)

#define SYSEX_PATCH_SIZE   (147)   // Number of bytes per patch in SysEx (7 bit) format

#define NUM_MOD           (8)
#define NUM_RES           (5)

// Possible CONFIG values
#define CFG0_EQ_PITCH_DELAY_REVERB (0)         // Quad Mode
#define CFG1_LESLIE_DELAY_REVERB (1)
#define CFG2_GEQ_DELAY (2)
#define CFG3_5BANDEQ_PITCH_DELAY (3)
#define CFG4_3BANDEQ_REVERB (4)
#define CFG5_RINGMOD_DELAY_REVERB (5)
#define CFG6_RESONATOR_DELAY_REVERB (6)
#define CFG7_SAMPLING (7)

// Possible EQ_MODE values
typedef enum 
{
  EQMODE0_EQ,
  EQMODE1_EQ_PLUS_RESONATOR
} tEq_Mode;

// Possible DELAY_MODE values
typedef enum
{
  DELAYMODE0_MONO,
  DELAYMODE1_STEREO,
  DELAYMODE2_PINGPONG,
  DELAYMODE3_MULTITAP
} tDelay_Mode;

// Possible PITCH_MODE values
typedef enum
{
  PITCHMODE0_MONO_CHORUS,
  PITCHMODE1_STEREO_CHORUS,
  PITCHMODE2_MONO_FLANGE,
  PITCHMODE3_STEREO_FLANGE,
  PITCHMODE4_DETUNE,
  PITCHMODE5_PHASER
} tPitch_Mode;


#define LOW_EQ_FREQ_IDX                 (0x00) // LOW EQ FREQ, 16bits
#define RES1_TUNE_IDX                   (0x00) // RES 1 TUNE
#define RES1_DECAY_IDX                  (0x01) // RES 1 DECAY
#define GEQ_16HZ_IDX                    (0x01) // 16Hz
#define LOW_EQ_AMP_IDX                  (0x02) // LOW EQ AMP , 16bits
#define RES1_AMP_IDX                    (0x02) // RES 1 AMP
#define GEQ_32HZ_IDX                    (0x02) // 32Hz
#define RES2_TUNE_IDX                   (0x03) // RES 2 TUNE
#define GEQ_62HZ_IDX                    (0x03) // 62Hz
#define MID_EQ_FREQ_IDX                 (0x04) // MID EQ FREQ, 16bits
#define GEQ_126HZ_IDX                   (0x04) // 126Hz
#define GEQ_250HZ_IDX                   (0x05) // 250Hz
#define MID_EQ_BW_IDX                   (0x06) // MID EQ BANDWIDTH
#define GEQ_500HZ_IDX                   (0x06) // 500Hz
#define MID_EQ_AMP_IDX                  (0x07) // MID EQ AMP, 16bits
#define GEQ_1KHZ_IDX                    (0x07) // 1KHz
#define GEQ_2KHZ_IDX                    (0x08) // 2KHz
#define HIGH_EQ_FREQ_IDX                (0x09) // HIGH EQ FREQ , 16bits
#define RES2_DECAY_IDX                  (0x09) // RES 2 DECAY
#define GEQ_4KHZ_IDX                    (0x09) // 4KHz
#define RES2_AMP_IDX                    (0x0A) // RES 2 AMP
#define GEQ_8KHZ_IDX                    (0x0A) // 8KHz
#define HIGH_EQ_AMP_IDX                 (0x0B) // HIGH EQ AMP
#define GEQ_16KHZ_IDX                   (0x0B) // 16KHz
#define LESLIE_HIGH_ROTOR_LEVEL_IDX     (0x0D) // LESLIE HIGH ROTOR LEVEL
#define TAP1_DELAY_MSB_IDX              (0x0D) // TAP 1 DELAY MSB NOTE: Split value
#define LOW_MID_EQ_FREQ_IDX             (0x0E) // LOW MID EQ FREQ, 16bits
#define RES1_TUNE_IDX_B                 (0x0E) // RES 1 TUNE - Alternative
#define RES1_DECAY_IDX_B                (0x0F) // RES 1 DECAY - Alternative
#define LOW_MID_EQ_BW_IDX               (0x10) // LOW MID EQ BANDWIDTH
#define RES2_TUNE_IDX_B                 (0x10) // RES 2 TUNE - Alternative
#define LOW_MID_EQ_AMP_IDX              (0x11) // LOW MID EQ AMP, 16bits
#define RES2_DECAY_IDX_B                (0x11) // RES 2 DECAY - Alternative
#define RES3_TUNE_IDX                   (0x12) // RES 3 TUNE
#define HIGH_MID_EQ_FREQ_IDX            (0x13) // HIGH MID EQ FREQ, 16bits
#define RES3_DECAY_IDX                  (0x13) // RES 3 DECAY
#define RES4_TUNE_IDX                   (0x14) // RES 4 TUNE
#define HIGH_MID_EQ_BW_IDX              (0x15) // HIGH MID EQ BANDWIDTH
#define RES4_DECAY_IDX                  (0x15) // RES 4 DECAY
#define HIGH_MID_EQ_AMP_IDX             (0x16) // HIGH MID EQ AMP, 16bits
#define RES5_TUNE_IDX                   (0x16) // RES 5 TUNE
#define RES5_DECAY_IDX                  (0x17) // RES 5 DECAY
#define SAMPLE_START_IDX                (0x18) // SAMPLE START
#define TAP1_DELAY_LSB_IDX              (0x18) // TAP 1 DELAY LSB
#define SAMPLE_LENGTH_IDX               (0x19) // SAMPLE LENGTH
#define TAP1_VOLUME_IDX                 (0x19) // TAP 1 VOLUME
#define PITCH_MODE_IDX                  (0x1A) // PITCH MODE
#define PITCH_INPUT_IDX                 (0x1B) // PITCH INPUT (0)
#define RES4_AMP_IDX                    (0x1B) // RES 4 AMP (1-7)
#define LFO_WAVEFORM_IDX                (0x1C) // LFO WAVEFORM (0)
#define EQ_PRESET_IDX                   (0x1C) // EQ PRESET NUMBER (1-7)
#define LFO_SPEED_IDX                   (0x1D) // LFO SPEED
#define LFO_DEPTH_IDX                   (0x1E) // LFO DEPTH
#define RES_DECAY_IDX                   (0x1E) // RES DECAY
#define SAMPLE_PLAYBACK_MODE_IDX        (0x1F) // SAMPLE PLAYBACK MODE
#define TAP1_PAN_IDX                    (0x1F) // TAP 1 PAN
#define PITCH_FEEDBACK_IDX              (0x20) // PITCH FEEDBACK
#define DETUNE_AMOUNT_IDX               (0x21) // DETUNE AMOUNT
#define SAMPLE_PITCH_IDX                (0x21) // SAMPLE PITCH
#define LESLIE_SEPARATION_IDX           (0x22) // LESLIE SEPARATION
#define TAP1_FEEDBACK_IDX               (0x22) // TAP 1 FEEDBACK
#define LESLIE_MOTOR_IDX                (0x23) // LESLIE MOTOR
#define SAMPLE_REC_AUDIO_TRIG_IDX       (0x23) // SAMPLE REC AUDIO TRIG
#define TAP2_DELAY_IDX                  (0x23) // TAP 2 DELAY, 16bits
#define LESLIE_SPEED_IDX                (0x24) // LESLIE SPEED
#define SAMPLE_MIDI_TRIG_IDX            (0x24) // SAMPLE MIDI TRIG
#define TRIGGER_FLANGE_IDX              (0x25) // TRIGGER FLANGE
#define RES_MIDI_GATE_IDX               (0x25) // RES MIDI GATE
#define SAMPLE_MIDI_BASE_NOTE_IDX       (0x26) // SAMPLE MIDI BASE NOTE
#define TAP2_VOLUME_IDX                 (0x26) // TAP 2 VOLUME
#define DELAY_MODE_IDX                  (0x27) // DELAY MODE
#define DELAY_INPUT_IDX                 (0x28) // DELAY INPUT (0)
#define RES5_AMP_IDX                    (0x28) // RES 5 AMP (1-7)
#define DELAY_INPUT_MIX_IDX             (0x29) // DELAY INPUT MIX
#define DELAY_LEFT_IDX                  (0x2A) // LEFT DELAY, 16bits  (also used for MONO delays)
#define TAP2_PAN_IDX                    (0x2A) // TAP 2 PAN
#define TAP2_FEEDBACK_IDX               (0x2B) // TAP 2 FEEDBACK
#define DELAY_LEFT_FEEDBACK_IDX         (0x2C) // LEFT FEEDBACK  (also used for MONO delays)
#define TAP3_DELAY_IDX                  (0x2C) // TAP 3 DELAY, 16bits
#define DELAY_RIGHT_IDX                 (0x2D) // RIGHT DELAY, 16bits
#define TAP3_VOLUME_IDX                 (0x2E) // TAP 3 VOLUME
#define DELAY_RIGHT_FEEDBACK_IDX        (0x2F) // RIGHT FEEDBACK
#define TAP3_PAN_IDX                    (0x2F) // TAP 3 PAN
#define SAMPLE_LOW_MIDI_NOTE_IDX        (0x30) // SAMPLE LOW MIDI NOTE
#define TAP3_FEEDBACK_IDX               (0x30) // TAP 3 FEEDBACK
#define SAMPLE_HIGH_MIDI_NOTE_IDX       (0x31) // SAMPLE HIGH MIDI NOTE
#define TAP4_DELAY_IDX                  (0x31) // TAP 4 DELAY, 16bits
#define REVERB_MODE_IDX                 (0x32) // REVERB MODE
#define TAP4_VOLUME_IDX                 (0x33) // TAP 4 VOLUME
#define REVERB_INPUT_1_IDX              (0x34) // REVERB INPUT 1
#define TAP4_PAN_IDX                    (0x34) // TAP 4 PAN
#define REVERB_INPUT_2_IDX              (0x35) // REVERB INPUT 2
#define TAP4_FEEDBACK_IDX               (0x35) // TAP 4 FEEDBACK
#define REVERB_INPUT_MIX_IDX            (0x36) // REVERB INPUT MIX
#define TAP5_DELAY_IDX                  (0x36) // TAP 5 DELAY, 16bits
#define REVERB_PREDELAY_IDX             (0x37) // REVERB PREDELAY
#define REVERB_PREDELAY_MIX_IDX         (0x38) // REVERB PREDELAY MIX
#define TAP5_VOLUME_IDX                 (0x38) // TAP 5 VOLUME
#define REVERB_DECAY_IDX                (0x39) // REVERB DECAY
#define TAP5_PAN_IDX                    (0x39) // TAP 5 PAN
#define REVERB_DIFFUSION_IDX            (0x3A) // REVERB DIFFUSION
#define TAP5_FEEDBACK_IDX               (0x3A) // TAP 5 FEEDBACK
#define REVERB_LOW_DECAY_IDX            (0x3B) // REVERB LOW DECAY
#define TAP6_DELAY_IDX                  (0x3B) // TAP 6 DELAY, 16bits
#define REVERB_HIGH_DECAY_IDX           (0x3C) // REVERB HIGH DECAY
#define REVERB_DENSITY_IDX              (0x3D) // REVERB DENSITY
#define TAP6_VOLUME_IDX                 (0x3D) // TAP 6 VOLUME
#define REVERB_GATE_IDX                 (0x3E) // REVERB GATE
#define TAP6_PAN_IDX                    (0x3E) // TAP 6 PAN
#define REVERB_GATE_HOLD_IDX            (0x3F) // REVERB GATE HOLD
#define TAP6_FEEDBACK_IDX               (0x3F) // TAP 6 FEEDBACK
#define REVERB_GATE_RELEASE_IDX         (0x40) // REVERB GATE RELEASE
#define TAP7_DELAY_IDX                  (0x40) // TAP 7 DELAY, 16bits
#define REVERB_GATED_LEVEL_IDX          (0x41) // REVERB GATED LEVEL
#define TAP7_DELAY_LSB_IDX              (0x41) // TAP 7 DELAY LSB
#define RING_MOD_SHIFT_IDX              (0x42) // RING MOD SHIFT, 16bits
#define TAP7_VOLUME_IDX                 (0x42) // TAP 7 VOLUME
#define TAP7_PAN_IDX                    (0x43) // TAP 7 PAN
#define CONFIG_IDX                      (0x44)
#define PREPOST_EQ_IDX                  (0x45) // PRE-POST EQ (0)
#define DIRECT_LEVEL_IDX                (0x45) // DIRECT LEVEL (1-7)
#define PREAMP_LEVEL_IDX                (0x46) // PREAMP LEVEL
#define EQ_LEVEL_IDX                    (0x46) // EQ LEVEL
#define MASTER_EFFECTS_LEVEL_IDX        (0x47) // MASTER EFFECTS LEVEL
#define PITCH_LEVEL_IDX                 (0x48) // PITCH LEVEL
#define LESLIE_LEVEL_IDX                (0x48) // LESLIE LEVEL
#define RING_MOD_LEVEL_IDX              (0x48) // RING MOD LEVEL
#define DELAY_LEVEL_IDX                 (0x49) // DELAY LEVEL
#define REVERB_LEVEL_IDX                (0x4A) // REVERB LEVEL
#define RES1_AMP_IDX_B                  (0x4A) // RES 1 AMP - Alternative
#define RES1_PITCH_IDX                  (0x4B) // RES PITCH 1
#define TAP8_DELAY_IDX                  (0x4B) // TAP 8 DELAY, 16bits
#define RES2_PITCH_IDX                  (0x4C) // RES PITCH 2
#define RES3_PITCH_IDX                  (0x4D) // RES PITCH 3
#define TAP8_VOLUME_IDX                 (0x4D) // TAP 8 VOLUME
#define RES4_PITCH_IDX                  (0x4E) // RES PITCH 4
#define TAP8_PAN_IDX                    (0x4E) // TAP 8 PAN
#define RES5_PITCH_IDX                  (0x4F) // RES PITCH 5
#define TAP8_FEEDBACK_IDX               (0x4F) // TAP 8 FEEDBACK

// Modulation parameters
#define MOD1_SOURCE_IDX                 (0x50) // MOD 1 SOURCE
#define MOD1_TARGET_IDX                 (0x51) // MOD 1 TARGET
#define MOD1_AMP_IDX                    (0x52) // MOD 1 AMPLITUDE
#define MOD2_SOURCE_IDX                 (0x53) // MOD 2 SOURCE
#define MOD2_TARGET_IDX                 (0x54) // MOD 2 TARGET
#define MOD2_AMP_IDX                    (0x55) // MOD 2 AMPLITUDE
#define MOD3_SOURCE_IDX                 (0x56) // MOD 3 SOURCE
#define MOD3_TARGET_IDX                 (0x57) // MOD 3 TARGET
#define MOD3_AMP_IDX                    (0x58) // MOD 3 AMPLITUDE
#define MOD4_SOURCE_IDX                 (0x59) // MOD 4 SOURCE
#define MOD4_TARGET_IDX                 (0x5A) // MOD 4 TARGET
#define MOD4_AMP_IDX                    (0x5B) // MOD 4 AMPLITUDE
#define MOD5_SOURCE_IDX                 (0x5C) // MOD 5 SOURCE
#define MOD5_TARGET_IDX                 (0x5D) // MOD 5 TARGET
#define MOD5_AMP_IDX                    (0x5E) // MOD 5 AMPLITUDE
#define MOD6_SOURCE_IDX                 (0x5F) // MOD 6 SOURCE
#define MOD6_TARGET_IDX                 (0x60) // MOD 6 TARGET
#define MOD6_AMP_IDX                    (0x61) // MOD 6 AMPLITUDE
#define MOD7_SOURCE_IDX                 (0x62) // MOD 7 SOURCE
#define MOD7_TARGET_IDX                 (0x63) // MOD 7 TARGET
#define MOD7_AMP_IDX                    (0x64) // MOD 7 AMPLITUDE
#define MOD8_SOURCE_IDX                 (0x65) // MOD 8 SOURCE
#define MOD8_TARGET_IDX                 (0x66) // MOD 8 TARGET
#define MOD8_AMP_IDX                    (0x67) // MOD 8 AMPLITUDE
#define MULTITAP_MASTER_FEEDBACK_IDX    (0x68) // MULTITAP MASTER FEEDBACK
#define MULTITAP_NUMBER_IDX             (0x69) // MULTITAP NUMBER
#define NAME_IDX                        (0x6A) // 1ST DIGIT NAME

// Ring modulator parameters
#define RING_MOD_OUTPUT_MIX_IDX         (0x78) // RING MOD OUTPUT MIX
#define RES2_AMP_IDX_B                  (0x78) // RES 2 AMP - Alternative
#define RING_MOD_DEL_REV_MIX_IDX        (0x79) // RING MOD DEL/REV MIX
#define RES3_AMP_IDX                    (0x79) // RES 3 AMP

#define PAN_SPEED_IDX                   (0x7A) // PAN SPEED
#define PAN_DEPTH_IDX                   (0x7B) // PAN DEPTH
#define EQ_MODE_IDX                     (0x7C) // EQ-MODE (7)

// Preamp parameters
#define PREAMP_COMP_IDX                 (0x7C) // COMPRESSION (4-6)
#define PREAMP_DIST_IDX                 (0x7C) // DISTORTION (3-0)
#define MIX_MOD_IDX                     (0x7D) // MIX MODULATION (6-7)
#define EFFECT_LOOP_IDX                 (0x7D) // EFFECT LOOP (5)
#define BASS_BOOST_IDX                  (0x7D) // BASS BOOST (4)
#define PREAMP_TONE_IDX                 (0x7D) // PREAMP TONE (3-2)
#define CAB_SIM_IDX                     (0x7D) // CAB (1-0)
#define PREAMP_OD_IDX                   (0x7E) // OVERDRIVE (7-5)
#define PREAMP_GATE_IDX                 (0x7E) // NOISE GATE (4-0)
#define PREAMP_OUT_LEVEL_IDX            (0x7F) // PREAMP OUTPUT LEVEL

#define NAME_LENGTH       (14)

#define EDIT_BUFFER   (100)
#define ALL_PROGS     (101)

//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------

// Internal data structure for Quadraverb GT parameters
typedef struct
{
  char  name[NAME_LENGTH+1];
  UInt8 config;                    // 0-7

  // Preamp parameters
  UInt8 comp;                      // Compression 0-7
  UInt8 od;                        // Overdrive 0-7
  UInt8 dist;                      // Distortion 0-8
  UInt8 preamp_tone;               // Preamp tone 0-2 (Flat, Prescence, Bright)
  UInt8 preamp_gate;               // Preamp noise gate level 0-17
  UInt8 preamp_out_level;          // Preamp output level 0-99
  UInt8 effect_loop;               // Effect loop on/off 0-1
  UInt8 bass_boost;                // Bass boost on/off 0-1
  UInt8 cab_sim;                   // Cabinet simulator 0-2 (Off, Cab 1, Cab 2)

  // Eq parameters
  UInt8 eq_mode;                   // 0=Eq,  1=Eq+Resonator
  UInt16 low_eq_freq;              // 20 to 999 Hz
  UInt16 low_eq_amp;               // -14 to 14 db  (0-560)
  UInt16 mid_eq_freq;              // 200 to 9999 Hz
  UInt16 mid_eq_amp;               // -14 to 14 db  (0-560)
  UInt8  mid_eq_q;                 // 0.2 to 2.55 octaves (20-255)
  UInt16 high_eq_freq;             // 2000 to 18000 Hz
  UInt16 high_eq_amp;              // -14 to 14 db  (0-560)
  UInt16 low_mid_eq_freq;          // 20 to 500 Hz
  UInt16 low_mid_eq_amp;           // -14 to 14 db  (0-560)
  UInt8  low_mid_eq_q;             // 0.2 to 2.55 octaves (20-255)
  UInt16 high_mid_eq_freq;         // 2000 to 18000 Hz
  UInt16 high_mid_eq_amp;          // -14 to 14 db  (0-560)
  UInt8  high_mid_eq_q;            // 0.2 to 2.55 octaves (20-255)
  UInt8 eq_preset;                 // 0=6 (User=0 or preset 1-6)

  // Graphic Eq parameters
  SInt8 geq_16hz;                  // -14 to 14  (0-28)
  SInt8 geq_32hz;                  // -14 to 14  (0-28)
  SInt8 geq_62hz;                  // -14 to 14  (0-28)
  SInt8 geq_126hz;                 // -14 to 14  (0-28)
  SInt8 geq_250hz;                 // -14 to 14  (0-28)
  SInt8 geq_500hz;                 // -14 to 14  (0-28)
  SInt8 geq_1khz;                  // -14 to 14  (0-28)
  SInt8 geq_2khz;                  // -14 to 14  (0-28)
  SInt8 geq_4khz;                  // -14 to 14  (0-28)
  SInt8 geq_8khz;                  // -14 to 14  (0-28)
  SInt8 geq_16khz;                 // -14 to 14  (0-28)

  // Pitch parameters
  UInt8 pitch_mode;                          // 0 - 5 (tPitch_Mode)
  UInt8 pitch_input;                         // 0 - 1
  UInt8 lfo_waveform;                        // 0 - 1
  UInt8 lfo_speed;                           // 0 - 98
  UInt8 lfo_depth;                           // 0 - 98
  UInt8 pitch_feedback;                      // 0 - 99
  UInt8 trigger_flange;                      // 0 - 1
  UInt8 detune_amount;                       // -99 - 99

  // Delay parameters
  UInt8  delay_mode;                         // 0 - 3  (tDelay_Mode)
  UInt8  delay_input;                        // 0 - 1
  SInt8  delay_input_mix;                    // -99 - 99
  UInt16 delay_left;                         // 1 - 375/775/800/1500/1470/705/400/320/720/750 Also used for mono
  UInt8  delay_left_feedback;                // 0 - 99, Also used for mono
  UInt16 delay_right;                        // 1 - 375/400/750/705/320 
  UInt8  delay_right_feedback;               // 0 - 99

  // Multi tap delay parameters
  UInt8  tap_delay[8];
  UInt8  tap_volume[8];
  UInt8  tap_pan[8];
  UInt8  tap_feedback[8];
  UInt8  master_feedback;

  // Reverb parameters
  UInt8 reverb_mode;
  UInt8 reverb_input_1;                      // 0 - 3
  UInt8 reverb_input_2;                      // 0 - 1
  SInt8 reverb_input_mix;                    // -99 - 99
  UInt8 reverb_predelay;                     // 1 - 140
  SInt8 reverb_predelay_mix;                 // -99 - 99
  UInt8 reverb_decay;                        // 0 - 99
  UInt8 reverb_diffusion;                    // 0 - 8
  UInt8 reverb_low_decay;                    // 0 - 60
  UInt8 reverb_high_decay;                   // 0 - 60
  UInt8 reverb_density;                      // 0 - 8
  UInt8 reverb_gate;                         // 0 - 1
  UInt8 reverb_gate_hold;                    // 0 - 99
  UInt8 reverb_gate_release;                 // 0 - 99
  UInt8 reverb_gated_level;                  // 0 - 99

  // Mix parameters
  UInt8 prepost_eq;                  // 0 - 1 (0=Pre-eq, 1=Post-eq)
  UInt8 direct_level;                // 0 - 99
  UInt8 master_effects_level;        // 0 - 99
  UInt8 preamp_level;                // 0 - 99
  UInt8 pitch_level;                 // 0 - 99
  UInt8 leslie_level;                // 0 - 99
  UInt8 ring_mod_level;              // 0 - 99
  UInt8 delay_level;                 // 0 - 99
  UInt8 reverb_level;                // 0 - 99
  UInt8 mix_mod;                     // 0 - 1
  UInt8 eq_level;

  UInt8 multitap_master_feedback;
  UInt8 multitap_number;

  // Midi modulation parameters
  UInt8 mod_source[NUM_MOD];      // 0 - 126
  UInt8 mod_target[NUM_MOD];    // 0 - ???
  SInt8 mod_amp[NUM_MOD];       // -99 - 99

  // Resonator parameters
  UInt8 res_tune[NUM_RES];      // 0 - 60
  UInt8 res_amp[NUM_RES];       // 0 - 99
  UInt8 res_decay[NUM_RES];     // 0 - 99
  UInt8 res_pitch[NUM_RES];
  UInt8 res_decay_all;
  UInt8 res_midi_gate;          // 0 - 1

  // Sampling parameters
  UInt8 sample_start;            // 0 - 150
  UInt8 sample_length;           // 5 - 155
  UInt8 sample_playback_mode;    // 0 - 2
  UInt8 sample_pitch;
  UInt8 sample_rec_audio_trigger; // 0 - 1
  UInt8 sample_midi_trigger;      // 0 - 2
  UInt8 sample_midi_base_note;    // 0 - 127
  UInt8 sample_low_midi_note;     // 0 - 127
  UInt8 sample_high_midi_note;    // 0 - 127

  // Ring modulator parameters
  SInt8 ring_mod_out_mix;  // -99-99
  SInt8 ring_mod_in_mix;   // -99-99
  UInt16 ring_mod_shift;   // 1-300

  UInt8 pan_speed;         // 0 - 98 (Mod Speed?)
  UInt8 pan_depth;         // 0 - 99 (Mod Depth?)
 
  // Leslie parameters (For Config = 1 - Leslie, Delay, Reverb)
  UInt8 leslie_speed;      // 0-1
  UInt8 leslie_motor;      // 0-1
  UInt8 leslie_seperation; // 0-99
  UInt8 leslie_high_rotor_level;  // 0 - 26

} tQuadGT_Prog;

//---------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------
const UInt8 Sysex_Alesis[]={0x00,0x00,0x0E};   // Manufacturer Id: Alesis

const UInt8 Sysex_Quad[]={0x02};               // Device Id: Quadraverb
const UInt8 Sysex_QuadGT[]={0x07};             // Device Id: Quadraverb GT

const UInt8 Sysex_Edit[]={0x01};               // Command to edit a Quadraverb function
const UInt8 Sysex_Data_Dump[]={0x02};          // Command to send a program data dump
const UInt8 Sysex_Dump_Req[]={0x03};           // Command to request a dump

//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------
UInt32 QuadGT_Decode_From_Sysex(UInt8 *in, UInt32 length, UInt8* out, UInt32 out_len);
UInt32 QuadGT_Encode_To_Sysex(UInt8 *in, UInt32 length, UInt8 * out, UInt32 out_len);

UInt32 QuadGT_Convert_QuadGT_To_Internal(UInt8 prog, UInt8* data);
UInt32 QuadGT_Convert_Data_From_Internal(UInt8 prog, UInt8* data);

void QuadGT_Sysex_Process(tBuffer sysex);

void QuadGT_Init(void);
void QuadGT_Redraw_Patch(UInt8 program);
void QuadGT_Redraw_Reverb(const UInt8 program);
void QuadGT_Redraw_Delay(const UInt8 program);
void QuadGT_Redraw_Pitch(const UInt8 program);
void QuadGT_Redraw_Eq(const UInt8 program);
void QuadGT_Redraw_PrePostEq(const UInt8 prog);
void QuadGT_Redraw_Mix(const UInt8 program);
void QuadGT_Redraw_Mod(const UInt8 program);
void QuadGT_Redraw_Preamp(const UInt8 program);
void QuadGT_Redraw_Resonator(const UInt8 program);
void QuadGT_Param_Change(TObject * Sender);
UInt16 QuadGT_Decode_16Bit(UInt8 *data);
UInt16 QuadGT_Decode_16Bit_Split(UInt8 *msb, UInt8 *lsb);
void QuadGT_Encode_16Bit(const UInt16 word, UInt8 *data);
void QuadGT_Encode_16Bit_Split(const UInt16 word, UInt8 *msb, UInt8 *lsb);
void QuadGT_Swap(const int patch1, const int patch2);
