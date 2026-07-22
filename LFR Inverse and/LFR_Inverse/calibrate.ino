void calibrate_line(char l)
{
  bool flag=false;
  line=l;
  delay(200);
  oled_print(0,0,"Calibrating Line"+l,true);
  int af[NUM_SENSORS_F], ab[NUM_SENSORS_B], calSpd = 60, calPeriod = 300;

  for (int count = 0; count < calPeriod; count+=1) {
    if(count > calPeriod/4 && count < calPeriod*3/4)setMotors(-calSpd, calSpd);
    else setMotors(calSpd, -calSpd);
    for (int i = 0; i < NUM_SENSORS_F; i++) {
      af[i] = sensorRead(f5[i]);
      if(af[i] < sminf[i]) sminf[i] = af[i];
      if(af[i] > smaxf[i]) smaxf[i] = af[i];
    }
    for (int i = 0; i < NUM_SENSORS_B; i++) {
      ab[i] = sensorRead(b9[i]);
      if(ab[i] < sminb[i]) sminb[i] = ab[i];
      if(ab[i] > smaxb[i]) smaxb[i] = ab[i];
    }
    oled.setCursor(count/10,1);oled.write(136);
    delay(10);
  }
  setMotors(0, 0);
  while(!btnpressed){oled.clear();oled.print(String(l));readLine();extractPatterns();}
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
