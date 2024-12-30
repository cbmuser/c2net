/*
 CBM 2001N 

S = 358 µs or  2.793kHz
M = 504 µs or  1.984kHz
L = 672 µs or  1.488kHz

C64 PAL (B3-Board)

S = 376 µs or  2.632kHz
M = 528 µs or  1.894kHz
L = 712 µs or  1.404kHz

  The Commodore Tape-Protocolflow
  -------------------------------

                                       count &     80     count &     80             2s     count &      count &        2s
         100ms      10.450s short      data      short    data      short   332ms   short   filedata     filedata       short
         low   __________...________             ______   repeat    ______   low    _____                repeat         _____ 
              |                     |||||...|||||      |||||...|||||      |        |     ||||.......||||.||||.......||||     |
  Read   _____|                     |||||...|||||      |||||...|||||      |________|     ||||.......||||.||||.......||||     |_______
  Write                                                            | 
                                                                    > FOUND on LOAD                                                 
              |____________________________________________________|                     |______________________________|
              {                   header                           }                     {          filedata            }

  Motor  __________________________________________________________                _________________________________________        
                                                                   |              |                                         |
                                                                   |______________|                                         |________ 
                                                                     
                                                                     
  Sense                                                           
        |
        |____________________________________________________________________________________________________________________________


*/
// Commodore 64 NTSC Pulses work fine for all.

#define  short_pulse   2840    // Hz
#define  medium_pulse  1953    // Hz
#define  long_pulse    1488    // Hz


/*
 *  c2n class 
 *   
 *  Code: Michael Sachse
 * 
 *  Done to the public domain.
 * 
 * 
 * Use the constructor for your pinout and eventually use of an inverting driver
 * 
 * 
 */


class c2n
{
    public: 

        c2n(const uint8_t wp, const uint8_t sp, const uint8_t mp, const uint8_t rp, bool iv)
         : writeport{wp}, senseport{sp}, readport{rp}, motorport{mp}, inverter{iv} {}
         
        
        void c2ninit();
        void delay_Hz(uint16_t hz);
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
        void toggle_sense(uint8_t _delay);
        void set_sense(uint8_t value);  
     uint8_t motor();
    
    
    protected: 

        uint8_t writeport;
        uint8_t readport;
        uint8_t senseport;
        uint8_t motorport;
        int payload = 192;

        char stream[192];

        bool isirq;
        bool inverter;  




};
