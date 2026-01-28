#ifndef _DEFINE_H_
#define _DEFINE_H_

enum
{
    OFF = 0,
    ON = 1,
};

// Port E Out define
#define PORTE_CFG 0xFC

#define _DACS   0x80
#define _DACLK  0x40
#define _DASDI  0x20
#define _DALOAD 0x10
#define _DARST  0x08
#define _PZTAMP 0x04


// Port D InOut Define
#define PORTD_CFG 0x30

#define _TRGL   0x10    // Trigger Signal
#define _TRGS   0x20    // Trigger Signal

// Memory Assign
#define _SRAM     (*((unsigned char *)0x2000))
#define _LEDW0    (*((unsigned char *)0x8000))
#define _LEDW1    (*((unsigned char *)0x8004))
#define _LEDW2    (*((unsigned char *)0x8008))
#define _LEDW3    (*((unsigned char *)0x800C))
#define _HANDW0   (*((unsigned char *)0x9008))
#define _HANDW1   (*((unsigned char *)0x900C))
#define _HANDR0   (*((unsigned char *)0xA000))
#define _HANDR1   (*((unsigned char *)0xA004))
#define _PCI0     (*((unsigned char *)0xF000))
#define _PCI1     (*((unsigned char *)0xF004))
#define _PCI2     (*((unsigned char *)0xF008))
#define _PCI3     (*((unsigned char *)0xF00C))


#define DARST(a)  a ? (PORTE |= _DARST) : (PORTE &= ~_DARST)
#define DASDI(a)  a ? (PORTE |= _DASDI) : (PORTE &= ~_DASDI)
#define DACLK(a)  a ? (PORTE |= _DACLK) : (PORTE &= ~_DACLK)
#define DACS(a)   a ? (PORTE |= _DACS)  : (PORTE &= ~_DACS)
#define DALOAD(a) a ? (PORTE |= _DALOAD): (PORTE &= ~_DALOAD)

#define PZT_AMP(a) a ? (PORTE |= _PZTAMP) : (PORTE &= ~_PZTAMP)

#define TRG_L(a)   a ? (PORTD |= _TRGL)   : (PORTD &= ~_TRGL)
#define TRG_S(a)   a ? (PORTD |= _TRGS)   : (PORTD &= ~_TRGS)

// EEPROM Offset Assign
#define _CONFIG   0   // Device Config Information 10Bytes.

#endif
