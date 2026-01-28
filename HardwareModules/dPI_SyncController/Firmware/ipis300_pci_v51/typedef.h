#ifndef __TYPE_DEFINE_HEADER_FILE__
#define __TYPE_DEFINE_HEADER_FILE__


typedef   unsigned char   BYTE;		
typedef   unsigned int    WORD;		


typedef struct _def8_bit
{
	unsigned char bit0		:1; /**/
	unsigned char bit1		:1; /**/
	unsigned char bit2		:1; /**/
	unsigned char bit3		:1; /**/
	unsigned char bit4		:1; /**/
	unsigned char bit5		:1; /**/
	unsigned char bit6		:1; /**/
	unsigned char bit7		:1; /**/
} DEF8_BIT;		

typedef struct  _def16_bit
{
    unsigned int bit0       :1; /**/
    unsigned int bit1       :1; /**/
    unsigned int bit2       :1; /**/
    unsigned int bit3       :1; /**/
    unsigned int bit4       :1; /**/
    unsigned int bit5       :1; /**/
    unsigned int bit6       :1; /**/
    unsigned int bit7       :1; /**/
    unsigned int bit8       :1; /**/
    unsigned int bit9       :1; /**/
    unsigned int bit10      :1; /**/
    unsigned int bit11      :1; /**/
    unsigned int bit12      :1; /**/
    unsigned int bit13      :1; /**/
    unsigned int bit14      :1; /**/
    unsigned int bit15      :1; /**/
} DEF16_BIT;


typedef union   __DEF16
{
    unsigned int        def_word;
    unsigned char       ml[2];
    DEF16_BIT           bit;
} DEF16;


typedef union __DEF8
{
	unsigned char		def_byte;
	DEF8_BIT			bit;
} DEF8;

typedef struct CONFIG
{
  unsigned char CTiming;
  unsigned char RFU[9];
  unsigned char BCC;
} CONFIG_BUFF;

#endif
