#define BIT0         (0x01)
#define BIT1         (0x02)
#define BIT2         (0x04)
#define BIT3         (0x08)
#define BIT4         (0x10)
#define BIT5         (0x20)
#define BIT6         (0x40)
#define BIT7         (0x80)

#define BITS0to1     (0x03)
#define BITS1to2     (0x06)
#define BITS2to3     (0x0C)
#define BITS6to7     (0xC0)

#define BITS0to2     (0x07)
#define BITS4to6     (0x70)
#define BITS5to7     (0xE0)

#define BITS0to3     (0x0F)

#define BITS0to4     (0x1F)

#define BITS1to7     (0xFE)

#define QUAD_PATCH_SIZE   (128)
#define QUAD_NUM_PATCH    (10)

#define CONFIG_IDX           (0x44)
#define LOW_EQ_FREQ_IDX (0x00) // LOW EQ FREQ MSB
#define RES1_TUNE_IDX (0x00) // RES* 1 TUNE   
#define RES1_DECAY_IDX (0x01) // RES* 1 DECAY
#define GEQ_16HZ_IDX (0x01) // 16Hz    
#define LOW_EQ_AMP_IDX (0x02) // LOW EQ AMP MSB / RES* 1 AMP / 32Hz      
//#define _IDX (0x03) // LOW EQ AMP LSB / RES* 2 TUNE / 62Hz     
#define MID_EQ_FREQ_IDX (0x04) // MID EQ FREQ MSB /126Hz  
//#define _IDX (0x05) // MID EQ FREQ LSB / 250Hz 
#define MID_EQ_BW_IDX (0x06) // MID EQ BANDWIDTH / 500Hz        
#define MID_EQ_AMP_IDX (0x07) // MID EQ AMP
#define GEQ_1KHZ_IDX (0x07) // 1KHz   
#define GEQ_2KHZ_IDX (0x08) // 2KHz   
#define HIGH_EQ_FREQ_IDX (0x09) // HIGH EQ FREQ 
//#define _IDX (0x09) // RES* 2 DECAY
#define GEQ_4KHZ_IDX (0x09) // 4KHz  
//#define _IDX (0x0A) // RES* 2 AMP
#define GEQ_8KHZ_IDX (0x0A) // 8KHz    
#define HIGH_EQ_AMP_IDX (0x0B) // HIGH EQ AMP
#define GEQ_16KHZ_IDX (0x0B) // 16KHz 
//#define _IDX (0x0D) // LESLIE HIGH ROTOR LEVEL / TAP 1 DELAY MSB       
//#define _IDX (0x0E) // LOW MID EQ FREQ MSB / RES **1 TUNE      
//#define _IDX (0x0F) // LOW MID EQ FREQ LSB / RES** 1 DECAY     
//#define _IDX (0x10) // LOW MID EQ BANDWIDTH / RES** 2 TUNE     
//#define _IDX (0x11) // LOW MID EQ AMP MSB / RES** 2 DECAY      
//#define _IDX (0x12) // LOW MID EQ AMP LSB / RES** 3 TUNE       
//#define _IDX (0x13) // HIGH MID EQ FREQ MSB / RES** 3 DECAY    
//#define _IDX (0x14) // HIGH MID EQ FREQ LSB / RES** 4 TUNE     
//#define _IDX (0x15) // HIGH MID EQ BANDWIDTH / RES** 4 DECAY   
//#define _IDX (0x16) // HIGH MID EQ AMP MSB / RES** 5 TUNE      
//#define _IDX (0x17) // HIGH MID EQ AMP LSB / RES** 5 DECAY     
//#define _IDX (0x18) // SAMPLE START / TAP 1 DELAY LSB  
//#define _IDX (0x19) // SAMPLE LENGTH / TAP 1 VOLUME    
//#define _IDX (0x1A) // PITCH MODE      
//#define _IDX (0x1B) // PITCH INPUT (0)
//#define _IDX (0x1B) // RES** 4 AMP (1-7)     
//#define _IDX (0x1C) // LFO WAVEFORM (0)
//#define _IDX (0x1C) // EQ PRESET NUMBER (1-7)       
//#define _IDX (0x1D) // LFO SPEED       
//#define _IDX (0x1E) // LFO DEPTH
//#define _IDX (0x1E) // RES*** DECAY        
//#define _IDX (0x1F) // SAMPLE PLAYBACK MODE / TAP 1 PAN        
//#define _IDX (0x1F) // SAMPLE PLAYBACK MODE / TAP 1 PAN        
//#define _IDX (0x20) // PITCH FEEDBACK  
//#define _IDX (0x21) // DETUNE AMOUNT / SAMPLE PITCH    
//#define _IDX (0x21) // DETUNE AMOUNT / SAMPLE PITCH    
//#define _IDX (0x22) // LESLIE SEPARATION / TAP 1 FEEDBACK      
//#define _IDX (0x22) // LESLIE SEPARATION / TAP 1 FEEDBACK      
//#define _IDX (0x23) // LESLIE MOTOR / SAMPLE REC AUDIO TRIG / TAP 2 DELAY MSB  
//#define _IDX (0x23) // LESLIE MOTOR / SAMPLE REC AUDIO TRIG / TAP 2 DELAY MSB  
//#define _IDX (0x23) // LESLIE MOTOR / SAMPLE REC AUDIO TRIG / TAP 2 DELAY MSB  
//#define _IDX (0x23) // LESLIE MOTOR / SAMPLE REC AUDIO TRIG / TAP 2 DELAY MSB  
//#define _IDX (0x23) // LESLIE MOTOR / SAMPLE REC AUDIO TRIG / TAP 2 DELAY MSB  
//#define _IDX (0x24) // LESLIE SPEED / SAMPLE MIDI TRIG / TAP 2 DELAY LSB       
//#define _IDX (0x24) // LESLIE SPEED / SAMPLE MIDI TRIG / TAP 2 DELAY LSB       
//#define _IDX (0x24) // LESLIE SPEED / SAMPLE MIDI TRIG / TAP 2 DELAY LSB       
//#define _IDX (0x25) // TRIGGER FLANGE / RES*** MIDI GATE       
//#define _IDX (0x25) // TRIGGER FLANGE / RES*** MIDI GATE       
//#define _IDX (0x26) // SAMPLE MIDI BASE NOTE / TAP 2 VOLUME    
#define DELAY_MODE_IDX (0x27) // DELAY MODE      
//#define _IDX (0x28) // DELAY INPUT (0)
//#define _IDX (0x28) // RES** 5 AMP (1-7)     
//#define _IDX (0x29) // DELAY INPUT MIX 
//#define _IDX (0x2A) // DELAY MSB
//#define _IDX (0x2A) // LEFT DELAY MSB
//#define _IDX (0x2A) // TAP 2 PAN  
//#define _IDX (0x2B) // TAP 2 FEEDBACK      
//#define _IDX (0x2C) // FEEDBACK
//#define _IDX (0x2C) // LEFT FEEDBACK
//#define _IDX (0x2C) // TAP 3 DELAY MSB      
//#define _IDX (0x2D) // RIGHT DELAY MSB
//#define _IDX (0x2E) // TAP 3 VOLUME  
//#define _IDX (0x2F) // RIGHT FEEDBACK
//#define _IDX (0x2F) // TAP 3 PAN      
//#define _IDX (0x30) // SAMPLE LOW MIDI NOTE / TAP 3 FEEDBACK   
//#define _IDX (0x31) // SAMPLE HIGH MIDI NOTE
//#define _IDX (0x31) // TAP 4 DELAY MSB 
//#define _IDX (0x32) // TAP 4 DELAY LSB   
#define REVERB_MODE_IDX (0x32) // REVERB MODE
//#define _IDX (0x33) // TAP 4 VOLUME    
//#define _IDX (0x34) // REVERB INPUT 1 / TAP 4 PAN      
//#define _IDX (0x35) // REVERB INPUT 2 / TAP 4 FEEDBACK 
//#define _IDX (0x36) // REVERB INPUT MIX / TAP 5 DELAY MSB      
//#define _IDX (0x37) // REVERB PREDELAY / TAP 5 DELAY LSB       
//#define _IDX (0x38) // REVERB PREDELAY MIX / TAP 5 VOLUME      
//#define _IDX (0x39) // REVERB DECAY / TAP 5 PAN        
//#define _IDX (0x3A) // REVERB DIFFUSION / TAP 5 FEEDBACK       
//#define _IDX (0x3B) // REVERB LOW DECAY / TAP 6 DELAY MSB      
//#define _IDX (0x3C) // REVERB HIGH DECAY / TAP 6 DELAY LSB     
//#define _IDX (0x3D) // REVERB DENSITY / TAP 6 VOLUME   
//#define _IDX (0x3E) // REVERB GATE / TAP 6 PAN 
//#define _IDX (0x3F) // REVERB GATE HOLD / TAP 6 FEEDBACK       
//#define _IDX (0x40) // REVERB GATE RELEASE / TAP 7 DELAY MSB   
//#define _IDX (0x41) // REVERB GATED LEVEL / TAP 7 DELAY LSB    
//#define _IDX (0x42) // RING MOD SHIFT MSB / TAP 7 VOLUME       
//#define _IDX (0x43) // RING MOD SHIFT LSB / TAP 7 PAN  
//#define _IDX (0x44) // CONFIGURATION   
#define PREPOST_EQ_IDX (0x45) // PRE-POST EQ (0)
#define DIRECT_LEVEL_IDX (0x45) // DIRECT LEVEL (1-7)    
#define PREAMP_LEVEL_IDX (0x46) // PREAMP LEVEL / EQ LEVEL 
#define EQ_LEVEL_IDX (0x46) // PREAMP LEVEL / EQ LEVEL 
#define MASTER_EFFECTS_LEVEL_IDX (0x47) // MASTER EFFECTS LEVEL    
#define PITCH_LEVEL_IDX (0x48) // PITCH LEVEL
//#define _IDX (0x48) // LESLIE LEVEL
//#define _IDX (0x48) // RING MOD LEV       
#define DELAY_LEVEL_IDX (0x49) // DELAY LEVEL     
#define REVERB_LEVEL_IDX (0x4A) // REVERB LEVEL
//#define _IDX (0x4A) // RES** 1 AMP      
//#define _IDX (0x4B) // RES*** PITCH 1
//#define _IDX (0x4B) // TAP 8 DELAY MSB        
//#define _IDX (0x4C) // RES*** PITCH 2
//#define _IDX (0x4D) // RES*** PITCH 3
//#define _IDX (0x4D) // TAP 8 VOLUME   
//#define _IDX (0x4E) // RES*** PITCH 4
//#define _IDX (0x4E) // TAP 8 PAN      
//#define _IDX (0x4F) // RES*** PITCH 5
//#define _IDX (0x4F) // TAP 8 FEEDBACK 
#define MOD1_SOURCE_IDX (0x50) // MOD 1 SOURCE    
#define MOD1_TARGET_IDX (0x51) // MOD 1 TARGET    
#define MOD1_AMP_IDX (0x52) // MOD 1 AMPLITUDE 
#define MOD2_SOURCE_IDX (0x53) // MOD 2 SOURCE    
#define MOD2_TARGET_IDX (0x54) // MOD 2 TARGET    
#define MOD2_AMP_IDX (0x55) // MOD 2 AMPLITUDE 
#define MOD3_SOURCE_IDX (0x56) // MOD 3 SOURCE    
#define MOD3_TARGET_IDX (0x57) // MOD 3 TARGET    
#define MOD3_AMP_IDX (0x58) // MOD 3 AMPLITUDE 
#define MOD4_SOURCE_IDX (0x59) // MOD 4 SOURCE    
#define MOD4_TARGET_IDX (0x5A) // MOD 4 TARGET    
#define MOD4_AMP_IDX (0x5B) // MOD 4 AMPLITUDE 
#define MOD5_SOURCE_IDX (0x5C) // MOD 5 SOURCE    
#define MOD5_TARGET_IDX (0x5D) // MOD 5 TARGET    
#define MOD5_AMP_IDX (0x5E) // MOD 5 AMPLITUDE 
#define MOD6_SOURCE_IDX (0x5F) // MOD 6 SOURCE    
#define MOD6_TARGET_IDX (0x60) // MOD 6 TARGET    
#define MOD6_AMP_IDX (0x61) // MOD 6 AMPLITUDE 
#define MOD7_SOURCE_IDX (0x62) // MOD 7 SOURCE    
#define MOD7_TARGET_IDX (0x63) // MOD 7 TARGET    
#define MOD7_AMP_IDX (0x64) // MOD 7 AMPLITUDE 
#define MOD8_SOURCE_IDX (0x65) // MOD 8 SOURCE    
#define MOD8_TARGET_IDX (0x66) // MOD 8 TARGET    
#define MOD8_AMP_IDX (0x67) // MOD 8 AMPLITUDE 
//#define _IDX (0x68) // MULTITAP MASTER FEEDBACK        
//#define _IDX (0x69) // MULTITAP NUMBER 
#define NAME_IDX (0x6A) // 1ST DIGIT NAME  
//#define _IDX (0x78) // RING MOD OUTPUT MIX
//#define _IDX (0x78) // RES** 2 AMP       
//#define _IDX (0x79) // RING MOD DEL/REV MIX
//#define _IDX (0x79) // RES** 3 AMP      
#define PAN_SPEED_IDX (0x7A) // PAN SPEED       
#define PAN_DEPTH_IDX (0x7B) // PAN DEPTH       
//#define _IDX (0x7C) // EQ-MODE (7)
#define PREAMP_COMP_IDX (0x7C) // COMPRESSION (4-6)
#define PREAMP_DIST_IDX (0x7C) // DISTORTION (3-0)        
#define MIX_MOD_IDX (0x7D) // MIX MODULATION (6-7) 
#define EFFECT_LOOP_IDX (0x7D) // EFFECT LOOP (5)
#define BASS_BOOST_IDX (0x7D) // BASS BOOST (4) 
#define PREAMP_TONE_IDX (0x7D) // AMP TONE (3-2)
#define CAB_SIM_IDX (0x7D) // CAB (1-0)    
#define PREAMP_OD_IDX (0x7E) // OVERDRIVE (7-5)
#define PREAMP_GATE_IDX (0x7E) // NOISE GATE (4-0)      
#define PREAMP_OUT_LEVEL_IDX (0x7F) // PREAMP OUTPUT LEVEL     

#define NAME_LENGTH       (14)

const UInt8 Sysex_Alesis[]={0x00,0x00,0x0E};   // Manufacturer Id: Alesis

const UInt8 Sysex_Quad[]={0x02};               // Device Id: Quadraverb
const UInt8 Sysex_QuadGT[]={0x07};             // Device Id: Quadraverb GT

const UInt8 Sysex_Edit[]={0x01};               // Command to edit a Quadraverb function
const UInt8 Sysex_Data_Dump[]={0x02};          // Command to send a program data dump
const UInt8 Sysex_Dump_Req[]={0x03};           // Command to request a dump

UInt32 QuadGT_Decode_From_Sysex(UInt8 *in, UInt32 length, UInt8* out, UInt32 out_len);
UInt32 QuadGT_Encode_To_Sysex(UInt8 *in, UInt32 length, UInt8 * out, UInt32 out_len);

void QuadGT_Init(void);
void QuadGT_Display_Update_Patch(UInt8 program);
void QuadGT_Display_Update(UInt8 program, UInt8 *quad_data);
void QuadGT_Display_Update_Reverb(const UInt8 config, const UInt8 * const quad_data);
void QuadGT_Display_Update_Delay(const UInt8 config, const UInt8 * const quad_data);
void QuadGT_Display_Update_Pitch(const UInt8 config, const UInt8 * const quad_data);
void QuadGT_Display_Update_Eq(const UInt8 config, const UInt8 * const quad_data);
void QuadGT_Display_Update_Mix(const UInt8 config, const UInt8 * const quad_data);
void QuadGT_Display_Update_Mod(const UInt8 config, const UInt8 * const quad_data);
void QuadGT_Display_Update_Preamp(const UInt8 config, const UInt8 * const quad_data);
