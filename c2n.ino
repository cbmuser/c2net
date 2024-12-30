#include <Arduino.h>
#include <NTPClient.h>
#include <cstdint>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <uri/UriRegex.h>
#include <SD.h>
#include "setups.h"
#include "c2n.h"
#include "url.h"
#include <LCD-I2C.h>
#include <Wire.h>
#include <DS1307Pico.h>
#include "AT24C256.h"
#include "SimpleTime.h"

DS1307Pico rtc;
AT24C256 eeprom = AT24C256();

#define maximum 65535
char prgbuffer[maximum];

c2n c2n(2,8,9,10,false);

setups setups("/sys/");

const int net_tape   = 14;
const int files_down = 15;
const int UTC = 3;

String dirstr ="";
String lastfile;
String filedisplay ="";

bool debug = true;
bool cardreader = false;
bool c2n_write = false;
bool myclock = false;

int secadd;

uint8_t count = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

LCD_I2C lcd(0x3f, 16, 2);

File root;

WebServer server(80);
File rawFile;
File dirFile;


String indexPage()
{
  
  String buf  ="";
  File indexFile = SD.open("sys/DIR",FILE_READ);
   if (indexFile)
     { while (indexFile.available()) {
      buf += char (indexFile.read()); }
     }
     return buf;
}  

//----------------------------------------------------------
// Writes the DIR-file
//----------------------------------------------------------

void printDirectory(File dir, int numTabs) {
  while (true) {

    if (SD.exists("sys/DIR")) {
      SD.remove("sys/DIR");
    }
    dirFile = SD.open("sys/DIR","w");
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
     if (debug) { Serial.print('\t'); }
      dirstr += ("\n");
    }
     if (debug) { Serial.print(entry.name()); }
    dirstr += (entry.name());
    String dummy = (entry.name());  
    
    for (uint8_t i = 16; i >= dummy.length(); i--) {
    dirstr +=" ";  }
    
    if (entry.isDirectory()) {
     if (debug) { Serial.println("/"); }
//      dirstr += ("\n\n");
    } else {
      if (debug) { Serial.print("\t\t"); }
      dirstr += (entry.size());
      if (debug) { Serial.println(entry.size(), DEC); }
      dirstr += ("\n");
    }

    
    entry.close();
  }
    dirFile.println(dirstr); 
    dirFile.close();
 }

//----------------------------------------------------------


void handleCreate() {
     server.send(500, "text/plain", "\r\n");
}


//----------------------------------------------------------

void buffer_lastfile (String lastfile) {

 for (uint8_t j = 0; j <= 15; j++) 
  {
   char b = lastfile[j];
   prgbuffer[j+2] = b;
  }
   File c2nFile = SD.open(lastfile, "r");
   int i = 18;
   while (c2nFile.available()) 
    {
     byte inputchar = c2nFile.read();
     prgbuffer[i] = inputchar;
     i++;
    }
    uint8_t xlow = i & 0xff;
    uint8_t xhigh = (i >> 8);
    prgbuffer[0] =  xhigh;
    prgbuffer[1] =  xlow;
    c2nFile.close();    
  }      

//----------------------------------------------------------
//  The Upload, may be expanded
//----------------------------------------------------------
void handleCreateProcess() {
   
  String path = urldecode(("/" + server.pathArg(0)));  
  String lastfile = urldecode(server.pathArg(0));
  
  
  HTTPRaw& raw = server.raw();
  if (raw.status == RAW_START) {
  
   if (SD.exists((char *)path.c_str())) {
         SD.remove((char *)path.c_str());
    }
    
   rawFile = SD.open(path.c_str(), "w");

     if (debug) { Serial.print("Upload: START, filename: ");
                  Serial.println (path); }

  } else if (raw.status == RAW_WRITE) {
    if (rawFile) {
      rawFile.write(raw.buf, raw.currentSize);
    }
     if (debug) {
                  Serial.print("Upload: WRITE, Bytes: ");
                  Serial.println(raw.currentSize); }
  } else if (raw.status == RAW_END) {

    if (rawFile) {
      rawFile.close();
    }
     if (debug) {
                  Serial.print("Upload: END, Size: ");
                  Serial.println(raw.totalSize); }
    dirstr ="";
    printDirectory(root, 0);

     buffer_lastfile (lastfile);
      lcd.setCursor(0, 1);
      lcd.print ("                ");
     delay(200);
      lcd.setCursor(0, 1);
      lcd.print(lastfile);
 
  }
  
}
    
//----------------------------------------------------------  


void returnFail(String msg) {
  server.send(200, "text/plain", indexPage());
}

//----------------------------------------------------------

void handleNotFound() {
    server.send(200, "text/plain", indexPage());
}
//------------------------------------------------------------
//  Setup 
//------------------------------------------------------------

void setup(void) {

     c2n.c2ninit();
     
     pinMode (net_tape, INPUT_PULLUP);
     pinMode (files_down, INPUT_PULLUP);
     pinMode (UTC, INPUT_PULLUP);

     int season =  digitalRead(UTC); 
      switch (season) {
                        case HIGH:  secadd = 7200;
                         break;
                        case LOW:   secadd = 3600;
                         break;
                      }     
     Wire.begin();
     lcd.begin(&Wire);
     lcd.display();
     lcd.backlight();
     lcd.print("CONNECT");

     
     delay(10);
      while (!SD.begin(17u)) 
             {
              delay(100);
             }     

 if (debug) { 
             delay(3000);
             Serial.begin(115200);
             Serial.println("SD Card initialized.");
            }          

        String credit = setups.get_creditials("secret.c2n");
         int ofs = credit.indexOf(",");   
         String myssid = credit.substring(0, ofs); 
         String password = credit.substring(ofs+1, credit.length()+1); 
          int ssidlen = ofs+1;
          int passlen =  credit.length() - ofs -1;
         char ssid[ssidlen];     
         char pass[passlen];  
      myssid.toCharArray(ssid,ssidlen);
      password.toCharArray(pass,passlen);
       

if (debug) {        
             int i=0;   
              do {
                   Serial.print(ssid[i]);
                    i++;
         
                 } while (i < ssidlen);
                 Serial.println("");
                i=0;
             do {
                 Serial.print(pass[i]);
                 i++;
                } while (i < passlen);
       
            Serial.println("");  
           
             Serial.print("Pass Chars: ");
             Serial.print( eeprom.read(0));
             Serial.println("");  
               i= eeprom.read(0);
              int j= 0;
             do {
                 Serial.print(eeprom.read(j),DEC);
                 j++;
                 } while (j <= i-1);
              Serial.println("");    
           }          
     WiFi.mode(WIFI_STA);
     WiFi.begin(ssid, pass);

      while (WiFi.status() != WL_CONNECTED) 
          {
           lcd.setCursor(15, 0);
           lcd.print(".");
           delay(150);
           lcd.setCursor(15, 0);
           lcd.print("o");
           delay(150);
           lcd.setCursor(15, 0);
           lcd.print("O");
           delay(150);
           lcd.setCursor(15, 0);
           lcd.print("#");
           delay(150);  
         }
lcd.setCursor(15, 0);
lcd.print(" ");

if (debug) {  
             Serial.print("Connected to ");
             Serial.println(ssid);
             Serial.print("IP address: ");
             Serial.println(WiFi.localIP());
           }

  timeClient.begin();
  timeClient.update();
if (debug) {  
             Serial.print("Get NTP timestamp: ");
             Serial.println(timeClient.getEpochTime());
             Serial.println("");
            }
  rtc.begin();
  rtc.DSadjust(timeClient.getEpochTime()+secadd);  
  rtc.DSread();
if (debug) {              
             Serial.print("RTC GMT+2: ");
             Serial.print(rtc.hour);
             Serial.print(":");
             Serial.print(rtc.minute);
             Serial.print(":");
             Serial.print(rtc.second);
             Serial.println(""); 
            }
  server.on(UriRegex("/(.*)"), HTTP_PUT, handleCreate, handleCreateProcess);
  server.onNotFound(handleNotFound);
  server.begin();

if (debug) { Serial.println("HTTP server started"); }

  root = SD.open("/");
  lcd.print(".");
  printDirectory(root, 0);
  lcd.setCursor(0, 0);
  lcd.print("C2Net online !");
  c2n.set_sense(HIGH);  

}

//------------------------------------------------------------

String toggle_dir() {
  File dir = root;
  File entry =  dir.openNextFile(); 
  if (entry.isDirectory()) { entry =  dir.openNextFile(); }
  
  String loadfile = entry.name();
  lcd.setCursor(0, 1);
  lcd.print(loadfile);
  return loadfile;
  
}
//------------------------------------------------------------


void show_ip(){

  uint32_t  lastTime = millis();           
           
           lcd.setCursor(0, 0);
           lcd.print(WiFi.localIP());
           lcd.print("   ");
           lcd.setCursor(0, 1);


   
   do{
    
   } while (millis() - lastTime >= 1000);
           

  if (!myclock)
        {
          lcd.print(hour(timeClient.getEpochTime()+secadd)); 
          lcd.print(":"); 
          lcd.print(minute(timeClient.getEpochTime()+secadd)); 
          lcd.print(":"); 
          lcd.print(second(timeClient.getEpochTime()+secadd)); 
        } 
         else
        {
          
           lcd.print (rtc.DSgetTime("%A, %H:%M:%S"));
        }
 
         delay(2500);
           lcd.setCursor(0, 0);
           lcd.print("C2Net online !");
           lcd.setCursor(0, 1);
           lcd.print("                ");
  
}

//------------------------------------------------------------



void loop(void) {


if (cardreader == false) {  
 
                           server.handleClient();
                           delay(2);                    
                           c2n.toggle_sense(100);  
                           if (digitalRead(files_down) == LOW)  { show_ip(); }

                           if(c2n.motor()){ c2n.loader(prgbuffer); }

                            if (filedisplay != lastfile) 
                              { lcd.setCursor(0, 1); 
                                lcd.print(lastfile); 
                                filedisplay == lastfile; 
                              }

                          }
     
if (digitalRead(net_tape) == LOW) { delay(200); cardreader = true; lastfile ="";}

if (cardreader == true) {
                           lcd.setCursor(0, 0);
                           lcd.print("DATASETTE       ");
                           lcd.setCursor(0, 1);
                           lcd.print("                ");
                           toggle_dir();
                           File dir = root;
  
 while(cardreader == true) {
   
                           
  
  
  if (digitalRead(files_down) == LOW) 
                             {
                               File entry =  dir.openNextFile(); 
                               if (! entry) 
                                 {
                                  lcd.setCursor(0, 1);
                                  lcd.print("                ");
                                  break;
                                 }

                               String loadfile = entry.name(); 
                               if (lastfile != loadfile) {
                               lcd.setCursor(0, 1);
                               lcd.print("                ");
                               lcd.setCursor(0, 1);
                               lcd.print(loadfile);
                                delay(200);
                           lastfile = loadfile;
                               
                               
                                
                                 buffer_lastfile (lastfile); 
                                
                                
                               
                                }      
                               }

                              if (digitalRead(net_tape) == LOW) { delay(200); cardreader = false; lcd.setCursor(0, 0); lcd.print("C2Net active !   "); }   
                              
                              }
                            }
}
