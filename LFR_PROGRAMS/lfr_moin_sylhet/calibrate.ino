void calibrate_line()
{
  bool flag=false;
  delay(200);
  oled.clear();oled_print(0,0,"Calibrating Line",true);
  int tmp, calSpd = 60, calPeriod = 200;

  for (int count = 0; count < calPeriod; count+=1) {
    if(count > calPeriod/4 && count < calPeriod*3/4)setMotors(-calSpd, calSpd);
    else setMotors(calSpd, -calSpd);
    for (int i = 0; i < NUM_SENSORS_F; i++) {
      tmp = sensorRead(f5[i]);
      if(tmp < sminf[i]) sminf[i] = tmp;
      if(tmp > smaxf[i]) smaxf[i] = tmp;
    }
    for (int i = 0; i < NUM_SENSORS_B; i++) {
      tmp = sensorRead(b9[i]);
      if(tmp < sminb[i]) sminb[i] = tmp;
      if(tmp > smaxb[i]) smaxb[i] = tmp;
    }
    oled.setCursor(count/10,1);oled.write(136);
    delay(10);
  }
  delay(10);setMotors(0, 0);
  while(!btnpressed){readLine();extractPatterns();}
  while(btnpressed);
  while(1){
    int k=0;
    oled_print(0,0,"  Ignore?  ");
    for(k=0; k<100; k++){if(btnpressed){while(btnpressed);flag=false;break;}delay(10);}if(k<100)break;
    oled_print(0,0,"  Update?  ");
    for(k=0; k<100; k++){if(btnpressed){while(btnpressed);flag=true;break;}delay(10);}if(k<100)break;
  }
  if(flag){
    for (int i = 0; i < NUM_SENSORS_F; i++) {
      EEPROM.update(ad_sminf+i, sminf[i]);
      EEPROM.update(ad_smaxf+i, smaxf[i]);
    }
    for (int i = 0; i < NUM_SENSORS_B; i++) {
      EEPROM.update(ad_sminb+i, sminb[i]);
      EEPROM.update(ad_smaxb+i, smaxb[i]);
    }
    oled_print(0,0," * Saved! * ");
  }
  delay(1000);
  oled.clear();
}
