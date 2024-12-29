#include <cstdint>
#include <WiFi.h>
#include <WebServer.h>
#include <uri/UriRegex.h>
#include <SD.h>
#include "cbm.h"
#include <LCD-I2C.h>
#include <Wire.h>


#define maximum 65535
char prgbuffer[maximum];


const int net_tape   = 14;
const int files_down = 15;


const char *ssid = "your SSID";
const char *password = "your password";

String dirstr ="";
String lastfile;
String filedisplay ="";
bool debug = false;
bool cardreader = false;
uint8_t count = 0;

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
      dirstr += ("\n\n");
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
//  The Upload, may be expanded
//----------------------------------------------------------
void handleCreateProcess() {
   
  String path = "/" + server.pathArg(0);
  String lastfile = server.pathArg(0);
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

     for (uint8_t j = 0; j <= 15; j++) {
      char b = lastfile[j];
      prgbuffer[j+2] = b;
       }
     File c2nFile = SD.open(lastfile, "r");
   int i = 18;
    while (c2nFile.available()) {
     byte inputchar = c2nFile.read();
     prgbuffer[i] = inputchar;
     i++;
    }
    uint8_t xlow = i & 0xff;
    uint8_t xhigh = (i >> 8);
     prgbuffer[0] =  xhigh;
     prgbuffer[1] =  xlow;
     c2nFile.close();  
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
  
void setup(void) {
     setup_ports();
      pinMode (net_tape, INPUT_PULLUP);
      pinMode (files_down, INPUT_PULLUP);
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

if (debug) { Serial.begin(115200);
             Serial.println("SD Card initialized.");
           }          
 
   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, password);

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
  server.on(UriRegex("/(.*)"), HTTP_PUT, handleCreate, handleCreateProcess);
  server.onNotFound(handleNotFound);
  server.begin();
if (debug) { Serial.println("HTTP server started"); }
  root = SD.open("/");
  lcd.print(".");
  printDirectory(root, 0);
  lcd.setCursor(0, 0);
if (debug) {  Serial.println("done!"); }
  lcd.print("C2Net active !");
  digitalWrite(senseport, HIGH);
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

void loop(void) {
  
if (cardreader == false) {  
 
                           server.handleClient();
                           delay(100);
                            digitalWrite(senseport, LOW);
                           delay(100);
                            digitalWrite(senseport, HIGH);
                           delay(100);
  
                            if(digitalRead(motorport)== HIGH){ loader(prgbuffer); }   

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
   
                           if (digitalRead(net_tape) == LOW) { 
                               delay(200); cardreader = false;
                               lcd.setCursor(0, 0);
                               lcd.print("C2Net active !   ");
                              }
  
  
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
                                String lastfile = loadfile;
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
                               }
                              }
                            }
}
