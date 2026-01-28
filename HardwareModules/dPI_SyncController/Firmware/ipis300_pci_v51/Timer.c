#include "TypeDef.h"
#include "iom128.h"
#include "Variables.h"
#include "Timer.h"

void Init_Timers(void)
{
    TCCR1A = 0x00;
    TCCR1B = 0x00;
    TCCR1C = 0x00;
    
    TCCR3A = 0x00;
    TCCR3B = 0x00;
    TCCR3C = 0x00;
    
	Timer_Reset();
	WatchDog_Reset();
}

void Timer_Start(void)          // Tick Clock : 1ms
{
    Timer_Count_ms = 0;
	TIMSK |= 0x04;              // 00000100 : Timer/Counter1 Overflow Interrupt Enable
	TCNT1 = Timer_Elapse_Tick;  // Overflow after 1000-Clock Elapsed
	TCCR1B = 0x02;              // 00000010 : clkT0S/8 (From prescaler)
}

void Timer_Reset(void)
{
	TCCR1B = 0x00;              // 00000000 : No Clock Source
	TCNT1 = 0x0000;             // Clear Counter
	TIMSK &= ~0x04;             // 00000100 : Timer/Counter1 Overflow Interrupt Disable
	Timer_Count_ms = 0;
}

void WatchDog_Start(void)       // Tick Clock : 1ms
{
    WatchDog_Count_ms = 0;
    ETIMSK |= 0x04;             // 00000100 : Timer/Counter3 Overflow Interrupt Enable
    TCNT3 = Timer_Elapse_Tick;  // Overflow after 100-Clock Elapsed
    TCCR3B = 0x02;              // 00000010 : clkT0S/8 (From prescaler)
}

void WatchDog_Reset(void)
{
    TCCR3B = 0x00;              // 00000000 : No Clock Source
    TCNT3 = 0x0000;             // Clear Counter
	ETIMSK &= ~0x04;            // 00000100 : Timer/Counter3 Overflow Interrupt Disable
	WatchDog_Count_ms = 0;
}

