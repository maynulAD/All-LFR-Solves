void menu()
{
  int i,k=10,n=4;
  oled.clear();
  while(DEBUGGING){
    i=constrain(map(analogRead(A7),0,1023,0,n*k), 1,n*k);
    if(i>0 && i<k){
      oled_print(0,0,"      Settings       ");
      if (btnpressed){
        while(btnpressed);
        settings();  
      }
    }else if(i>k && i<2*k){
      oled_print(0,0,"    Calibrate Line   ");
      if (btnpressed){
        while(btnpressed);
        calibrate_line();
      }
    }else if(i>2*k && i<3*k){
      oled_print(0,0,"    Hardware Check    ");
      if (btnpressed){
        while(btnpressed);
        hardware_check();
      }
    }else if(i>3*k && i<=4*k){
      oled_print(0,0,"       Dashboard      ");
      if (btnpressed){
        while(btnpressed);
        break; 
      } 
    }
  }
  oled.clear();
} 
void hardware_check()
{
  int i,k=10,n=5;
  oled.clear();
  while(DEBUGGING){
    i=constrain(map(analogRead(A7),0,1023,0,n*k), 1,n*k);
    if(i>0 && i<k){
      oled_print(0,0,"   check IR (raw)    ");
      if (btnpressed){
        while(btnpressed);
        oled.clear();
        while(DEBUGGING){
          checkIR(1);
          if(btnpressed){while(btnpressed);break;}
        }oled.clear();
      }
    }else if(i>k && i<2*k){
      oled_print(0,0,"   check IR (cal)    ");
      if (btnpressed){
        while(btnpressed);
        oled.clear();
        while(DEBUGGING){
          checkIR(0);if(btnpressed){while(btnpressed);break;}
        }oled.clear();
      }
    }else if(i>2*k && i<3*k){
      oled_print(0,0,"  Left Motor check  ");
      if (btnpressed){
        while(btnpressed);
        oled.clear();oled_print(0,0,"Left Motor forward");
        while(DEBUGGING){
          setMotors(50,0);
          if(btnpressed){while(btnpressed);break;}
        }oled.clear();setMotors(0,0);
      }
    }else if (i>3*k && i<4*k){
      oled_print(0,0,"  Right Motor check   ");
      if (btnpressed){
        while(btnpressed);
        oled.clear();oled_print(0,0,"Right Motor forward");
        while(DEBUGGING){
          setMotors(0,50);
          if(btnpressed){while(btnpressed);break;}
        }oled.clear();setMotors(0,0);
      }
    }else if(i>4*k && i<=5*k){
      oled_print(0,0,"       <<< Back      ");
      if (btnpressed){while(btnpressed);break;}
    }
  }oled.clear();
}
void calibrate_lines()
{
  int i,num_option=7;
   oled.clear();
   while(DEBUGGING){
     i=constrain(map(analogRead(A7),0,1023,1,num_option*6), 1,num_option*6);
     if(i<=num_option*2){
       oled_print(0,0,"Calibrate black line");
     }else if(i<=num_option*3){
       oled_print(0,0,"Calibrate White line");
     }else if(i<=num_option*4){
       oled_print(0,0,"Back                ");
        if (btnpressed){
          while(btnpressed);
          break; 
          }
       }
   }oled.clear();
}
void settings()
{
  int i,k=10,n=7;
   oled.clear();
   while(DEBUGGING){
     i=constrain(map(analogRead(A7),0,1023,0,n*k), 1,n*k);
     if(i>0 && i<k){
      oled_print(0,0,"        Speed         ");
      if (btnpressed){
        while(btnpressed);
        set_speed();
        oled.clear();
      }
     }else if(i>k && i<2*k){
       oled_print(0,0,"        Hand         ");
      if (btnpressed){
        while(btnpressed);
        set_hand();
        oled.clear();
      }
     }else if(i>2*k && i<3*k){
       oled_print(0,0,"      PID Mode       ");
      if (btnpressed){
        while(btnpressed);
        set_mode();
        oled.clear();
      }
     }else if(i>3*k && i<4*k){
       oled_print(0,0,"        Line         ");
      if (btnpressed){
        while(btnpressed);
        set_line();
        oled.clear();
      }
     }else if(i>4*k && i<5*k){
       oled_print(0,0,"    Ramp Up Speed    ");
        if (btnpressed){
          while(btnpressed);
          set_rampUpSpeed();
          oled.clear();
        }
      }else if(i>5*k && i<6*k){
       oled_print(0,0,"   Ramp Down Speed   ");
        if (btnpressed){
          while(btnpressed);
          set_rampDownSpeed();
          oled.clear();
        }
      }else if(i>6*k && i<=7*k){
       oled_print(0,0,"      <<< Back       ");
        if (btnpressed){
          while(btnpressed);
          break;
        }
      }
   }oled.clear();  
}

void set_speed(){
  int i,n=16, tmp;
  oled.setCursor(0,7);oled.print("   Current = "+String(EEPROM.read(ad_baseSpd))+"     ");
  while(DEBUGGING){
    i=constrain(map(analogRead(A7),0,1000,0,n), 0,n);
    tmp=constrain(100+i*10, 100,255);
    oled.setCursor(0,3);oled.print("       Set = "+String(tmp)+"     ");
    if (btnpressed){
      while(btnpressed);
      base_spd=tmp;
      EEPROM.update(ad_baseSpd,base_spd);
      break;
    }
  }
}
void set_rampUpSpeed(){
  int i,n=14, tmp;
  oled.setCursor(0,7);oled.print("   Current = "+String(EEPROM.read(ad_rampUpSpd))+"     ");
  while(DEBUGGING){
    i=constrain(map(analogRead(A7),0,1000,0,n), 0,n);
    tmp=constrain(120+i*10, 120,255);
    oled.setCursor(0,3);oled.print("       Set = "+String(tmp)+"     ");
    if (btnpressed){
      while(btnpressed);
      rampUpSpd=tmp;
      EEPROM.update(ad_rampUpSpd,rampUpSpd);
      break;
    }
  }
}
void set_rampDownSpeed(){
  int i,n=14, tmp;
  oled.setCursor(0,7);oled.print("   Current = "+String(EEPROM.read(ad_rampDownSpd))+"     ");
  while(DEBUGGING){
    i=constrain(map(analogRead(A7),0,1000,0,n), 0,n);
    tmp=constrain(120+i*10, 120,255);
    oled.setCursor(0,3);oled.print("       Set = "+String(tmp)+"     ");
    if (btnpressed){
      while(btnpressed);
      rampDownSpd=tmp;
      EEPROM.update(ad_rampDownSpd,rampDownSpd);
      break;
    }
  }
}
void set_hand(){
  int i,n=4;
  char tmp;
  oled.setCursor(0,7);oled.print("   Current = "+String(char(EEPROM.read(ad_hand)))+"     ");
  while(DEBUGGING){
    i=constrain(map(analogRead(A7),100,1000,0,n), 0,n);
    tmp = i%2 ? 'R':'L';
    oled.setCursor(0,3);oled.print("       Set = "+String(tmp)+"     ");
    if (btnpressed){
      while(btnpressed);
      hand=tmp;
      EEPROM.update(ad_hand,hand);
      break;
    }
  }
}
void set_line(){
  int i,n=4;
  char tmp;
  oled.setCursor(0,7);oled.print("   Current = "+String(char(EEPROM.read(ad_line)))+"     ");
  while(DEBUGGING){
    i=constrain(map(analogRead(A7),100,1000,0,n), 0,n);
    tmp = i%2 ? 'B':'W';
    oled.setCursor(0,3);oled.print("       Set = "+String(tmp)+"     ");
    if (btnpressed){
      while(btnpressed);
      line=tmp;
      EEPROM.update(ad_line,line);
      break;
    }
  }
}
void set_mode(){
  int i,n=6, tmp;
  int op[]={0,1,2};
  oled.setCursor(0,7);oled.print("   Current = "+String(EEPROM.read(ad_pidMode))+"     ");
  while(DEBUGGING){
    i=constrain(map(analogRead(A7),100,1000,0,n), 0,n);
    tmp=op[i%3];
    oled.setCursor(0,3);oled.print("       Set = "+String(tmp)+"     ");
    if (btnpressed){
      while(btnpressed);
      pidMode=tmp;
      EEPROM.update(ad_pidMode,pidMode);
      break;
    }
  }
}

void readMemory(){
  hand=EEPROM.read(ad_hand);
  // objDist=EEPROM.read(ad_objDist);
  // wallDist=EEPROM.read(ad_wallDist);
  base_spd=EEPROM.read(ad_baseSpd);
  pidMode=EEPROM.read(ad_pidMode);
  line=EEPROM.read(ad_line);
  print_delay=EEPROM.read(ad_printDelay);
  rampUpSpd=EEPROM.read(ad_rampUpSpd);
  rampDownSpd=EEPROM.read(ad_rampDownSpd);
  
  for(int i=0; i<NUM_SENSORS_F; i++){sminf[i]=EEPROM[ad_sminf+i];smaxf[i]=EEPROM[ad_smaxf+i];}
  for(int i=0; i<NUM_SENSORS_B; i++){sminb[i]=EEPROM[ad_sminb+i];smaxb[i]=EEPROM[ad_smaxb+i];}
}
