void inch(long runtime)
{

}
void turnPID(char dir)
{
  if (dir == 'L') setMotors(-tSpd, 0);
  else if(dir == 'R') setMotors(0, -tSpd);
  while(getPattern(9) == "000000000");
  setMotors(0, 0);
  dir='S';
}
void turn(char dir,byte sen)
{
  if (dir == 'L') setMotors(-tSpd, tSpd/2.38);
  else if(dir == 'R') setMotors(tSpd/2.38, -tSpd);
  if(sen==5)while(getPattern(5) == "00000");
  else if(sen==7)while(getPattern(7) == "0000000");
  else if(sen==9)while(getPattern(9) == "000000000");
  setMotors(0, 0);
  dir='S';
}
void turnDeg(char dir, int deg, bool adjust)
{
  if(deg==180)tSpd=100;
  
  int rightHandAdjust=0;
  if(dir=='R')rightHandAdjust=10;

  if (dir == 'L') setMotors(-tSpd, tSpd);
  else if(dir == 'R') setMotors(tSpd, -tSpd);
  if(deg==45){
    delay(90);
  }else if(deg==90){
    delay(156);
  }else if(deg==135){
    delay(230+rightHandAdjust);
  }else if(deg==180){
    delay(315);
  }
  if(adjust==true){while(getPattern(7) == "0000000");}
  setMotors(0,0);
  dir='S';
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
void brake(unsigned long runtime)
{
  int bSpd;
  bSpd=map( (runtime), 180,800, 140,-100 );
  bSpd=constrain(bSpd*0.75,0,140);
  setMotors(bSpd,bSpd);
  oled_print(0,6,"       Braking       ");
  delay(100);
//  oled_print(0,4,"rt="+String(runtime)+"     spd="+String(max_spd)+"   ");
//  oled_print(0,5,"bSpd="+String(bSpd)+"    ");
}
void halt()
{
  setMotors(0, 0);
  max_spd=initSpd;
}
void followUntil(int spd, int duration)
{
  unsigned long t0;
  max_spd=spd;
  if(duration){
    t0=millis();
    while((millis()-t0) < duration){readLine();pos=getPos(7);pid(pidMode);}
  }else{
    readLine();pos=getPos(pidMode);pid(pidMode);
  }
}
bool scanGap(unsigned long duration)
{
  unsigned long t0, tmp_spd;
  t0=millis();
  tmp_spd=map(max_spd,initSpd,255, 160,80);
  setMotors(tmp_spd,tmp_spd);
  while(millis()-t0 < duration){
    readLine();extractPatterns();
    if(patt9 != "000000000" || patt55!="00000")return true;
  }
  return false;
}

void pid(int mode) {
  static long P, I = 0, D, err, lasterr = 0, _spd;
  KD = max_spd/10;
  err = pos - max_spd;
  P = err;
  I+=err;
  if(err==0)I=0;
  D = err - lasterr;
  lasterr = P;
  _spd = KP * P + KI * I + KD * D;
  if (mode==1)setMotors(min(max(max_spd+_spd, -max_spd), max_spd), min(max(max_spd-_spd, -max_spd), max_spd));
   else if(mode==0)setMotors(min(max(max_spd+_spd, 0), 255), min(max(max_spd-_spd, 0), 255));
  // else if(mode==0)setMotors(min(max(max_spd+_spd, -(max_spd/3)), 255), min(max(max_spd-_spd, -(max_spd/3)), 255));
   else if(mode==2)setMotors(min(max(_spd, -max_spd), max_spd), min(max(-_spd, -max_spd), max_spd));
}
