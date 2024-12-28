# c2net
A Wifi C2N-Datasette for Commodore Computers



```

 Raspberry Pi Pico W (RP2040)


                                        -----
                                Pin  __|     |__
                           GP0  1 )o( )|     |( )o( 40 VBUS     <---- 5V=
                           GP1  2 )o      R      o( 39 VSYS
                           GND  3 )o      A      o( 38 GND      ----- GND
 levelshifter---write <--- GP2  4 )o      S      o( 37 3_3V_EN          
                           GP3  5 )o      P      o( 36 3_3V_OUT ----- 3.3V Out
                 LCD  <--- SDA  6 )o      B      o( 35 ADC_VREF
                 LCD  <--- SCL  7 )o      E      o( 34 GP28
                           GND  8 )o      R      o( 33 GND    
                           GP6  9 )o      R      o( 32 GP27
                           GP7 10 )o      Y      o( 31 GP26
 levelshifter---SENSE ---> GP8 11 )o             o( 30 RUN   <--- SW-RESET -- GND (RESET)
 levelshifter---MOTOR ---> GP9 12 )o   PI PICO   o( 29 GP22          
                           GND 13 )o             o( 28 GND           
               (READ)x--> GP10 14 )o   -------   o( 27 GP21         
                          GP11 15 )o  |       |  o( 26 GP20
                          GP12 16 )o  |   W   |  o( 25 GP19/SPI  TX  --> Cardreader
                          GP13 17 )o  |       |  o( 24 GP18/SPI CLK  --> Cardreader
                           GND 18 )o   -------   o( 23 GND            
     GND -- SW-READER ->  GP14 19 )o             o( 22 GP17/SPI /CS  --> Cardreader
     GND -- SW-SELECT ->  GP15 20 )o( )       ( )o( 21 GP16/SPI  RX  <-- Cardreader
                                    -------------  
                             
     Motor
     -----
          
     CBM Tapeport Pin C3     ---[ 1K resistor ]---|
                                                   >------- Levelshifter 
                         GND ---[ 1K resistor ]---| 

```
