#include <mega32.h>
#include <delay.h>
#define RED_N    0
#define YLW_N    1
#define GRN_N    2
#define RED_S    3
#define YLW_S    4
#define GRN_S    5
#define RED_E    6
#define YLW_E    7
#define GRN_E    0
#define RED_W    1
#define YLW_W    2
#define GRN_W    3
const unsigned char seg7[10] = {
    0b00111111,
    0b00000110,
    0b01011011,
    0b01001111,
    0b01100110,
    0b01101101,
    0b01111101,
    0b00000111,
    0b01111111,
    0b01101111
};
volatile unsigned char timer_ticks = 0;
volatile unsigned char seconds     = 0;
volatile unsigned char disp_val    = 0;
volatile unsigned char blink_ticks = 0;
volatile unsigned char blink_flag  = 0;
typedef enum {
    NS_GREEN = 0,
    NS_YELLOW,
    EW_GREEN,
    EW_YELLOW
} Phase;
volatile Phase current_phase = NS_GREEN;
const unsigned char PHASE_TIME[4] = { 9, 3, 9, 3 };
interrupt [TIM0_OVF] void timer0_ovf(void)
{
    TCNT0 = 131;
    timer_ticks++;
    blink_ticks++;
    if(blink_ticks >= 63) {
        blink_ticks = 0;
        blink_flag ^= 1;
    }
    if(timer_ticks >= 125) {
        timer_ticks = 0;
        seconds++;
        if(disp_val > 0) disp_val--;
    }
}
void set_lights(Phase ph)
{
    unsigned char pa = 0, pb = 0;
    switch(ph) {
        case NS_GREEN:
            pa = (1<<GRN_N)|(1<<GRN_S)|(1<<RED_E);
            pb = (1<<RED_W);
            break;
        case NS_YELLOW:
            if(blink_flag) {
                pa = (1<<YLW_N)|(1<<YLW_S)|(1<<YLW_E);
                pb = (1<<YLW_W);
            } else {
                pa = 0;
                pb = 0;
            }
            break;
        case EW_GREEN:
            pa = (1<<RED_N)|(1<<RED_S);
            pb = (1<<GRN_E)|(1<<GRN_W);
            break;
        case EW_YELLOW:
            if(blink_flag) {
                pa = (1<<YLW_N)|(1<<YLW_S)|(1<<YLW_E);
                pb = (1<<YLW_W);
            } else {
                pa = 0;
                pb = 0;
            }
            break;
    }
    PORTA = pa;
    PORTB = pb & 0x0F;
}
cvoid display_7seg(unsigned char val)
{
    PORTC = seg7[val];
}
void update_phase(void)
{
    if(seconds >= PHASE_TIME[current_phase]) {
        seconds = 0;
        blink_flag = 0;
        blink_ticks = 0;
        current_phase = (current_phase + 1) % 4;
        disp_val = PHASE_TIME[current_phase];
    }
}
void init_ports(void)
{
    DDRA = 0xFF;  PORTA = 0x00;
    DDRB = 0xFF;  PORTB = 0x00;
    DDRC = 0xFF;  PORTC = 0x00;
    DDRD = 0x00;  PORTD = 0x00;
}
void init_timer0(void)
{
    TCCR0 = 0x04;
    TCNT0 = 131;
    TIMSK = (1<<TOIE0);
}
void main(void)
{
    init_ports();
    init_timer0();
    disp_val = PHASE_TIME[NS_GREEN];
    #asm("sei")
    while(1) {
        update_phase();
        set_lights(current_phase);
        display_7seg(disp_val);
    }
}
