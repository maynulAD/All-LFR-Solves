void inch(long runtime)
{

}
void turnPID(char dir)
{
  if(dir=='R')lastPos=max_spd*2;
  else lastPos=0;
  pos=lastPos;
  do{
    readLine();
    extractPatterns();
    pos=getPos(9);
    pid(-1);
  }while(patt9=="000000000");
}
void turn(char dir)
{
  if (dir == 'L') setMotors(-base_spd, base_spd/2.38);
  else if(dir == 'R') setMotors(base_spd/2.38, -base_spd);
  while(getPattern(5) == "00000");
  setMotors(0, 0);
  // oled_print(0,0,"turn("+String(dir)+")");delay(3000);
}
void turnDeg(char dir, int deg, bool adjust)
{
  int rightHandAdjust=0;
  if(dir=='R')rightHandAdjust=10;
  
  if (dir == 'L') setMotors(-base_spd, base_spd/2.38);
  else if(dir == 'R') setMotors(base_spd/2.38, -base_spd);
  if(deg==45){
    delay(120);
  }else if(deg==90){
    delay(210-rightHandAdjust);
  }else if(deg==135){
    delay(275-rightHandAdjust);
  }else if(deg==180){
    delay(350-rightHandAdjust);
  }
  if(adjust==true){while(getPattern(5) == "00000");}
  setMotors(0,0);
}
void setMotors(int spdL, int spdR)
{
  spdL = constrain(spdL, -255, 255);
  spdR = constrain(spdR, -255, 255);
  if (spdL < 0) {analogWrite(lmf, 0);   analogWrite(lmb, -spdL);}
  else          {analogWrite(lmf, spdL);analogWrite(lmb, 0);}
  if (spdR < 0) {analogWrite(rmf, 0);   analogWrite(rmb, -spdR);}
  else          {analogWrite(rmf, spdR);analogWrite(rmb, 0);}
}
void brake(long runtime)
{
  
}
void halt()
{
  setMotors(0, 0);
}

void pid(int mode) {
  static long P, I = 0, D, err, lasterr = 0, spd;
  // KD = max_spd/60;
  err = pos - max_spd;
  P = err;
  I+=err;
  if(err==0)I=0;
  D = err - lasterr;
  lasterr = P;
  spd = KP * P + KI * I + KD * D;
  if (mode==1)setMotors(min(max(max_spd+spd, -max_spd), max_spd), min(max(max_spd-spd, -max_spd), max_spd));
  // else if(mode==0)setMotors(min(max(max_spd+spd, 0), 255), min(max(max_spd-spd, 0), 255));
  // else if(mode==0)setMotors(min(max(max_spd+spd, -(max_spd/3)), 255), min(max(max_spd-spd, -(max_spd/3)), 255));
  // else if(mode==-1)setMotors(min(max(0+spd, -max_spd), max_spd), min(max(0-spd, -max_spd), max_spd));
}
