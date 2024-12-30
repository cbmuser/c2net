#include <Arduino.h>
#include <cstdint>
#include "c2n.h"
#include "hardware/timer.h"
#include "hardware/irq.h"



void c2n::c2ninit() 
  {
    pinMode(writeport,OUTPUT);  
    digitalWrite(writeport, LOW);
    
    pinMode(senseport,OUTPUT);  
    digitalWrite(senseport, HIGH);
    
    pinMode(motorport,INPUT_PULLUP);    
   
    pinMode(readport,INPUT_PULLUP);    
        
    pinMode(LED_BUILTIN,OUTPUT);  
    digitalWrite(LED_BUILTIN, LOW);
  }

void c2n::delay_Hz(uint16_t hz)
{
     uint64_t ticks;
     uint32_t base = 500000;      
     uint32_t microseconds = base / hz;       
     ticks = time_us_64();


    while(true)
    {
        if(timer_hw->timelr - ticks >= microseconds)
        {
            return;
        }
    }
    
}


bool c2n::no_irq() {
   noInterrupts();
    return isirq = false;
}

bool c2n::irq() {
   interrupts();
    return isirq = true;   
}


void c2n::write_pulse (float pulse ){
    
    
    if (inverter) {
     digitalWrite(writeport, HIGH);
     delay_Hz(pulse);   
     digitalWrite(writeport, LOW);
     delay_Hz(pulse);
      }

      else

      {
     digitalWrite(writeport, LOW);
     delay_Hz(pulse);     
     digitalWrite(writeport, HIGH);
     delay_Hz(pulse);
      }
}


void c2n::set_long_pulse (){

 c2n::write_pulse (long_pulse);
 
}

void c2n::set_medium_pulse (){

 c2n::write_pulse (medium_pulse);
 
}

void c2n::set_short_pulse (){

 c2n::write_pulse (short_pulse);
 
}

void c2n::send_bit(uint8_t value) {
  if (value) {
    c2n::set_medium_pulse ();
    c2n::set_short_pulse ();
  } else {
    c2n::set_short_pulse ();
    c2n::set_medium_pulse ();
  }
}


void c2n::send_byte(uint8_t value) {
  /* marker */
    c2n::set_long_pulse ();
    c2n::set_medium_pulse ();

  bool parity = true;
  for (uint8_t i = 0; i < 8; i++) {
    uint8_t bit = value & 1;

    c2n::send_bit(bit);
    if (bit)
      parity = !parity;

    value >>= 1;
  }

  c2n::send_bit(parity);
}



void c2n::leader_intro(uint16_t length)

{

   for (int i=0; i <= length;i++){

     c2n::set_short_pulse ();
     
   }

  
}

void c2n::sync(uint8_t value) {

  for (uint8_t i = 9; i > 0; i--) {
  
   c2n::send_byte(value + i);
  
  }
}

void c2n::loader (char* buffer) {

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
    c2n::no_irq();    
    c2n::leader_intro(4000);
    c2n::sync(0x80);

  uint8_t checksum =0;
  for (uint8_t j = 0; j <= 191; j++) {

    c2n::send_byte (stream[j]);
    checksum ^= stream[j];
  }
     c2n::send_byte(checksum);
     c2n::set_long_pulse();

     c2n::leader_intro(80);
     c2n::sync(0x00);
  checksum =0;
  for (uint8_t j = 0; j <= 191; j++) {

    c2n::send_byte (stream[j]);
    checksum ^= stream[j];
  }
     c2n::send_byte(checksum);
     c2n::set_long_pulse ();

     c2n::set_long_pulse ();
     c2n::set_short_pulse ();
     digitalWrite(writeport, LOW);      
     c2n::delay_Hz(12);   
     c2n::leader_intro(6000);
    
// FOUND ....
   
    c2n::sync(0x80); 
    checksum =0;
  for (uint16_t j = 20; j <= result+19; j++) {

    c2n::send_byte (buffer[j]);
    checksum ^= buffer[j];
    }  
   c2n::send_byte(checksum);
   c2n::set_long_pulse();

   c2n::leader_intro(80); 
   c2n::sync(0x00);
   
  checksum =0;
  for (uint16_t j = 20; j <= result+19; j++) {

    c2n::send_byte (buffer[j]);
    checksum ^= buffer[j];
    }    
    c2n::send_byte(checksum);
    c2n::set_long_pulse ();
    c2n::leader_intro(10);  
    digitalWrite(writeport, LOW);
    c2n::irq();
    digitalWrite(senseport, HIGH);
    digitalWrite(LED_BUILTIN, LOW);
}


void c2n::toggle_sense(uint8_t _delay) {

     delay(_delay);
      digitalWrite(senseport, LOW);
     delay(_delay);
      digitalWrite(senseport, HIGH);
     delay(_delay);
 
}


void c2n::set_sense (uint8_t value) {

     digitalWrite(senseport, value);

}

bool c2n::motor(){

   uint8_t is_motor = digitalRead(motorport); 
   
   return is_motor;
   
}

