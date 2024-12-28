#include <Arduino.h>
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "cbm.h"

int payload = 192;
bool datasette;  
bool isirq = true;
char stream[192];

void setup_ports() {
  pinMode(writeport,OUTPUT);  
  digitalWrite(writeport, LOW);
  
  pinMode(senseport,OUTPUT);  
  digitalWrite(senseport, HIGH);
  
  pinMode(motorport,INPUT_PULLUP);    

  pinMode(LED_BUILTIN,OUTPUT);  
  digitalWrite(LED_BUILTIN, LOW);
}

void delay_halfHz(uint32_t microseconds)
{
     uint64_t ticks;
     uint32_t base = 500000; 
     
     microseconds = base / microseconds;       

         
     ticks = time_us_64();


    while(true)
    {
        if(timer_hw->timelr - ticks >= microseconds)
        {
            return;
        }
    }
    
}


bool no_irq() {
   noInterrupts();
    return isirq = false;
}

bool irq() {
   interrupts();
    return isirq = true;   
}


void write_pulse (float pulse ){
    
    
    if (inverter) {
     digitalWrite(writeport, HIGH);
     delay_halfHz(pulse);   
     digitalWrite(writeport, LOW);
     delay_halfHz(pulse);
      }

      else

      {
     digitalWrite(writeport, LOW);
     delay_halfHz(pulse);     
     digitalWrite(writeport, HIGH);
     delay_halfHz(pulse);
      }
}



void set_long_pulse (){

 write_pulse (long_pulse);
 
}

void set_medium_pulse (){

 write_pulse (medium_pulse);
 
}

void set_short_pulse (){

 write_pulse (short_pulse);
 
}



void send_bit(uint8_t value) {
  if (value) {
    set_medium_pulse ();
    set_short_pulse ();
  } else {
    set_short_pulse ();
    set_medium_pulse ();
  }
}


void send_byte(uint8_t value) {
  /* marker */
    set_long_pulse ();
    set_medium_pulse ();

  bool parity = true;
  for (uint8_t i = 0; i < 8; i++) {
    uint8_t bit = value & 1;

    send_bit(bit);
    if (bit)
      parity = !parity;

    value >>= 1;
  }

  send_bit(parity);
}

void leader_intro(uint16_t length)

{

   for (int i=0; i <= length;i++){

     set_short_pulse ();
     
   }

  
}

void sync(uint8_t value) {

  for (uint8_t i = 9; i > 0; i--) {
  
   send_byte(value + i);
  
  }
}



void loader (char* buffer) {

 uint8_t pointer_lo= buffer[18];
 uint8_t pointer_hi= buffer[19];
 uint8_t basend_hi   = pointer_hi + buffer[0];
 uint8_t basend_lo   = pointer_lo + buffer[1] - 20;
 uint16_t result =  ((basend_hi * 256) + basend_lo ) - ((pointer_hi *256) + pointer_lo);
 uint8_t payloads = floor(result /192);
 uint8_t rest = result- (payloads *192);


/*

Serial.println("----------------------");
Serial.print("Filename: ");
  for (uint8_t j = 0; j < 16; j++) {
    char b = buffer[j+2];

Serial.print(b);

  }
Serial.println("");
Serial.print("Adresse: ");
Serial.print(pointer_hi,HEX);
Serial.print(pointer_lo,HEX);
Serial.println("");
Serial.print("bas_hi: ");
Serial.print(pointer_hi,HEX);
Serial.println("");
Serial.print("bas_lo: ");
Serial.print(pointer_lo,HEX);
Serial.println("");
Serial.print("Endaddr: ");
Serial.print(basend_hi,HEX);
Serial.print(basend_lo,HEX);
Serial.println("");
Serial.print("filesize ");
Serial.print(result);
Serial.println("");
Serial.print("payloads ");
Serial.print(payloads);
Serial.println("");
Serial.print("rest ");
Serial.print(rest);
Serial.println("");


Serial.println("LOADING ....");

*/

digitalWrite(LED_BUILTIN, HIGH);
   stream[0] = 0x01;
   stream[1] = pointer_lo;
   stream[2] = pointer_hi;
   stream[3] = basend_lo;
   stream[4] = basend_hi;
  for (uint8_t j = 0; j < 16; j++) {

    if ( buffer[j+2] <= 122 && buffer[j+2] >= 97)  
     { stream[5+j]  = bitClear(buffer[j+2],5); }
    else
     { stream[5+j]  = buffer[j+2]; }   
  }
 
  for (uint8_t j = 0; j < 168; j++) {
    stream[24+j] = 0x20;
  }
 //---------------------------------------
 // Header
 //---------------------------------------
   digitalWrite(senseport, LOW);
   no_irq();    
    leader_intro(4000);
    sync(0x80);

  uint8_t checksum =0;
  for (uint8_t j = 0; j <= 191; j++) {

    send_byte (stream[j]);
    checksum ^= stream[j];
  }
     send_byte(checksum);
     set_long_pulse();

     leader_intro(80);
     sync(0x00);
  checksum =0;
  for (uint8_t j = 0; j <= 191; j++) {

    send_byte (stream[j]);
    checksum ^= stream[j];
  }
     send_byte(checksum);
     set_long_pulse ();

     set_long_pulse ();
     set_short_pulse ();
     digitalWrite(writeport, LOW);      
     delay_halfHz(12);   
     leader_intro(6000);
    
// FOUND ....
   
    sync(0x80); 
    checksum =0;
  for (uint16_t j = 20; j <= result+19; j++) {

    send_byte (buffer[j]);
    checksum ^= buffer[j];
    }  
   send_byte(checksum);
   set_long_pulse();

   leader_intro(80); 
   sync(0x00);
   
  checksum =0;
  for (uint16_t j = 20; j <= result+19; j++) {

    send_byte (buffer[j]);
    checksum ^= buffer[j];
    }    
    send_byte(checksum);
    set_long_pulse ();
    leader_intro(10);  
    digitalWrite(writeport, LOW);
    irq();
    digitalWrite(senseport, HIGH);
    digitalWrite(LED_BUILTIN, LOW);
}
