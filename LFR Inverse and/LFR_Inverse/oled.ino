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

void oled_printBar(int* a, int l, int r, int max_val)
{
  for (int i = l; i <= r; i++) {
    oled.setCursor(6 + i, 6);
    //    oled.set2X();
    oled.write(constrain(map(a[i], 0, max_val, 129, 135), 129, 135));
    //    oled.set1X();
  }
}
