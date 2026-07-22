uint8_t sensorRead(int i)
{
  PORTC &= B11110000;PORTC |= i;
  // delayMicroseconds(100);
  return analogReadFast(s_pin)/4;
}
void readLine()
{
  int tmp;
  for(int i=0; i<NUM_SENSORS_F; i++){ 
    tmp = map(sensorRead(f5[scan_order_f[i]]) , sminf[scan_order_f[i]],smaxf[scan_order_f[i]], -5 *_F, 15 *_F);
    arf[scan_order_f[i]] = constrain(tmp, 0,10*_F);
    if(line=='W'){
       arf[scan_order_f[i]]=10*_F - arf[scan_order_f[i]];
       drf[scan_order_f[i]] = arf[scan_order_f[i]] < (10)*_F ? 0:1;
    }else{
       drf[scan_order_f[i]] = arf[scan_order_f[i]] < 3 *_F ?0:1;
    }
  }
  for(int i=0; i<NUM_SENSORS_B; i++){
    tmp = map(sensorRead(b9[scan_order_b[i]]),sminb[scan_order_b[i]],smaxb[scan_order_b[i]], -7 *_F, 17 *_F);
    arb[scan_order_b[i]]=constrain(tmp, 0,10*_F);
    if(line=='W'){
       arb[scan_order_b[i]]=10*_F - arb[scan_order_b[i]];
       drb[scan_order_b[i]] = arb[scan_order_b[i]] < (10)*_F ? 0:1;
    }else{
       drb[scan_order_b[i]] = arb[scan_order_b[i]] < 3 *_F ? 0:1;
    }
  }
}
void extractPatterns()
{
  String pattern="";
  patt55=pattern+drf[0]+drf[1]+drf[2]+drf[3]+drf[4];
  patt5=pattern+drb[2]+drb[3]+drb[4]+drb[5]+drb[6];
  patt7=pattern+drb[1]+drb[2]+drb[3]+drb[4]+drb[5]+drb[6]+drb[7];
  patt9=pattern+drb[0]+drb[1]+drb[2]+drb[3]+drb[4]+drb[5]+drb[6]+drb[7]+drb[8];   
  
  if(DEBUGGING) {
    oled_print(6,5,String(patt55[0])+" "+String(patt55[1])+" "+String(patt55[2])+" "+String(patt55[3])+" "+String(patt55[4]));
    oled_print(6,6,String(patt9));
  }
}
String getPattern(int mode)
{
  String pattern="";
  readLine();
  if(mode==3){
    return (pattern+drb[3]+drb[4]+drb[5]);
  }else if(mode==5){
    return (pattern+drb[2]+drb[3]+drb[4]+drb[5]+drb[6]);
  }else if(mode==7){
    return (pattern+drb[1]+drb[2]+drb[3]+drb[4]+drb[5]+drb[6]+drb[7]);
  }else if(mode==9){
    return (pattern+drb[0]+drb[1]+drb[2]+drb[3]+drb[4]+drb[5]+drb[6]+drb[7]+drb[8]);
  }else if(mode==55){
    return pattern+drf[0]+drf[1]+drf[2]+drf[3]+drf[4];
  }
}
long getPos(int n)
{
  long _pos, sum=0,w_sum=0, weight, count=0;
  weight=max_spd;
  if(DEBUGGING) oled_printBar(arb,4-(n/2),4+(n/2),10*_F);
  for(int i=4-(n/2); i<=4+(n/2); i++){sum+=arb[i];}
  if(n==5){
    w_sum=arb[2]*0+arb[3]*weight*0.5+arb[4]*weight+arb[5]*weight*1.5+arb[6]*weight*2;
  }else if(n==7){
    w_sum=arb[1]*0+arb[2]*weight*0.33+arb[3]*weight*0.66+arb[4]*weight+arb[5]*weight*1.34+arb[6]*weight*1.67+arb[7]*weight*2;
  }else{
    w_sum=arb[0]*0+arb[1]*0+arb[2]*weight*0.33+arb[3]*weight*0.66+arb[4]*weight+arb[5]*weight*1.34+arb[6]*weight*1.67+arb[7]*weight*2+arb[8]*weight*2;
  }
  _pos=w_sum/sum;
  if(sum==0)_pos=lastPos;
  if(DEBUGGING) {oled.clearField(9,3,6);oled_print(9,3, String(pos));}
  lastPos=_pos;
  return _pos;
}

void checkIR(bool raw){
  int k=2;
  if(raw){
    oled_print( 1,0, String( sensorRead(f5[0]) )+" "+String( sensorRead(f5[1]) )+" "+String( sensorRead(f5[2]) )+" "+String( sensorRead(f5[3]) )+" "+String( sensorRead(f5[4]) )+"     " );
    for(int i=0,j=5;i<NUM_SENSORS_B;i++){
      if(i==4||i==5)k=k+3;
      oled_print( k,j, String(sensorRead(b9[i])) + (j==2 ? " " : "    ") );
      if(i==5)j++;
      k++;
      if(i<3)j--;
      if(i>5)j++;
    }
  }else{
    readLine();
    oled_print( 1,0, String( arf[0] )+" "+String( arf[1] )+" "+String( arf[2] )+" "+String( arf[3] )+" "+String( arf[4] )+"     " );
    for(int i=0,j=5;i<NUM_SENSORS_B;i++){
      if(i==4||i==5)k=k+3;
      oled_print( k,j, String(arb[i]) + (j==2 ? " " : "    ") );
      if(i==5)j++;
      k++;
      if(i<3)j--;
      if(i>5)j++;
    }
  }
}
