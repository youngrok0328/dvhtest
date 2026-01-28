#ifndef __PZT_HEADER_FILE__
#define __PZT_HEADER_FILE__

void PZT_Init(void);
void PZT_ON(BYTE nStep);
void PZT_OFF(void);
void DA_Conversion(BYTE reg, BYTE nValue_h, BYTE nValue_l);

#endif
