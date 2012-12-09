//*******************************************************************************************************************
// 								                                 Check Time
//*******************************************************************************************************************
void checktime()
{
  uint8_t temp =0;

  I2C_RX(RTCDS1337,RTC_SEC);
  SecOnes = i2cData & B00001111;                   // 

  SecTens = i2cData & B01110000;                   // 
  SecTens = SecTens >> 4;


  I2C_RX(RTCDS1337,RTC_MIN);
  MinOnes = i2cData & B00001111;                   // 

  MinTens = i2cData & B01110000;                   // 
  MinTens = MinTens >> 4;

  I2C_RX(RTCDS1337,RTC_HOUR);
  UTCHourOnes = i2cData & B00001111;                   // 
  // We only support 24 hour mode
  UTCHourTens = i2cData & B00110000;                   // 
  UTCHourTens = UTCHourTens >> 4;
}

//*******************************************************************************************************************
// 								                                 Check Date
//*******************************************************************************************************************
void checkDate()
{

  int temp = 0;
  I2C_RX(RTCDS1337,RTC_DAY);
  UTCDays = i2cData & B00000111;                   // 

  I2C_RX(RTCDS1337,RTC_MONTH);
  UTCMonthCode = i2cData & B00001111;                   // 

  temp = (i2cData & B00010000) >> 4;
  if(temp)
  {
    UTCMonthCode = UTCMonthCode +10;                           // Convert BCD month into interger month
  }

  I2C_RX(RTCDS1337,RTC_DATE);
  UTCDateOnes = i2cData & B00001111;                   // 
  UTCDateTens = (i2cData & B00110000) >> 4;
  
  I2C_RX(RTCDS1337,RTC_YEAR);
  UTCYearsOnes = i2cData & B00001111;
  UTCYearsTens = (i2cData & B11110000) >> 4;
}


//*******************************************************************************************************************
// 								                                 SET Time - NEW
//*******************************************************************************************************************

void settimeNEW(uint8_t setselect)                     // both min digits or both hour digits (advance one at a time)
{
  uint8_t temp = 0;
  switch(setselect)
  {

  case 1:
    MinOnes = MinOnes +1;
    if(MinOnes >9)
    {
      MinOnes = 0;

      MinTens = MinTens +1;
      if(MinTens >5)
      {
        MinTens = 0;
      }

      //   temp = (MinTens << 4) + MinOnes;
      //   I2C_TX(RTCDS1337,RTC_MIN,temp);
    }

    temp = (MinTens << 4) + MinOnes;
    I2C_TX(RTCDS1337,RTC_MIN,temp);
    I2C_TX(RTCDS1337,RTC_SEC,0); // This will act as a "hack second"
    break;


    // -----------------------------------------------

  case 2:
    UTCHourOnes = UTCHourOnes +1;
  
    // Note: We only support setting time in UTC mode, so there is no 12 hour mode  

    if((UTCHourOnes >9) && (UTCHourTens < 2))
    {
      UTCHourOnes = 0;
      UTCHourTens = UTCHourTens +1;
    }
    
    if((UTCHourTens ==2) && (UTCHourOnes == 4))
    {
      UTCHourOnes = 0;
      UTCHourTens = 0;    
    }
    
    temp = (UTCHourTens << 4) + UTCHourOnes;
    // This is always 24 hour time
    I2C_TX(RTCDS1337,RTC_HOUR,temp);
    break;    

  case 3:
    UTCDays = UTCDays +1 ;
    if(UTCDays>7)
    {
      UTCDays = 1;
    }
    temp = UTCDays & B00000111;                   // 
    I2C_TX(RTCDS1337,RTC_DAY,temp);
    break;

  case 4:
    temp = 0;
    UTCMonthCode = UTCMonthCode +1 ;  
    if(UTCMonthCode >12)
    {
      UTCMonthCode = 1;
    } 
    if(UTCMonthCode>9)
    {
      temp = UTCMonthCode - 10;
      // MonthCode = MonthCode & B00001111;
      bitSet(temp, 4);                                      // Convert int to BCD
    }
    else
    {
      temp = UTCMonthCode & B00001111;
    }

    I2C_TX(RTCDS1337,RTC_MONTH,temp);
    break;

  case 5:    // Date

    //   I2C_RX(RTCDS1337,RTC_DATE);
    //   DateOnes = i2cData & B00001111;                   
    //   DateTens = (i2cData & B00110000) >> 4;    
    UTCDateOnes = UTCDateOnes + 1;
    if((UTCDateTens == 3) && (UTCDateOnes > 1))
    {
      UTCDateOnes = 1;
      UTCDateTens =0;
    }
    else
    {
      if(UTCDateOnes>9)
      {
        UTCDateOnes = 0;
        UTCDateTens = UTCDateTens +1;
      }
    }
    temp = (UTCDateOnes & B00001111) | ((UTCDateTens << 4) & B00110000);
    I2C_TX(RTCDS1337,RTC_DATE,temp);
    break;  

  case 6:     // year
    UTCYearsOnes = UTCYearsOnes + 1;
    if (UTCYearsOnes > 9) {
      UTCYearsOnes = 0;
      UTCYearsTens++;
    }
    if (UTCYearsTens > 9) {
      UTCYearsTens = 0;
      UTCYearsOnes = 0;
    }
    temp = (UTCYearsOnes & B00001111) | ((UTCYearsTens << 4) & B11110000);
    I2C_TX(RTCDS1337,RTC_YEAR,temp);
    break;
  }
}

//*******************************************************************************************************************
// 12:00								                                 Start Time
//*******************************************************************************************************************

void SetStartTime()
{
  // For now, don't do this, as I don't want to reset the RTC all the time when debugging
  /*
  uint8_t temp =0;

  UTCHourTens = 1;
  UTCHourOnes = 2;
  temp = (UTCHourTens << 4) + UTCHourOnes;
  // We only support 24 hour UTC time regardless of what is displayed

  I2C_TX(RTCDS1337,RTC_HOUR,temp);  

  MinTens = 0;
  MinOnes = 0;
  temp = (MinTens << 4) + MinOnes;
  I2C_TX(RTCDS1337,RTC_MIN,temp);
  */
}

//*******************************************************************************************************************
// 								                                 SET Alarm
//*******************************************************************************************************************

void SetAlarmTime()                              // Just for testing set to 12:01 PM
{
  uint8_t temp =0;

  uint8_t HourTens = 1;
  uint8_t HourOnes = 2;
  temp = (HourTens << 4) + HourOnes;
  // Only support 24 hour time

  I2C_TX(RTCDS1337,RTC_ALARM1HOUR,temp);   

  uint8_t MinTens = 0;
  uint8_t MinOnes = 1;
  temp = (MinTens << 4) + MinOnes;
  I2C_TX(RTCDS1337,RTC_ALARM1MIN,temp);
}

//*******************************************************************************************************************
// 								                                 Check Alarm
//*******************************************************************************************************************

void CheckAlarm()
{
  uint8_t temp =0;
  I2C_RX(RTCDS1337,RTCSTATUS);
  ALARM1FLAG = bitRead(i2cData, 0);

  if(ALARM1FLAG)
  {
    temp =i2cData;
    bitClear(temp, 0);
    I2C_TX(RTCDS1337,RTCSTATUS,temp);   
  }

}

//*******************************************************************************************************************
// 								                                 Enable Alarm
//*******************************************************************************************************************

void EnableAlarm1(boolean onoff)                                            // Trigger on Hours & Minutes Match
{
  uint8_t temp =0;

  // Adjust for Hours - Minutes Trigger -S
  I2C_RX(RTCDS1337,RTC_ALARM1SEC);
  temp =i2cData;
  bitClear(temp, 7);
  I2C_TX(RTCDS1337,RTC_ALARM1SEC,temp);

  I2C_RX(RTCDS1337,RTC_ALARM1MIN);
  temp =i2cData;
  bitClear(temp, 7);
  I2C_TX(RTCDS1337,RTC_ALARM1MIN,temp);

  I2C_RX(RTCDS1337,RTC_ALARM1HOUR);
  temp =i2cData;
  bitClear(temp, 7);
  I2C_TX(RTCDS1337,RTC_ALARM1HOUR,temp); 

  I2C_RX(RTCDS1337,RTC_ALARM1DATE);
  temp =i2cData;
  bitSet(temp, 7);
  I2C_TX(RTCDS1337,RTC_ALARM1DATE,temp);  
  // Adjust for Hours - Minutes Trigger -E

  I2C_RX(RTCDS1337,RTCCONT);                                  // Enable Alarm Pin on RTC
  temp =i2cData;

  if(onoff)
  {
    bitSet(temp, 0);    
  }
  else
  {
    bitClear(temp, 0);
  }
  I2C_TX(RTCDS1337,RTCCONT,temp);

  I2C_RX(RTCDS1337,RTCSTATUS);                                // Clear Alarm RTC internal Alarm Flag
  temp =i2cData;
  bitClear(temp, 0);
  I2C_TX(RTCDS1337,RTCSTATUS,temp);
}
//*******************************************************************************************************************
// 								                                 SET ALARM TIME
//*******************************************************************************************************************

void setAlarm(uint8_t setselect)                                // both min digits or both hour digits (advance one at a time)
{
  uint8_t temp =0;
  switch(setselect)
  {

  case 1:
    AMinOnes = AMinOnes +1;
    if(AMinOnes >9)
    {
      AMinOnes = 0;

      AMinTens = AMinTens +1;
      if(AMinTens >5)
      {
        AMinTens = 0;
      }
    }

    temp = (AMinTens << 4) + AMinOnes;
    I2C_TX(RTCDS1337,RTC_ALARM1MIN,temp);
    break;


  case 2:
    AHourOnes = AHourOnes +1;
    
// -----------*
    if(A_TH_Not24_flag)
//                                                                    12 hours mode increment    
    {

    if(AHourOnes >9 )
    {
      AHourOnes = 0;
      AHourTens = 1;
    }
    
   if((AHourOnes ==2) &&  (AHourTens == 1))
    {
      A_PM_NotAM_flag = !A_PM_NotAM_flag;
    }
    
    if((AHourOnes >2) &&  (AHourTens == 1))
    {
//      PM_NotAM_flag = !PM_NotAM_flag;
      AHourTens = 0;
      AHourOnes = 1;      
    }
    
    }
    else
//                                                                    24 hours mode increment - S    
    {

    if((AHourOnes >9) && (AHourTens < 2))
    {
      AHourOnes = 0;
      AHourTens = AHourTens +1;
    }
    
     if((AHourTens ==2) && (AHourOnes == 4))
    {
      AHourOnes = 0;
      AHourTens = 0;    
    }
    }
//                                                                    24 hours mode increment - E
// -----------*
    
/*    
    if(AHourOnes >9)
    {
      AHourOnes = 0;
      AHourTens = AHourTens +1;
      if((AHourTens >1) && (A_TH_Not24_flag))
      {
        AHourTens = 0;
      }
      else
      {
        if(AHourTens >2)
        {
          AHourTens = 0;
        }
      }
    }
*/
    temp = (AHourTens << 4) + AHourOnes;
    if(A_TH_Not24_flag)
    {
      bitWrite(temp, 5, A_PM_NotAM_flag);
    }

    bitWrite(temp, 6, A_TH_Not24_flag);
    I2C_TX(RTCDS1337,RTC_ALARM1HOUR,temp);
    break;
  }

}

//*******************************************************************************************************************
// 								           Toggle Twelve and  Twenty Four hour time
//*******************************************************************************************************************

/* Temporarily disable
void TwelveTwentyFourConvert()                                    
{

  int temphours = 0;
  int temp = 0;

  I2C_RX(RTCDS1337,RTC_HOUR);
  HourOnes = i2cData & B00001111;                   // 

  //  TH_Not24_flag = bitRead(i2cData, 6);                   // False on RTC when 24 mode selected
  //  PM_NotAM_flag = bitRead(i2cData, 5);

  if(TH_Not24_flag)
  {
    HourTens = i2cData & B00010000;                   // 
    HourTens = HourTens >> 4;

  }
  else
  {
    HourTens = i2cData & B00110000;                   // 
    HourTens = HourTens >> 4;
  }

  temphours = HourOnes + (HourTens*10);             // 12 .... 1.2.3...12 or 0 ..1.2.3. ...23

  if(TH_Not24_flag != NewTimeFormate)
  {
    if(NewTimeFormate)                                    // NewTimeFormate is same formate as TH_Not24_flag where H is 12 and LOW is 24
    { 
      // ---------------- 24 -> 12    
      // Convert into 12 hour clock
      if(temphours >= 12)
      {
        PM_NotAM_flag = true;                             // it is in the PM  
        if(temphours> 12)
        {
          temphours = temphours - 12;                     // Convert from 13:00 .... 23:00 to 1:00 ... 11:00 [Go from 23:59 / 13:00 to 12:00 to 1:00] ?
        }
        else
        {
          temphours = temphours;                          // do nothing it is 12:00
        }
      }
      else
      {
        PM_NotAM_flag = false;                            // it is in the AM - No other conversion needed      
      }
      if(temphours == 0)
     {
       temphours = 12;
     } 
    }
    else
      // ---------------- 12 -> 24                        // Convert into 24 hour clock
    { 
       if((PM_NotAM_flag == false) && (temphours == 12))  // AM only check for 00 hours
        {
          temphours = 0;
        }
       if(PM_NotAM_flag == true)
        {                                                  // PM conversion
                 if(temphours != 12)                       // Leave 12 as 12 in 24h time
                 {
                   temphours = temphours + 12;
                 }
        }

    }
    
    // Common finish conversion section
    TH_Not24_flag = NewTimeFormate;
    HourTens = temphours / 10;
    HourOnes = temphours % 10;
    
    // ---
    temp = (HourTens << 4) + HourOnes;
    if(TH_Not24_flag)
    {
      bitWrite(temp, 5, PM_NotAM_flag);
    }

    bitWrite(temp, 6, TH_Not24_flag);
    I2C_TX(RTCDS1337,RTC_HOUR,temp);
    
    // ---
    
  }


}
*/

void UTCtoLocal()
{
  int8_t Hour = UTCHourTens * 10 + UTCHourOnes;
  int8_t DayRoll = 0;
  Hour += UTCOffset;
  if ( Hour < 0 ) {
    Hour += 24;
    DayRoll = -1;
  }
  else if ( Hour > 23 ) {
    Hour -= 24; 
    DayRoll = 1;
  }
  LocalHourOnes = Hour % 10;
  Hour -= LocalHourOnes;
  LocalHourTens = Hour / 10;

  LocalDays = UTCDays;
  LocalDateOnes = UTCDateOnes;
  LocalDateTens = UTCDateTens;
  LocalMonthCode = UTCMonthCode;
  LocalYearsOnes = UTCYearsOnes;
  LocalYearsTens = UTCYearsTens;
  if ( DayRoll == 0 )
    return;

  int8_t Date = UTCDateTens * 10 + UTCDateOnes;
  int8_t Year = UTCYearsTens * 10 + UTCYearsOnes;
  
  if ( DayRoll == 1 ) {
    // Roll local date forward
    LocalDays++;
    if ( LocalDays == 8 )
      LocalDays = 1;
    Date++;
    if ( Date == 32 ) {
      Date = 1;
    } else if ( Date == 31 && ( LocalMonthCode == 4 || LocalMonthCode == 6 || LocalMonthCode == 9 || LocalMonthCode == 11 ) ) {
      Date = 1;
    } else if ( LocalMonthCode == 2 ) {
      if ( Date == 30 )
        Date = 1;
      else if ( Date == 29 && ! ( Year > 0 && Year % 4 == 0 ) )// Note: 2000 is not a leap year!
        Date = 1;
    }
    if ( Date == 1 ) {
      LocalMonthCode++;
      if ( LocalMonthCode == 13 ) {
        // Advance year
        LocalMonthCode = 1;
        Year++;
      }      
    }

  } else if ( DayRoll == -1 ) {  
    // Roll local date backward
    LocalDays--;
    if ( LocalDays == 0 ) {
      LocalDays = 7;
    }
    Date--;
    if ( Date == 0 ) {
      // Roll month backward
      LocalMonthCode--;
      if ( LocalMonthCode == 0 ) {
        // Roll year backward
        LocalMonthCode = 12;
        Year--;
        if ( Year == -1 )
          Year = 99;
      }
      // Set day to last day of new month
      if ( LocalMonthCode == 4 || LocalMonthCode == 6 || LocalMonthCode == 9 || LocalMonthCode == 11 ) {
        Date = 30;
      } else if ( LocalMonthCode == 2 ) {
        if ( Year > 0 && Year % 4 == 0 ) // Note: 2000 is not a leap year!
          Date = 29;
        else
          Date = 28;
      } else {
        Date = 31; 
      }
    }
  }

  LocalDateOnes = Date % 10;
  LocalDateTens = (Date - LocalDateOnes) / 10;
  LocalYearsOnes = Year % 10;
  LocalYearsTens = ((Year - LocalYearsOnes) % 100) / 10;   
}  

