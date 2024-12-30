#include <Arduino.h>
#include <SD.h>
#include "setups.h"
#include <Wire.h>
#include "AT24C256.h"
AT24C256 e2prom = AT24C256();

#define maximum 128
char secret[maximum];



/*
 *  Reads the file /sys/secret.c2n
 *  Requires:
 *  SSID,PASSWORD <- No spaces or anything else at the end ! 
 *  
 *  Writes the e²prom and deletes the file
 *  
 *  No file: attempt to read the e²prom creditials 
 */
String setups::get_creditials(String creditsfile) {
     String filepath = String(folder)+creditsfile;  
     int i=0; int j=0;
if (SD.exists(filepath)) {
                          File creditfile = SD.open(filepath,"r");  // get the creditials 
                          while (creditfile.available()) 
                           {
                            char inputchar = creditfile.read();
                            secret[i] = inputchar;
                            i++;
                           }                          
                
                            e2prom.write(i, 0);                     // write amount of chars
                            
                             j=0;                                   // write creditials to e²prom                            
                        do {                              
                              e2prom.write(secret[j],j+1);
                              j++; 
                         } while (j <= i);    
                      
                        SD.remove(filepath);                        // delete the creditials-file secret.c2n 
                      }
    
               else {                                               // no file, read e²prom for creditials                      
                       i = e2prom.read(0);                          // get amount of chars
                       j = 0;
                   do {               
                       secret[j] =  e2prom.read(j+1);
                       j++;                  
                      } while(j <= i+1);
                    }    
        return secret;
}     
