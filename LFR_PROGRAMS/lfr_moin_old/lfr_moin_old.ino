#include <NewPing.h>
#include <EEPROM.h>
#include <Wire.h>
#include "SSD1306AsciiAvrI2c.h"
#include "avdweb_AnalogReadFast.h"

#define RPATT ( (!drb[1]) && (drb[3]||drb[4]) && (drb[7]) )
#define LPATT ( (drb[1]) && (drb[4]||drb[5]) && (!drb[7]) )

int tSpd=130;
#define initSpd 50
#define _F 10

#define KP 1.4
float KD=25;
#define KI 0.000

int max_spd, base_spd, print_delay,halt_delay;
char line, hand, dir='S';
int pidMode, wallDist=12, objDist=12, rampUpSpd,rampDownSpd;

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
void turn(char,byte=5);void turnPID(char);void turnDeg(char,int=0,bool=false);void halt();void brake(long=0);void inch(long=0);
void menu();void settings();

void setup() {
  pinMode(lmf,OUTPUT);pinMode(lmb,OUTPUT);pinMode(rmf,OUTPUT);pinMode(rmb,OUTPUT);
  pinMode(12,INPUT_PULLUP);pinMode(2,INPUT_PULLUP);pinMode(5,OUTPUT);
  DDRC|=B00001111;
  Serial.begin(115200);
  oled.begin(&SH1106_128x64, 0x3C);oled.setFont(System5x7);
  readMemory();
  oled.clear();
  max_spd=initSpd;
  buz(1);
  delay(500);
  buz(0);
}

void lop(){
  turnDeg(hand,90);
  if(DEBUGGING){
    menu();
  }
  delay(2000);
}

void loop() {
  int tCount = 0, yCount = 0, count = 0;
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
    pos=getPos(9);
    
    if(dir!='S' && count>50){
      dir='S';
    }else{
      count++;
    }
    if(hand=='R'){
      if(drb[7] || drb[8]){
        dir='R';
        count=0;
      }
      if(drf[0]){
        // oled_print(0,0,"<<<<<<<              ");
        dir='L';
        count=0;
      }
    }else{
      if(drb[0] || drb[1]){
        dir='L';
        count=0;
      }
      if(drf[4]){
        // oled_print(0,0,"              >>>>>>>");
        dir='R';
        count=0;
      }
    }

    if(DEBUGGING)continue;
    if(first_time){
      oled.clear();
      oled_print(0,3,"  <<<<<  GO!  >>>>>  ");
      while(!btnpressed && !DEBUGGING);while(btnpressed);
      oled.clear();delay(200);
      first_time=false;
      lastMillis=millis();
      continue;
    }

    // while(1){ //     test loop
    //   followWall('R',0.30);  // right  amar 0.40
    //   // followWall('L',0.40);  // left    amar 0.50
    //   halt();delay(500);
    //   lastMillis=millis();continue;
    // }
      
    if(patt55=="11111" && patt9=="111111111"){ // black box
      END:
      setMotors(150,150);delay(70);
      halt();
      if(hasWall(30)){
        line='B';
        buz(1);delay(100);buz(0);
        followWall('R',0.40);  // right  amar 0.40
        halt();delay(500);
        lastMillis=millis();continue;
      }
      while(btnpressed);
      oled.clear();oled_print(2,0," MISSION COMPLETE! ");oled_print(0,5,"  Press to restart  ");
      while(!btnpressed);while(btnpressed);
      dir='S';
      oled.clear();
      first_time=true;
      lastMillis=millis();
      continue;
    }

    // if(patt55=="10101"){  // 1 0 1 0 1
    //   oled.clear();oled_print(0,0,"      1 0 1 0 1      ");
    //   halt();
    //   readLine();extractPatterns();
    //   if(patt9=="000000000"){
    //     // while(!btnpressed);while(btnpressed);
    //     goto TTT;
    //   }else if(patt55=="11011"){
    //     // while(!btnpressed);while(btnpressed);
    //     lastMillis=millis();
    //     goto INV;
    //   }else{
    //     // while(!btnpressed);while(btnpressed);
    //     turnDeg(hand,90);turn(hand);delay(3000);
    //     lastMillis=millis();
    //     continue;
    //   }
    // }
    
//    if(patt55=="11011"){ // inverse line
//      INV:
//      oled.clear();oled_print(0,0,"       inverse       ");
//      line = (line=='B')?'W':'B';
//      wlinePassed=true;
//      continue;
//    }

    if(patt55=="01010"){  // Y
      runtime = millis()-lastMillis; delay(20); brake(runtime);
      halt();oled.clear();oled_print(0,0,"       Y     Y       ");
      // while(!btnpressed);while(btnpressed);
      turnDeg(hand,45);turn(hand,7);
      lastMillis=millis();continue;
    }
    
    if(patt55=="11111"){ // T or + 
      runtime = millis()-lastMillis; delay(30); brake(runtime);
      oled.clear();oled_print(0,0,"           T         ");
      halt();
      readLine();extractPatterns();
      if(patt9=="000000000" && patt55=="00000"){ // T
        TTT:
        oled.clear();oled_print(0,0,"        T T T        ");
        if(line=='B')turnDeg(hand);
        turn(hand,7);
        lastMillis=millis();continue;
      }else if(patt55=="00100"){ // +
        PPP:
        oled.clear();oled_print(0,0,"        +++++        ");
        turn(hand,7);
        lastMillis=millis();continue;
      }else if(patt55=="11011"){
        // while(!btnpressed);while(btnpressed);
        lastMillis=millis();
//        goto INV;
      }else if(patt9=="111111111"){goto END;}
      halt();oled.clear();oled_print(0,0,"       unknown 1     ");delay(100);    
      turn(hand);
      delay(100);
      continue  ;
      // while(!btnpressed);while(btnpressed);
      lastMillis=millis();
      continue;
    }


    if(hand=='R'){ // Right hand rule
      if(drf[0]){  // left pattern
      }
      if(patt7=="0001111"){
        runtime = millis()-lastMillis;delay(20);brake(runtime);
         halt(); oled.clear();oled_print(7,3,patt7 );delay(20);
        // if(!drb[6]){
        //   turnDeg('R',135);
        //   turn('R');
        //   lastMillis=millis();continue;
        // }
        if(getPattern(7)=="0000000"){    // only right
          oled.clear();oled_print(0,0,"                  RRR");
          turnDeg('R',90);turn('R',7);   // use 7 sensor
        }else{    // something infront
          oled.clear();oled_print(0,0,"         FFF      RRR");delay(50);
         turnDeg('R',135);
        }
        lastMillis=millis();continue;
      }
    }
    else{ // Left hand rule
      if(drf[4]){  // right pattern
      }
      if(LPATT){
        runtime = millis()-lastMillis;delay(20);brake(runtime);
        // halt(); oled.clear();oled_print(7,3,patt7 );
        // if(!drb[2]){
        //   turnDeg('L',135);
        //   // while(!btnpressed);while(btnpressed);
        //   turn('L');
        //   lastMillis=millis();continue;
        // }
        if(getPattern(7)=="0000000"){    // only left
          oled.clear();oled_print(0,0,"LLL                  ");
          turnDeg('L',45);turn('L',7);    // use 7 sensor
       }
       // else{    // something infront
//          oled.clear();oled_print(0,0,"LLL      FFF         ");delay(50);
//          turnDeg('L',90,true);
//        }
        lastMillis=millis();continue;
      }
    } // left hand rule end


    if (patt9=="000000000" && (!pidMode || 0<pos && pos<max_spd*2) ) { // line gap or dead end
      if(dir=='S'){
        runtime = millis()-lastMillis;brake(runtime);
        halt();
        // while(!btnpressed);while(btnpressed);
        if(line=='B' && getPattern(9)!="000000000"){
          max_spd = 210;
          buz(1);
          for(int i=0;i<700;i++){readLine();extractPatterns(); pos=getPos(9);pid(0);}
          buz(0);
          lastMillis=millis();continue;
        }

        // if(scanGap(200))continue; // line gap
        // dead end
        halt();oled.clear();oled_print(0,0,"          U          ");delay(220);
        turn(hand,9);  // use 9 sensor
        lastMillis=millis();
        continue;
      }else{
        runtime = millis()-lastMillis;brake(runtime);
        halt();oled.clear();oled_print(0,0,"XX                 XX");delay(50);
        turn(dir,7);  // use 9 sensor
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

  } // endwhile;
}
