
#include "U8glib.h"
#include <SPI.h>

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);  

#include <Wire.h>
#include <TEA5767N.h>
TEA5767N radio = TEA5767N(); 

#include <IRremote.h>
int RECV_PIN = 2;
int LED_PIN = 13;
IRrecv irrecv(RECV_PIN);
decode_results results;

const double radioON_OFF = 0xFFA25D;
const double kabayar = 0xFF906F;
const double kabayar1 = 0xFFE01F;
const double inceayar = 0xFF02FD;
const double inceayar1 = 0xFF9867;


double frequency = 88.00; 
double topFrequency = 110.00;
double bottomFrequency = 88.00;
double intervalFreq = 0.01; 
int maxPresets; 
double stationsMHZ[7]; 
String stations[7]; 
//
int presetCounter = 0; 
int favourite; 
int sl; 
boolean mute = true;
String stringFrequency;
String stringStation;
String stringSignal;
boolean stereo= false;
String rawData;
boolean IRsignal = false;
int a;
boolean stationName = true; 

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  irrecv.enableIRIn(); 
  radio.selectFrequency(bottomFrequency); 
  pinMode(3,INPUT_PULLUP); 
  digitalWrite(3, HIGH);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  // bir kaç sık kullanılan radyo kanallarını buraya ekledim
  stationsMHZ[1] = 88.58;
  stations[1] = "Kral Pop"; 
  stationsMHZ[2] = 95.5;
  stations[2] = "Polis Radyosu"; 
  stationsMHZ[3] = 89.8;
  stations[3] = "Radyo 7"; 
  stationsMHZ[4] = 98.20;
  stations[4] = "101.8"; 
  stationsMHZ[5] = 101.1;
  stations[5] = "Radyo Megasite"; 
  stationsMHZ[6] = 95.0;
  stations[6] = "TRT Haber";
  //
  maxPresets = 6; 
  favourite = 6;
  presetCounter = favourite;
  selectStation();

}

void loop() {
  if(digitalRead(3) == 0){
    // picture loop
    u8g.firstPage();  
    do {
      drawSetupCalibrate();
    } while( u8g.nextPage() );
    calibrateIR();
  }

  if (irrecv.decode(&results)) {
      digitalWrite(LED_PIN, HIGH);
    if(results.value == kabayar){
      presetCounter = presetCounter + 1;
      if(presetCounter > maxPresets){
        presetCounter = 1; 
      }
      stationName = true;
      selectStation();
      }

    if(results.value == kabayar1){
      presetCounter = presetCounter - 1;
      if(presetCounter <1){
        presetCounter = maxPresets; 
      }
      stationName = true;
      selectStation();
    }

    if(results.value == radioON_OFF){
      if(mute){
        radio.mute();
        radio.setStandByOff();
      }
      else{
        radio.turnTheSoundBackOn();
      }
      mute = !mute;
    }

    if(results.value == inceayar){
      frequency = frequency + intervalFreq;
      if(frequency > topFrequency){
        frequency = bottomFrequency; 
      }
      stationName = false; 
      selectStation2();
    } 
       
    
    if(results.value == inceayar1){
      frequency = frequency - intervalFreq;
      if(frequency < bottomFrequency){
        frequency = topFrequency; 
      }
      stationName = false; 
      selectStation2();
    }    

    irrecv.resume(); 
    digitalWrite(LED_PIN, LOW); 

  }

  printStereoStatus();
  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() );
} 



void selectStation() {
  frequency = stationsMHZ[presetCounter];
  radio.mute();
  radio.selectFrequency(frequency); 
  radio.turnTheSoundBackOn();
}

void selectStation2() {
  radio.mute();
  radio.selectFrequency(frequency);
  radio.turnTheSoundBackOn();
}

void printStereoStatus() {
  sl = radio.getSignalLevel();
  if (radio.isStereo()) {
    stereo = true;
  } else {
    stereo = false;
  }
}

void draw(void){
  u8g.setFont(u8g_font_profont12); 
  u8g.drawStr(40,8, "FM Radio");
  if(mute){     
    stringFrequency = String(radio.readFrequencyInMHz()); 
    stringFrequency = stringFrequency + "mhz";
    a = stringFrequency.length();
    a = 18 - a;  
    a = a*7;  
    a = (a/2) + 1; 
    const char* newFrequency = (const char*) stringFrequency.c_str();
    u8g.setFont(u8g_font_profont15); 
    u8g.drawStr(a,30, newFrequency);
    u8g.setFont(u8g_font_profont12);
    if (stationName){
     stringStation = stations[presetCounter]; 
    }
    else{stringStation = "fine tune";}
    a = stringStation.length();
    a = 21 - a;  
    a = a*6;  
    a = (a/2) + 1; 
    const char* newStation = (const char*) stringStation.c_str();
    u8g.drawStr(a,42,newStation);
  
    if(stereo){
      u8g.drawStr(5,17,"Stereo");
    }
    else{
      u8g.drawStr(5,17,"Mono");
    }
    if(sl <10){
      stringSignal = "Signal: " + String(sl);
      const char* newSignal = (const char*) stringSignal.c_str();
      u8g.drawStr(70,17,newSignal);
    }

   
    u8g.drawLine(4,58,124,58);
    for(int f = 4; f<128; f=f+40){
      u8g.drawLine(f,52,f,58);
    }
    for(int f = 4; f<128; f=f+8){
      u8g.drawLine(f,55,f,58);
    }   
    u8g.drawLine(((frequency-80)*4)+4,53,((frequency-80)*4)+4,63);
    u8g.setFont(u8g_font_profont10);
    u8g.drawStr(0,52,"80"); 
    u8g.drawStr(40,52,"90");
    u8g.drawStr(78,52,"100");
    u8g.drawStr(114,52,"110"); 
  }
  else{
    u8g.drawStr(23,30,"Radio sesizde");
  }
  
}

void calibrateIR(){

  while(digitalRead(3) == 0){
    if (irrecv.decode(&results)) {
      digitalWrite(LED_PIN, HIGH); 
      
      u8g.firstPage();  
      do {
        drawSetup();
      } while( u8g.nextPage() );
    }
    delay(500);
    digitalWrite(LED_PIN, LOW);
  }
}

void drawSetup(){
  radio.mute(); 
  u8g.setFont(u8g_font_profont12); 
  u8g.drawStr(15,8, "IR tarafından gönderilen kodlar");
  u8g.drawLine(0,15,128,15);
  u8g.drawLine(0,55,128,55); 
  rawData = String(results.value);
  const char* newRawData = (const char*) rawData.c_str();
  u8g.setFont(u8g_font_profont15);
  u8g.drawStr(35,40,newRawData);
  irrecv.resume();
}
void drawSetupCalibrate(){
  u8g.setFont(u8g_font_profont12); 
  u8g.drawStr(15,8, "IR tarafından gönderilen kodlar");
  u8g.drawStr(20,40,"Veri bekleniyor");
  u8g.drawLine(0,15,128,15);
  u8g.drawLine(0,55,128,55); 
}
