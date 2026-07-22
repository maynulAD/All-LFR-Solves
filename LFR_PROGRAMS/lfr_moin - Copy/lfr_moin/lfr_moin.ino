#include <NewPing.h>
#include <EEPROM.h>
#include <Wire.h>
#include "SSD1306AsciiAvrI2c.h"
#include "avdweb_AnalogReadFast.h"

#define tSpd 130
#define initSpd 50
#define _F 10

#define KP 1.45
float KD=9;
#define KI 0.000

int max_spd, base_spd, print_delay,halt_delay;
char line, hand, dir='S';
int pidMode, wallDist, objDist, rampUpSpd,rampDownSpd;

#define s_pin A6
#define NUM_SENSORS_F 5
#define NUM_SENSORS_B 9
#define lmf 10
#define lmb 11
#define rmf 9
#define rmb 3
#define DEBUGGING !digitalRead(2)
#define btnpressed !digitalRead(12)
#define ad_sminf 100
#define ad_smaxf 105
#define ad_sminb 122
#define ad_smaxb 131
#define ad_hand 44
#define ad_objDist 45
#define ad_wallDist 46
#define ad_baseSpd 47
#define ad_pidMode 48
#define ad_line 49
#define ad_printDelay 52
#define ad_rampUpSpd 54
#define ad_rampDownSpd 55

int sminf[5] = {255, 255, 255, 255, 255}, smaxf[5] = {0, 0, 0, 0, 0};
int sminb[9] = {255, 255, 255, 255, 255, 255, 255, 255, 255}, smaxb[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t arf[NUM_SENSORS_F],arb[NUM_SENSORS_B];
byte drf[NUM_SENSORS_F], drb[NUM_SENSORS_B];
byte f5[5]={14,13,12,11,10}, b9[9]={15,0,1,2,3,4,5,6,7};
byte scan_order_f[5]={2,3,1,4,0}, scan_order_b[9]={4,5,3,6,2,7,1,8,0};

NewPing sonarL(8,8,400), sonarF(7,7,400), sonarR(4,4,400);
SSD1306AsciiAvrI2c oled;
String patt3,patt5,patt55,patt7,patt9;
long pos,lastPos=0;

void oled_print(int,int,String,int=1,bool=false);void oled_println(int,int,String,int=1,bool=false);
void oled_printBar(long*,int,int,int);
void indicator();
void calibrate_line();void calibrate_wall();void calibrate_obj();
bool hasWall(long);char detectWall(char);void followWall(char,float);void followWallBoth();void checkSonar();int readSonar(byte,int);
bool hasObject(long);void avoidObject(char,float);bool isRamp(long);void avoidObjectBlind(char, float, int=0);
void readLine();void extractPatterns();String getPattern(int=0);long getPos(int);void checkIR(bool=false);
void setMotors(int,int);void pid(int=0);void followUntill(int, int=0);
void turn(char);void turnPID(char);void turnDeg(char,int=0,bool=false);void halt();void brake(long=0);void inch(long=0);
void menu();void settings();

void setup() {
  pinMode(lmf,OUTPUT);pinMode(lmb,OUTPUT);pinMode(rmf,OUTPUT);pinMode(rmb,OUTPUT);
  pinMode(12,INPUT_PULLUP);pinMode(2,INPUT_PULLUP);
  DDRC|=B00001111;
  Serial.begin(115200);
  oled.begin(&SH1106_128x64, 0x3C);oled.setFont(System5x7);
  readMemory();
  oled.clear();
  max_spd=initSpd;
  delay(500);
}

void lop(){
  turnDeg(hand,90);
  if(DEBUGGING){
    menu();
  }
  delay(2000);
}

void loop() {
  int tCount = 0, pCount = 0, count = 0, bSpd;
  bool obj = false, ramp=false;
  bool wallPassed = false, wlinePassed = false;
  unsigned long lastMillis = 0,stepMillis=0, runtime = 0;
  int eyeL,eyeR;
  bool first_time=true;
  
  while(1){
    if(DEBUGGING){
      halt();
      if(!first_time)oled.clear();
      if(btnpressed){while(btnpressed);menu();}
      oled_print(0, 0, "Hand="+String(hand)+", Line="+String(line)+", dir="+String(dir)+"   ");
      oled_print(0,2, "ramp=" + String(ramp) + "          obj=" + String(obj));
      oled_print(0,1,String(sonarL.ping_cm())+"   ");oled_print(9,1,String(sonarF.ping_cm())+"   ");oled_print(18,1,String(sonarR.ping_cm())+"   ");
      oled_print(0,3,"S=" + String(base_spd)+"   ");oled_print(16,3,"pid=" + String(pidMode));
      first_time=true;
    }

    ramp = isRamp();
    obj=hasObject(24);
    readLine();
    extractPatterns();
    pos=getPos(7);
    
    if(DEBUGGING)continue;
    if(first_time){
      oled.clear();
      oled_print(0,3,"  <<<<<  GO!  >>>>>  ");
      while(!btnpressed && !DEBUGGING);
      while(btnpressed);
      oled.clear();
      delay(200);
      first_time=false;
      lastMillis=millis();
      continue;
    }
      
    if(patt55=="11111" && patt9=="111111111"){ // black box
      halt();
      while(btnpressed);
      oled.clear();oled_print(2,0," MISSION COMPLETE! ");oled_print(0,5,"  Press to restart  ");
      while(!btnpressed);while(btnpressed);
      dir='S';
      oled.clear();
      first_time=true;
      lastMillis=millis();
      continue;
    }

    if(ramp){
      max_spd=rampUpSpd;
      for(int i=0;i<400;i++){readLine();pos=getPos(7);pid(1);}
      oled.clear();oled_print(0,0,"        Ramp         ");
      max_spd=rampDownSpd;
      while(!isRamp()){readLine();pos=getPos(7);pid(1);}
      for(int i=0;i<100;i++){readLine();pos=getPos(7);pid(1);}
      halt();delay(500);
    }
     
     if(obj){
       halt();while(!hasObject(15)){readLine();pos=getPos(7);pid(1);}
       halt();oled.clear();oled_print(0,0,"        object       ");delay(200);
       turnDeg(hand,90);
       avoidObjectBlind(hand,0.45,300);
       turn(hand);
//       while(!btnpressed);while(btnpressed);
       lastMillis=millis();
       continue;
    }

    if(line=='W' && patt55=="10101"){
      oled.clear();oled_print(0,0,"       1001001       ");
      halt();delay(100);
      patt9=getPattern(9);
      if(patt9=="000000000"){
        goto TTT;
      }else{
//        while(!btnpressed);while(btnpressed);
        turn(hand);
        lastMillis=millis();
        continue;
      }

    }
    
    if(patt55=="11011"){ // inverse line
      INV:
      oled.clear();oled_print(0,0,"       inverse       ");
      line =(line=='B')?'W':'B';
      continue;
    }else if(patt55=="11111"){ // T or +
      runtime = millis()-lastMillis; delay(30); brake(runtime);
      oled.clear();oled_print(0,0,"           T         ");
      patt55=getPattern(55);
      if(patt55=="00000"){ // T
        TTT:
        oled.clear();oled_print(0,0,"        T T T        ");
        turn(hand);
        lastMillis=millis();
        continue;
      }else if(patt55=="00100"){ // +
        oled.clear();oled_print(0,0,"        +++++        ");
        continue;
      }else if(patt55=="11011"){
        lastMillis=millis();
        goto INV;
      }
      halt();oled.clear();oled_print(0,0,"       unknown 1     ");delay(50);
      turn(hand);
//      while(!btnpressed);while(btnpressed);
      lastMillis=millis();
      continue;
    }
    
//    if(patt5=="11111"){
//      runtime = millis()-lastMillis; brake(runtime);
//      oled.clear();oled_print(0,0,"           T         ");
////      halt();while(!btnpressed);while(btnpressed);
////      lastMillis=millis();
////      continue;
////      setMotors(max_spd,max_spd);delay(50);
//      patt7=getPattern(7);
//      if(patt7=="0000000"){ // T
//        oled.clear();oled_print(0,0,"        TTTTT        ");
//        turn(hand);
//      }else if(!drb[2] && (drb[3]||drb[4]||drb[5]) && !drb[6]){ // +
//        oled.clear();oled_print(0,0,"        +++++        ");
//        continue;
//      }else{ // 
//        halt();oled.clear();oled_print(0,0,"       unknown       ");delay(50);
//        turnDeg(hand,45);
//        while(!btnpressed);while(btnpressed);
//      }
//      lastMillis=millis();
//      continue;
//    }else 

    if( (!drb[1]) && (drb[3]||drb[4]||drb[4]) && (drb[7]) ){ //  right
      runtime = millis()-lastMillis;brake(runtime);
      if(getPattern(5)=="00000"){ // only right
        halt();oled.clear();oled_print(0,0,"                  RRR");
        turn('R');
      }else{ // right & front 
        if(hand=='R'){
          halt();oled.clear();oled_print(0,0,"         FF       RRR");delay(100);
          turnDeg('R',90);
        }else{
          lastMillis=millis();
          continue;
        }
      }
      lastMillis=millis();
      continue;
    }else if( (drb[1]) && (drb[4]||drb[4]||drb[5]) && (!drb[7]) ){ //  left
      runtime = millis()-lastMillis;brake(runtime);
//      setMotors(max_spd,max_spd);delay(50);
      if(getPattern(5)=="00000"){ // only left
        halt();oled.clear();oled_print(0,0,"LLL                  ");
        turn('L');
      }else{ // left & front
        if(hand=='L'){
          halt();oled.clear();oled_print(0,0,"LLL       FF         ");delay(100);
          turnDeg('L',90);
        }else{
          lastMillis=millis();
          continue;
        }
      }
      lastMillis=millis();
      continue;
    }

    if (patt7=="0000000" && (0<pos && pos<max_spd*2) ) { // line gap or dead end      
      if(scanGap(200))continue; // line gap
      // dead end
      halt();oled.clear();oled_print(0,0,"          U          ");delay(200);
      turn(hand);
      lastMillis=millis();
      continue;
    }

    // max_spd=base_spd;
    if(millis()-stepMillis > 5){
      max_spd+=5;
      max_spd=min(max_spd,base_spd);
      stepMillis=millis();
    }
    pid(pidMode);

  } // endwhile;
}
