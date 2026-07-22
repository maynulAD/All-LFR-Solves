#include <NewPing.h>
#include <EEPROM.h>
#include <Wire.h>
#include "SSD1306AsciiAvrI2c.h"
#include "avdweb_AnalogReadFast.h"

#define ILPATT (drb[2] && (!drb[3] || !drb[4] || !drb[5]) && drb[6])

#define halt_delay 2000
#define print_delay 1
#define tSpd 120
#define MINSPD 100

#define KP 1.5
float KD=25;
#define KI 0.0001

int max_spd=250, base_spd=120;
int wallDist=14, objDist=12, taan;

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
#define ad_baseSpd 46
#define ad_taan 47
#define ad_line 48

int sminf[] = {255, 255, 255, 255, 255}, smaxf[] = {0, 0, 0, 0, 0};
int sminb[] = {255, 255, 255, 255, 255, 255, 255, 255, 255}, smaxb[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int s_pin = A6, arf[NUM_SENSORS_F],arb[NUM_SENSORS_B];
byte drf[NUM_SENSORS_F], drb[NUM_SENSORS_B];
byte f5[]={14,13,12,11,10}, b9[]={15,0,1,2,3,4,5,6,7};
byte scan_order_f[]={2,3,1,4,0}, scan_order_b[]={4,5,3,6,2,7,1,8,0};

NewPing sonarL(8,8,400), sonarF(7,7,400), sonarR(4,4,400);
SSD1306AsciiAvrI2c oled;
String patt,pattb5, lastPatt = "", patt3, lastPatt3 = "010", patt5, lastPatt5 = "00100", patt7, lastPatt7 = "0001000", patt9, lastPatt9 = "000010000";
long pos,lastPos=0;
bool obFound=false;
int fs1=0,fs5=0;
char line='B', hand='L', dir='S';

void oled_print(int,int,String,int=1,bool=false);void oled_println(int,int,String,int=1,bool=false);
void oled_printBar(long*,int,int,int);
void indicator();
void calibrate_line(char);void calibrate_wall();void calibrate_obj();
bool hasWall(long);char detectWall(char);void followWall(char,float);void followWallBoth();void checkSonar();int readSonar(byte,int);
bool hasObject(long);void avoidObject(char,float);bool isRamp(long);void avoidObjectBlind(char, float, int=0);
void readLine();void extractPatterns();String getPattern(int=0);long getPos(int);void checkIR(bool=false);
void setMotors(int,int);void pid(int=0);
void turn(char);void turnPID(char);void turnDeg(char,int=0,bool=false);void halt();void brake(long=0);void inch(long=0);
void menu();void settings();

void setup() {
  pinMode(lmf,OUTPUT);pinMode(lmb,OUTPUT);pinMode(rmf,OUTPUT);pinMode(rmb,OUTPUT);
  pinMode(12,INPUT_PULLUP);pinMode(2,INPUT_PULLUP);
  DDRC|=B00001111;
  Serial.begin(115200);
  oled.begin(&SH1106_128x64, 0x3C);oled.setFont(System5x7);
  hand=EEPROM.read(ad_hand);
  line=EEPROM.read(ad_line);
  base_spd=EEPROM.read(ad_baseSpd);
  for(int i=0; i<NUM_SENSORS_F; i++){sminf[i]=EEPROM[ad_sminf+i];smaxf[i]=EEPROM[ad_smaxf+i];}
  for(int i=0; i<NUM_SENSORS_B; i++){sminb[i]=EEPROM[ad_sminb+i];smaxb[i]=EEPROM[ad_smaxb+i];}
  oled.clear();
  delay(500);
}

void lop(){
  // turnDeg(hand,135);
  // delay(1000);
  // setMotors(80,80);
  // checkIR();
  // readLine();
  // extractPatterns();
  // pos=getPos(9);
  // delay(300);  
}

void loop() {
  int tCount = 0, pCount = 0, count = 0;
  bool obj = false, ramp=false;
  bool wallPassed = false, wlinePassed = false;
  long lastMillis = 0,stepMillis=0;
  int eyeL,eyeR;
  bool first_time=true;
  
  while(1){
    if(DEBUGGING){
      halt();
      if(btnpressed){while(btnpressed);menu();}
      oled_print(0, 0, "Hand="+String(hand)+", Line="+String(line)+", dir="+String(dir)+"   ");
      oled_print(0,2, "ramp=" + String(ramp) + "          obj=" + String(obj));
      oled_print(0,1,String(sonarL.ping_cm())+"  "+String(sonarF.ping_cm())+"  "+String(sonarR.ping_cm())+"                ");
      oled_print(0,3,"sp=" +String(base_spd));
      first_time=true;
      lastMillis=millis();
    }

    ramp = isRamp();
    // obj=hasObject(20);
    readLine();
    extractPatterns();
    pos=getPos(7);
    
    if(DEBUGGING)continue;
    if(first_time){
      oled.clear();
      oled_print(0,0,"<<<<<<<< GO! >>>>>>>>");
      while(!btnpressed);
      delay(400);
      first_time=false;
    }
    
    // if(patt9=="111111111"){ // black box
    //   END:
    //   halt();
    //   while(btnpressed);
    //   oled.clear();oled_print(2,0," MISSION COMPLETE! ");oled_print(0,5,"  Press to restart  ");
    //   while(!btnpressed);while(btnpressed);
    //   dir='S';
    //   oled.clear();
    //   delay(1000);
    //   lastMillis=millis();
    //   first_time=true;
    //   // max_spd=100;
    //   continue;
    // }

    // if(ramp){
    //   max_spd=250;
    //   for(int i=0;i<300;i++){readLine();pos=getPos(7);pid(1);}
    //   // halt();delay(3000);
    //   while(!isRamp()){max_spd=120;readLine();pos=getPos(7);pid(1);}
    //   for(int i=0;i<80;i++){readLine();pos=getPos(7);pid(1);}
    //   halt();delay(3000);
    // }
    
    // if(patt5=="00101"||patt5=="01101"){ // right acute angle
    //   setMotors(max_spd,max_spd);delay(100);
    //   halt();oled_print(0,0,"             >>>>>RRR");delay(100);
    //   turnDeg('R',90,true);
    //   continue;
    // }else if(patt5=="10100"||patt5=="10110"){
    //   setMotors(max_spd,max_spd);delay(100);
    //   halt();oled_print(0,0,"             >>>>>RRR");delay(100);
    //   turnDeg('L',90,true);
    //   continue;
    // }

    if(obj){
      halt();oled_print(0,0,"       obj5       ");delay(1000);
      turnDeg('R',90);
      avoidObjectBlind('R',0.4);
      turn('R');
      continue;
    }

    if(patt5=="10101"){ // ^
     
      setMotors(150,150);delay(500);
      halt();oled_print(0,0,"       paiche       ");delay(2000);
      continue;
    }

    if(patt5=="11111"){
      setMotors(max_spd,max_spd);delay(50);
      halt();delay(100);
      patt5=getPattern(5);
      if(patt5=="00000"){ // T
        oled_print(0,0,"         TTTTT       ");delay(100);
        turnDeg(hand,90,true);
      }else if(patt5=="00100"||patt5=="01110"||patt5=="01100"||patt5=="00110"){ // +
//        hand='R';
//        continue;
          turn(hand);   //  tuhin
      }else{ // Y
        turn(hand);
      }
      
      max_spd=100;
      continue;
    }else if( (!drb[1]) && (drb[3]||drb[4]||drb[5]) && (drb[7]) ){ //  right
      setMotors(max_spd,max_spd);delay(50);
      if(getPattern(5)=="00000"){ // 90 deg
        halt();oled_print(0,0,"        tuhin      RRR");delay(100);
//     while(getPattern(5) == "00000");  
        turn('R');
      }else{ // right & front 
        if(hand=='R'){
          halt();oled_print(0,0,"                  RRR");delay(100);
          turnDeg('R',90,true);
        }else{
          continue;
        }
      }
      max_spd=100;
      continue;
    }else if( (drb[1]) && (drb[3]||drb[4]||drb[5]) && (!drb[7]) ){ // sharp left
      setMotors(max_spd,max_spd);delay(100);
      if(getPattern(5)=="00000"){ // 90 deg
        halt();oled_print(0,0,"LLL   tuhin              ");delay(50);
        turn('L');
      }else{ // right & front 
        if(hand=='L'){
          halt();oled_print(0,0,"LLL                  ");delay(100);
          turnDeg('L',90,true);
        }else{
          continue;
        }
      }
      max_spd=100;
      continue;
    }

//    if (patt7=="0000000" && (0<pos && pos<max_spd*2) ) { // line gap or dead end
//      setMotors(150,150); delay(130);
//      if(getPattern(5)=="00000"){
//        halt();oled_print(0,0,"          U          ");delay(20);
//        speed_control();
//        turn(hand);
//      }else{continue;}
//      
//      max_spd=100;
//      continue;
//    }

 if (patt7=="0000000" && (0<pos && pos<max_spd*2) ) { // line gap or dead end
      setMotors(150,150); delay(130);
      if(getPattern(5)=="00000"){
        halt();oled_print(0,0,"          U          ");delay(20);
//        speed_control();
        turn(hand);
      }else{continue;}
      
      max_spd=100;
      continue;
    }

    // max_spd=base_spd;
    if(millis()-stepMillis > 5){
      max_spd+=10;
      max_spd=min(max_spd,base_spd);
      stepMillis=millis();
    }
    pid(1);
    

    lastPatt = patt;
    lastPatt3 = patt3;
    lastPatt5 = patt5;
    lastPatt7 = patt7;
    lastPatt9 = patt9;
  } // endwhile;
}
