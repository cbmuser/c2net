#ifndef cbm_h
#define cbm_h

// C2N Setups
// for Raspberry Pi Pico W

const int writeport = 2;       // use level-shifter
const int senseport = 8;       // use level-shifter
const int motorport = 9;       // use level-shifter and voltage devider        
const int readport  = 10;      // not in use, for future development
const int LED  = 25;
const bool inverter = false;   // use of an driving inverter like 74LS06 ?
/*
 CBM 2001N 

S = 358 µs or  2.793kHz
M = 504 µs or  1.984kHz
L = 672 µs or  1.488kHz

*/
// Commodore 64 NTSC Pulses work fine for all.

#define  short_pulse   2840    // Hz
#define  medium_pulse  1953    // Hz
#define  long_pulse    1488    // Hz
// Functions

void setup_ports();
void delay_halfHz(uint32_t const microseconds);
bool no_irq();
bool irq();
void write_pulse (float pulse );
void set_long_pulse ();
void set_medium_pulse ();
void set_short_pulse ();
void send_bit(uint8_t value);
void send_byte(uint8_t value);
void leader_intro(uint16_t length);
void sync(uint8_t value);
void loader (char* buffer);
#endif
