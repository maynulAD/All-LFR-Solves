#include <NewPing.h>
#include <EEPROM.h>
#include <Wire.h>
#include "SSD1306AsciiAvrI2c.h"
#include "avdweb_AnalogReadFast.h"

#define RPATT ( (!drb[1]) && (drb[3]||drb[4]) && (drb[7]) )
#define LPATT ( (drb[1]) && (drb[4]||drb[5]) && (!drb[7]) )

int tSpd=115;
#define initSpd 50
#define _F 10

#define KP 1.4
float KD=15;
#define KI 0.000

int max_spd, base_spd, print_delay,halt_delay;
char line, hand, dir='S';
int pidMode, wallDist=15, objDist=12, rampUpSpd,rampDownSpd;

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
String patt3,patt5,patt55, lastPatt55="00100",patt7,patt9;
long pos,lastPos=0;

void oled_print(int,int,String,int=1,bool=false);void oled_println(int,int,String,int=1,bool=false);
void oled_printBar(long*,int,int,int);
void indicator();
void calibrate_line();void calibrate_wall();void calibrate_obj();
bool hasWall(long);char detectWall(char);void followWall(char,float);void followWallBoth();void checkSonar();int readSonar(byte,int);
bool hasObject(long);void avoidObject(char,float);bool isRamp(long);void avoidObjectBlind(char, float, int=0);
void readLine();void extractPatterns();String getPattern(int=0);long getPos(int);void checkIR(bool=false);
void setMotors(int,int);void pid(int=0);void followUntill(int, int=0);
void turn(char,byte=5);void turnPID(char);void turnDeg(char,int=0,bool=false);void halt();void brake(long=0);void inch(long=0);
void menu();void settings();

void setup() {
  pinMode(lmf,OUTPUT);pinMode(lmb,OUTPUT);pinMode(rmf,OUTPUT);pinMode(rmb,OUTPUT);
  pinMode(12,INPUT_PULLUP);pinMode(2,INPUT_PULLUP);pinMode(13,OUTPUT);
  DDRC|=B00001111;
  Serial.begin(115200);
  oled.begin(&SH1106_128x64, 0x3C);oled.setFont(System5x7);
  readMemory();
  oled.clear();
  max_spd=initSpd;
}

void lop(){
  if(DEBUGGING){
    menu();
    wait();
  }
  turnDeg(hand,90);
  delay(2000);
}

void loop() {
  int tCount = 0, yCount = 0, count = 0, uCount=0, pCount=0;
  bool obj = false, ramp=false, scan=true;;
  bool wallPassed = false, inv = false;
  unsigned long lastMillis = 0,stepMillis=0, runtime = 0;
  int eyeL,eyeR;
  bool first_time=true;

  while(1){
    if(DEBUGGING){
      halt();buz(0);
      if(!first_time)oled.clear();
      if(btnpressed){while(btnpressed);menu();}
      oled_print(0, 0, "Hand="+String(hand)+", Line="+String(line)+", dir="+String(dir)+"   ");
      oled_print(0,2, "ramp=" + String(ramp) + "          obj=" + String(obj));
      oled_print(0,1,String(sonarL.ping_cm())+"   ");oled_print(9,1,String(sonarF.ping_cm())+"   ");oled_print(18,1,String(sonarR.ping_cm())+"   ");
      oled_print(0,3,"S=" + String(base_spd)+"   ");oled_print(16,3,"pid=" + String(pidMode));
      first_time=true;
    }

    ramp = isRamp();
    obj=hasObject(34);
    readLine();
    extractPatterns();
    pos=getPos(9);
    
    if(dir!='S' && count>30){
      dir='S';
      buz(0);
    }else{
      count++;
    }
    if(hand=='R'){
      if(drb[7] || drb[8]){
        dir='R';
        count=0;
        buz(1);
      }
      if(drf[0]){
        // oled_print(0,0,"<<<<<<<              ");
        dir='L';
        count=0;
        buz(1);
      }
    }else{
      if(drb[0] || drb[1]){
        dir='L';
        count=0;
        buz(1);
      }
      if(drf[4]){
        // oled_print(0,0,"              >>>>>>>");
        dir='R';
        count=0;
        buz(1);
      }
    }

    if(DEBUGGING)continue;
    if(first_time){
      oled.clear();
      oled_print(0,3,"  <<<<<  GO!  >>>>>  ");
      buz(0);
      while(!btnpressed && !DEBUGGING);while(btnpressed);
      oled.clear();delay(200);
      first_time=false;
      lastMillis=millis();
      continue;
    }
    
    if(patt55=="11111" && patt9=="111111111"){ // black box
      END:
//      setMotors(150,150);delay(70);
      halt();buz(0);
      while(btnpressed);
      oled.clear();oled_print(2,0," MISSION COMPLETE! ");oled_print(0,5,"  Press to restart  ");
      wait();
      dir='S';
      uCount=1;
      oled.clear();
      first_time=true;
      lastMillis=millis();
      continue;
    }

//    if(ramp){
//      max_spd=rampUpSpd;
//      buz(1);
//      for(int i=0;i<600;i++){readLine();pos=getPos(7);pid(1);}
//      oled.clear();oled_print(0,0,"        Ramp         ");
//      buz(0);
//      max_spd=rampDownSpd;
//      while(!isRamp()){readLine();pos=getPos(7);pid(1);}
//      for(int i=0;i<100;i++){readLine();pos=getPos(7);pid(1);}
//      halt();delay(100);
//      // turn(hand);
//      lastMillis=millis();
//      continue;
//    }

//    if(obj){
//      runtime = millis() - lastMillis; brake(runtime);
//      halt();oled.clear();oled_print(0,0,"       object       ");delay(500);
//      turnDeg(hand,90);
//      avoidObject(hand, 0.4);
//      delay(100);
//      turn(hand,9);
//      lastMillis = millis();continue;
//    }

     if(inv && patt55=="10101"){  // 1 0 1 0 1
      delay(150);
       halt();oled.clear();oled_print(0,0,"      1 0 1 0 1      ");
       turnDeg(hand,90);
       turnDeg(hand,45,true);
       lastMillis=millis();continue;
     }
    
    if(patt55=="11011"){ // inverse line
      INV:
      oled.clear();oled_print(0,0,"       inverse       ");
      line = (line=='B')?'W':'B';
      if(line=='B'){
        inv=true;
      }
      hand='L';
      continue;
     }

    if(patt55=="01010"){  // Y
      runtime = millis()-lastMillis; delay(2); brake(runtime);
      halt();oled.clear();oled_print(0,0,"       Y     Y       ");
      turnDeg(hand,45);turn(hand,7);
      lastMillis=millis();continue;
    }
    
    if(patt55=="11111"){ // T or + 
      if(tCount==0 || tCount==1){
        for(int i=0;i<200;i++){readLine();pos=getPos(7);pid(1);}
        tCount++;
        lastMillis=millis();continue;
      }
      runtime = millis()-lastMillis; delay(2); brake(runtime);
      halt();oled.clear();oled_print(0,0,"           T         ");
      readLine();extractPatterns();
      if(patt9=="000000000" && patt55=="00000"){ // T
        TTT:
        oled.clear();oled_print(0,0,"        T T T        ");
        turn(hand,7);
        lastMillis=millis();continue;
      }else if(patt55=="00100" || (!drb[0]&&!drb[1] && (drb[3]||drb[4]||drb[5]) && !drb[7]&&!drb[8]) ){ // +
        PPP:
        oled.clear();oled_print(0,0,"        +++++        "); 
        turnDeg(hand,90,true);
        lastMillis=millis();continue;
      }else if(patt9=="111111111"){goto END;}
      halt();oled.clear();oled_print(0,0,"       unknown 1     ");delay(50);
      turn(hand);delay(100);
      lastMillis=millis();
      continue;
    }


    if(hand=='R'){ // Right hand rule
      if(drf[0]){  // left pattern
      }
      if(RPATT){
        runtime = millis()-lastMillis;delay(20);brake(runtime);
        if(getPattern(7)=="0000000"){    // only right
          oled.clear();oled_print(0,0,"                  RRR");
//          turnDeg('R',45);
          turn('R',7);   // use 7 sensor
        }else{    // something infront
          oled.clear();oled_print(0,0,"         FFF      RRR");delay(50);
          turnDeg('R',90,true);
        }
        lastMillis=millis();continue;
      }
    }
    else{ // Left hand rule
      if(drf[4]){  // right pattern
      }
      if(LPATT){
        if(inv){
          scan=false;
        }
        runtime = millis()-lastMillis;delay(20);brake(runtime);
        if(getPattern(7)=="0000000"){    // only left
          oled.clear();oled_print(0,0,"LLL                  ");
//          turnDeg('L',45);
          turn('L',7);    // use 7 sensor
        }else{    // something infront
          oled.clear();oled_print(0,0,"LLL      FFF         ");delay(50);
          turnDeg('L',90,true);
        }
        lastMillis=millis();continue;
      }
    } // left hand rule end
    

    if (patt9=="000000000" && (!pidMode || 0<pos && pos<max_spd*2) ) { // line gap or dead end
      if(dir=='S'){
        if(uCount==0){
          setMotors(150,150);delay(700);
          halt();delay(1000);
          uCount=1;
        }
        
        if(scan && scanGap(100)){
          continue;
        }
        // dead end
        halt();oled.clear();oled_print(0,0,"          U          ");delay(200);
        if(hasWall(25)){
          followWall('R',0.4);
          hand='R';
          lastMillis=millis();continue;
        }
        turnDeg('R',45);
        turn('R',9);  // use 9 sensor
        lastMillis=millis();
        continue;
      }else{
        runtime = millis()-lastMillis;brake(runtime);
        halt();oled.clear();oled_print(0,0,"XX                 XX");delay(50);
//        turnDeg(dir,45);
        turn(dir,9);  // use 7 sensor
        dir='S';
        lastMillis=millis();
        continue;
      }
    }


    if(millis()-stepMillis > 5){
      max_spd+=5;
      max_spd=min(max_spd,base_spd);
      stepMillis=millis();
    }
    
    pid(1);

    lastPatt55=patt55;
  } // endwhile;
}
