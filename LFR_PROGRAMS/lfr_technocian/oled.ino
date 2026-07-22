void oled_print(int col, int row, String text, int zoom, bool clr)
{
  if (clr)oled.clear();
  //  if(zoom==2)oled.set2X();
  oled.setCursor(col, row);
  oled.print(text);
  //  oled.set1X();
}
void oled_println(int col, int row, String text, int zoom, bool clr)
{
  if (clr)oled.clear();
  //  if(zoom==2)oled.set2X();
  oled.setCursor(col, row);
  oled.println(text);
  //  oled.set1X();
}

void oled_printBar(uint8_t* a, int l, int r, int max_val)
{
  int tmp;
  for (int i = l; i <= r; i++) {
    tmp = map(a[i], 0, max_val, 129, 135);
    oled.setCursor(6 + i, 7);
    oled.write(constrain(tmp, 129, 135));
  }
}
void buz(bool state){
  digitalWrite(13,state);
}
void beep(byte n){
  for(byte i=0; i<n; i++){
    buz(1);
    delay(50);
    buz(0);
    delay(50);
  }
}
void wait(){
  buz(0);
  while(!btnpressed);while(btnpressed);
}
