void menu()
{
  int i,j,num_option=3;
  oled.clear();
  while(DEBUGGING){
    i=constrain(map(analogRead(A7),0,1023,1,num_option*5), 1,num_option*5);
    if(i>3 && i<=num_option*2){
      oled_print(0,0,"      Settings       ");
      if (btnpressed){
        while(btnpressed);
        settings();  
      }
    }else if(i>num_option*2 && i<=num_option*3){
      oled_print(0,0,"    Calibrate Line   ");
      if (btnpressed){
        while(btnpressed);
        calibrate_line('B');
      }
    }else if(i>num_option*3 && i<=num_option*4){
      oled_print(0,0,"    Hardware Check    ");
      if (btnpressed){
        while(btnpressed);
        hardware_check();
      }
    }else if(i>num_option*4 && i<=num_option*5){
      oled_print(0,0,"       Dashboard      ");
      if (btnpressed){
        while(btnpressed);
        break; 
      } 
    }else{
      oled.clear();
    }
  }
  oled.clear();
} 
void hardware_check()
{ 
  int i,num_option=7;
  oled.clear();
  while(DEBUGGING){
    i=constrain(map(analogRead(A7),0,1023,1,num_option*6), 1,num_option*6);
    if(i>num_option && i<=num_option*2){
      oled_print(0,0,"   check IR (raw)    ");
      if (btnpressed){
        while(btnpressed);
        while(DEBUGGING){
          checkIR(1);
          if(btnpressed){while(btnpressed);break;}
        }
      }
    }else if(i>num_option*2 && i<=num_option*3){
      oled_print(0,0,"   check IR (cal)    ");
      if (btnpressed){
        while(btnpressed);
        while(DEBUGGING){
          checkIR(0);
          if(btnpressed){while(btnpressed);break;}
        }
      }
    }else if(i>num_option*3 && i<=num_option*4){
      oled_print(0,0,"  Left Motor check  ");
      if (btnpressed){
        while(btnpressed);
        oled_print(0,0,"Left Motor forward");
        while(DEBUGGING){
          setMotors(100,0);
          if(btnpressed){while(btnpressed);break;}
        }oled.clear();setMotors(0,0);
      }
    }else if (i>num_option*4 && i<=num_option*5){
      oled_print(0,0,"  Right Motor check   ");
      if (btnpressed){
        while(btnpressed);
        oled_print(0,0,"Right Motor forward");
        while(DEBUGGING){
          setMotors(0,100);
          if(btnpressed){while(btnpressed);break;}
        }oled.clear();setMotors(0,0);
      }
    }
    else if(i>num_option*5 && i<=num_option*6){
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
  int i,num_option=3;
   oled.clear();
   while(DEBUGGING){
     i=constrain(map(analogRead(A7),0,1023,1,num_option*7), 1,num_option*7);
     if(i<=num_option*2){
      oled_print(0,0,"      Set Speed       ");
      if (btnpressed){
        while(btnpressed);
         set_speed1();
        oled.clear();
      }
     }else if(i<=num_option*3){
       oled_print(0,0,"      Set Hand       ");
      if (btnpressed){
        while(btnpressed);
        set_hand();
        oled.clear();
      }
     }else if(i<=num_option*3){
       oled_print(0,0,"      Set PID       ");
      if (btnpressed){
        while(btnpressed);
        set_mode();
        oled.clear();
      }
     }else if(i<=num_option*4){
       oled_print(0,0,"      Set Line      ");
      if (btnpressed){
        while(btnpressed);
        set_line();
        oled.clear();
      }
     }else if(i<=num_option*5){
       oled_print(0,0,+"     <<< Back      ");
        if (btnpressed){
          while(btnpressed);
          break;
        }
      }
   }oled.clear();  
}
void set_speed1(){
  int i,j,num_option=15,op[14]={130,140,150,160,170,180,190,200,210,220,230,240,250};
   oled.clear();
   while(DEBUGGING){
     i=constrain(map(analogRead(A7),0,1023,1,num_option), 1,num_option);
     oled.setCursor(0,7);oled.print("   Current = "+String(EEPROM.read(ad_baseSpd))+"     ");
     for(j=1;j<13;j++){
     if(j>=i-1){
     oled.setCursor(0,0);oled.print("       Set = "+String(op[i])+"     ");
      if (btnpressed){
        while(btnpressed);
        base_spd=op[i];
        EEPROM.update(ad_baseSpd,base_spd);
        oled.clear();
        oled_print(0,3,"     ** saved **     ");
        delay(500);
        oled.clear();
       }
      }
     }
    }
}

void set_hand(){
  int s_delay=60, anim_delay=20, op_delay=60;
  while(1){
    int k=0,n=2, op_count;
    char op[]={'R','L'};
    oled.setCursor(0,7);oled.print("   Current = "+String(char(EEPROM.read(ad_hand)) )+"     ");
    for(op_count=0;op_count<n;op_count++){
      oled.setCursor(0,0);oled.print("      Hand = "+String(op[op_count])+"       ");
      for(k=0; k<op_delay; k++){if(btnpressed){hand=op[op_count];EEPROM.update(ad_hand,hand);break;}delay(10);}
      if(k<op_delay){oled.setCursor(0,1);oled.print("     ** SAVED **     ");delay(500);break;}
    }if(op_count<n)break;
  }while(btnpressed);
}
void set_line(){
  int s_delay=60, anim_delay=20, op_delay=60;
  while(1){
    int k=0,n=2, op_count;
    char op[]={'B','W'};
    oled.setCursor(0,7);oled.print("   Current = "+String(char(EEPROM.read(ad_line)) )+"     ");
    for(op_count=0;op_count<n;op_count++){
      oled.setCursor(0,0);oled.print("      Line = "+String(op[op_count])+"       ");
      for(k=0; k<op_delay; k++){if(btnpressed){line=op[op_count];EEPROM.update(ad_line,line);break;}delay(10);}
      if(k<op_delay){oled.setCursor(0,1);oled.print("     ** SAVED **     ");delay(500);break;}
    }if(op_count<n)break;
  }while(btnpressed);
}
void set_mode(){
  int s_delay=60, anim_delay=20, op_delay=60;
  while(1){
    int k=0,n=3, op_count;
    int op[]={0,1,2};
    oled.setCursor(0,7);oled.print("   Current = "+String(EEPROM.read(ad_taan))+"     ");
    for(op_count=0;op_count<n;op_count++){
      oled.setCursor(0,0);oled.print("       Set = "+String(op[op_count])+"     ");
      for(k=0; k<op_delay; k++){if(btnpressed){taan=op[op_count];EEPROM.update(ad_taan,taan);break;}delay(10);}
      if(k<op_delay){oled.setCursor(0,1);oled.print("     ** SAVED **     ");delay(500);break;}
    }if(op_count<n)break;
  }while(btnpressed);
}
