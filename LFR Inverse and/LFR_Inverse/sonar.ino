bool hasObject(long dis)
{
  int front = 0;
  front = sonarF.ping_cm();
  if (front > 4 && front < dis)return true;
  else return false;
}
bool hasWall(long dis)
{
  int left = 0, right = 0;
  right = sonarR.ping_cm();
  left = sonarL.ping_cm();
//  if (right > 4 && right < dis)return true;
  if (left > 4 && left < dis)return true;
  return false;
}
char detectWall(char hand)
{
  int left = 0, right = 0, dis = 30;
  right = sonarR.ping_cm();
  left = sonarL.ping_cm();
  if (hand == 'R') {
    if (right > 4 && right < dis)return 'R';
    if (left > 4 && left < dis)return 'L';
  } else {
    if (left > 4 && left < dis)return 'L';
    if (right > 4 && right < dis)return 'R';
  }
}

void avoidObject(char h, float ratio)
{
  int minDist = (objDist * 10) - 50, maxDist = (objDist * 10) + 50, minSpd, maxSpd = 150;
  int distance, err, lasterr = 0, P, D, spd;
  minSpd = maxSpd * ratio;
  while (1) {
//  while (getPattern(5) == "00000") {
    if (h == 'R')distance = round(sonarL.ping() * 0.343 / 2);
    else distance = round(sonarR.ping() * 0.343 / 2);
    if (distance < minDist && distance != 0)distance = minDist;
    if (distance > maxDist || distance == 0)distance = maxDist;
    distance = constrain(map(distance, minDist, maxDist, 0, 100), 0, 100);
    err = distance - 50;
    P = err;
    D = err - lasterr;
    lasterr = err;
    spd = 4 * P + 9 * D;
    if (h == 'R')setMotors(min(max(maxSpd - spd, minSpd), maxSpd), min(max(maxSpd + spd, minSpd), maxSpd));
    else setMotors(min(max(maxSpd + spd, minSpd), maxSpd), min(max(maxSpd - spd, minSpd), maxSpd));
    delay(10);
  }
  delay(100);
  setMotors(0, 0);
}

void avoidObjectBlind(char h, float ratio, int duration)
{
  int maxSpd=250, lastMillis=millis();
  while (millis()-lastMillis<duration || getPattern(5) == "00000") {
    if(h == 'R')setMotors(maxSpd*ratio, maxSpd);
    else setMotors(maxSpd, maxSpd*ratio);
  }
  delay(100);
  setMotors(0,0);
}

void followWall(char h, float ratio)
{
  int minDist = (wallDist*10) - 50, maxDist = (wallDist*10) + 50, minSpd, maxSpd = 150;
  int front, left, right, distance, err, lasterr = 0, P, D, spd;
  minSpd = maxSpd * ratio;
//  while (1) {
  while (getPattern(7) == "0000000") {
    if (h == 'R')distance = round(sonarR.ping() * 0.343 / 2);
    else distance = round(sonarL.ping() * 0.343 / 2);
    if (distance < minDist && distance != 0)distance = minDist;
    if (distance > maxDist || distance == 0)distance = maxDist;
    distance = constrain(map(distance, minDist, maxDist, 0, 100), 0, 100);
    err = distance - 50;
    P = err;
    D = err - lasterr;
    lasterr = err;
    spd = 5 * P + 9 * D;
    if (h == 'R')setMotors(min(max(maxSpd + spd, minSpd), maxSpd), min(max(maxSpd - spd, minSpd), maxSpd));
    else setMotors(min(max(maxSpd - spd, minSpd), maxSpd), min(max(maxSpd + spd, minSpd), maxSpd));
    delay(1);
  }
  delay(100);
//  setMotors(0, 0);
}

//void followWallBoth()
//{
//  int minDist = 50, maxDist = 250, minSpd = 120, maxSpd = 200;
//  int front, left, right, distanceR, distanceL, err, lasterr = 0, P, D, spd;
//  while (getPattern(5) == "00000") {
//    distanceR = round(sonarR.ping() * 0.343 / 2);
//    distanceL = round(sonarL.ping() * 0.343 / 2);
//    if (distanceR < minDist && distanceR != 0)distanceR = minDist;
//    if (distanceR > maxDist || distanceR == 0)distanceR = maxDist;
//    if (distanceL < minDist && distanceL != 0)distanceL = minDist;
//    if (distanceL > maxDist || distanceL == 0)distanceL = maxDist;
//    distanceR = constrain(map(distanceR, minDist, maxDist, 0, 100), 0, 100);
//    distanceL = constrain(map(distanceL, minDist, maxDist, 0, 100), 0, 100);
//    err = distanceR - distanceL;
//    P = err;
//    D = err - lasterr;
//    lasterr = err;
//    spd = 5 * P + 4 * D;
//    setMotors(min(max(maxSpd + spd, minSpd), maxSpd), min(max(maxSpd - spd, minSpd), maxSpd));
//    //    setMotors(min(max(maxSpd-spd, minSpd), maxSpd), min(max(maxSpd+spd, minSpd), maxSpd));
//    delay(10);
//  }
//  delay(200);
//  setMotors(0, 0);
//}
void checkSonar(){
  oled_print(0,0,"Sonar Test");
  oled_print(0,1, "F="+String(sonarF.ping_cm())+"   ");
  oled_print(0,2, "R="+String(sonarR.ping_cm())+"   ");
  oled_print(0,3, "L="+String(sonarL.ping_cm())+"   ");
}
int readSonar(byte pin, int defaultValue){
  uint16_t val = pulseIn(pin, HIGH, 7000);
  if (val < 100) return defaultValue;
  return val;
}
bool isRamp(){
  PORTC &= B11110000;PORTC |= 9;
  return analogReadFast(s_pin)<512 ? 1:0;
}
