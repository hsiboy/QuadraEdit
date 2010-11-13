typedef unsigned char UInt8;
typedef signed char SInt8;

typedef unsigned short UInt16;

typedef unsigned int UInt32;     // 32 Bits, Unsigned, Equivalent to DWORD

// One bit
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
#define WORD_MASK (0xFFFF)
#define WORD_SHIFT (16)

#define BYTE_MASK (0xFF)
#define BYTE_SHIFT (8)
