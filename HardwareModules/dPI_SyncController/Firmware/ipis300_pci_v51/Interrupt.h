#ifndef __INTERRUPT_HEADER_FILE__
#define __INTERRUPT_HEADER_FILE__

void Init_Interrupt(void);

void INT0_Enable(void);
void INT0_Disable(void);

void INT1_Enable(void);
void INT1_Disable(void);

void Wait_FEN(void);

#endif
