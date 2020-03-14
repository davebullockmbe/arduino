
/*****************************************************************************
*          R E A D   B U T T O N S   A N D   D I S P L A Y    M E N U   
******************************************************************************
 * Read the buttons and display the correct menu if button is clicked.
 * Each item in the switch statement is a menu. For each menu the buttons actions are handled.
*/
void readButtons() 
{
// Get button states  
  bool menu_pressed = (button_MENU.update() && button_MENU.rose());  
  bool select_pressed = (button_SELECT.update() && button_SELECT.rose());
  byte chars = 0;

  if (menu_pressed)
  {
    if (menumode == 0)
    {
      menumode = 1;
    }
    else
    {
      menumode = 0;
      displayMeasurements();
    }
  }

  if (menumode)
  {
  lcd.home();
  switch(current_menu)
    {
    case MENU_ATTENUATION_0dB:      // 0
      if (select_pressed){
        attenuation = 0;
        displayMeasurements();
        menumode = 0;
      }
      else {
        attn_menu_text(0);
        select_menu_text();
      }  
    break;
//*****************************************************
    case MENU_ATTENUATION_10dB:     // 1
      if (select_pressed){
        attenuation = -10;
        displayMeasurements();
        menumode = 0;
      }
      else {
        attn_menu_text(-10);
        select_menu_text();
      }  
    break;
//*****************************************************
    case MENU_ATTENUATION_20dB:     // 2
      if (select_pressed){
        attenuation = -20;
        displayMeasurements();
        menumode = 0;
      }
      else {
        attn_menu_text(-20);
        select_menu_text();
      }  
    break;
//*****************************************************
    case MENU_ATTENUATION_30dB:     // 3
      if (select_pressed){
        attenuation = -30;
        displayMeasurements();
        menumode = 0;
      }
      else {
        attn_menu_text(-30);
        select_menu_text();
      }  
    break;
//*****************************************************
    case MENU_ATTENUATION_40dB:     // 4
      if (select_pressed){
        attenuation = -40;
        displayMeasurements();
        menumode = 0;
      }
      else {
        attn_menu_text(-40);
        select_menu_text();
      }  
    break;
//*****************************************************
    case MENU_ATTENUATION_50dB:     // 5
      if (select_pressed){
        attenuation = -50;
        displayMeasurements();
        menumode = 0;
      }
      else {
        attn_menu_text(-50);
        select_menu_text();
      }  
    break;
//*****************************************************
    case MENU_RF_METER:             // 6
      if (select_pressed){
        menumode = 0;
      }
      else {
        lcd.home();
        chars = lcd.print(F("Menu: ")); chars += lcd.print(current_menu);
        chars += lcd.print(F(" RF PowerMeter   ")); 
        select_menu_text();
      }
    break;
//*****************************************************
    case MENU_CALIBRATION:          // 7
      if (select_pressed){
        lcd.home();
        float actualPowerdBm = roundDecimals(avg_dbm - attenuation, 1);
        if ((measure == 0 && (actualPowerdBm < -20 || actualPowerdBm > 0)) || (measure == 1 && (actualPowerdBm < -50 || actualPowerdBm > -30)))
        {
          lcd.print(F("Not between 0 and -20dB "));
          lcd.setCursor(0,1);
          lcd.print(F("or between -30 and -50dB"));
          delay(2000);
          menumode = 0;
          measure = 0;
          displayMeasurements();
        }  
        else
        {
          slopeCalculation();
          delay(1000);
          if (measure == 1)
          {  
            saveCalibrationData();
            mergeCalibrationData();
            get_slope_intercept();
            displayCalibrationSavedNotification();
            menumode = 0;
            measure = 0;
            displayMeasurements();
          }
          else if (measure == 0) measure = 1;
        }
      }
      else {
        lcd.home();
        chars = lcd.print(F("Menu: ")); chars += lcd.print(current_menu);
        if (measure == 0) {
          chars += lcd.print(F(" Cal -10dBm ")); chars += lcd.print(BANDS[current_band]);
        } 
        else if (measure == 1) {
          chars += lcd.print(F(" Cal -40dBm ")); chars += lcd.print(BANDS[current_band]);
        }
        select_menu_text();
      }
    break;
//*****************************************************
    case MENU_CALIBRATION_READ:     // 8
      if (select_pressed){
        lcd.home();
        for (int i=0; i<4; i++)
        {
          lcd.print(mergedCalibrationData[i].dBm_at_0V);
          lcd.print(F(" "));
        }
        lcd.setCursor(0,1);
        for (int i=4; i<8; i++)
        {
          lcd.print(mergedCalibrationData[i].dBm_at_0V);
          lcd.print(F(" "));
        }
        select_pressed = false;
        
        while(!(button_SELECT.update() && button_SELECT.rose())) {   // Wait on select button
        delay(50);
        }
        
        menumode = 0;
        displayMeasurements();
      }
      else {
        lcd.home();
        chars = lcd.print(F("Menu: ")); chars += lcd.print(current_menu);
        chars += lcd.print(F(" Read Cal.Data   ")); 
        select_menu_text();
      }
    break;
//*****************************************************
    case MENU_CALIBRATION_RESET:    // 9
      if (select_pressed){
        lcd.home();
        deleteCalibrationPoints();
        displayCalibrateDeletedMenuNotification();
        menumode = 0;
        displayMeasurements();
      }
      else {
        lcd.home();
        chars = lcd.print(F("Menu: ")); chars += lcd.print(current_menu);
        chars += lcd.print(F("  Zero All Cal   ")); 
        select_menu_text();
      }
    break;    
    }  // end of switch

  bool menu_pressed = false;  
  bool select_pressed = false;                
  } 
}

void attn_menu_text(int attn)
{
  byte chars = 0;
  lcd.home();
  chars = lcd.print(F("Menu: ")); chars += lcd.print(current_menu);
  chars += lcd.print(F(" Attn = ")); chars += lcd.print(attn);
  chars += lcd.print(FS(str_dB));
  filloutLine(chars); 
}

void select_menu_text()
{
  lcd.setCursor(0, 1);        // goto next line
  lcd.print(F("   Select = Activate    "));
}

/*
 * Wait for x ms but read buttons meanwhile
*/
void readButtonsAndWait(int waitMs)
{
  unsigned long t = millis();
  while (millis() - t < waitMs)
    readButtons();
}

int blinkCnt = 0;
/*
 * Display warning message
*/
void displayOverloadWarning() 
{
  blinkCnt++;
  if (blinkCnt % 2 == 0) {
    lcd.home();  
    lcd.print(F("OVERLOAD WARNING"));         
    lcd.setCursor(0, 1); // goto next line
    lcd.print(F("DISCONNECT NOW! "));        
    lcd.display();
  }
  else
    lcd.noDisplay();
}

/*****************************************************************************
*                      D I S P L A Y    M E A S U R E M E N T S   
******************************************************************************
*  
* Display the settings and primary measurements
* Frequency, attenuation and power in dBm and mW
*/
void displayMeasurements() 
{
  byte chars = 0;
  lcd.home();
  
// Show power measurement in dBm.
  float actualPowerdBm = roundDecimals(avg_dbm - attenuation, 1);
  Serial.print(avg_dbm - attenuation);Serial.println(attenuation);
  printFormattedNumber(actualPowerdBm, 1, 1, true, true);     
  lcd.print(FS(str_dBm));  
  
  // Display Band and attenuation in first line alternatly
  lcd.setCursor(10, 0);
  if (cnt <= 2)
  {    
    lcd.print(BANDS[current_band]);    
  }
  else if (cnt <= 4)
  {
    lcd.print(F(" ATTN"));   
  }
  else if (cnt <= 6)
  {
    if (attenuation == 0) chars += lcd.print(F("  ")); 
    printFormattedNumber(attenuation, 1, 0, false, true);
    lcd.print(FS(str_dB));      
  }
  else if (cnt <= 8)
  {
    lcd.print(F(" Err."));   
  }
  else if (cnt <= 10)
  {
    lcd.print(F("  ")); 
    printFormattedNumber(currentError(), 1, 1, false, false);
  }
  cnt++;
  if (cnt > 10) cnt = 0; // reset loop

  lcd.print(F("        ")); 
  
  float actualVoltage = convertdBmToVolt(actualPowerdBm);
  char pos = printVoltage(actualVoltage);
  lcd.setCursor(24-pos, 0);                   // to get fixed position
  printVoltage(actualVoltage);    

  
// Line 2
  lcd.setCursor(0, 1); // goto next line     
  if (power_notconnected)     
    lcd.print(F("   INPUT TOO LOW / NC   ")); // Show not connected
  else
  {           
//Show Bar graph
    int user2 = 0;                            // carracter pointer
    char user1 = 76 - (avg_voltage*45);       // now max rf level is 2V 
    if (user1 > 76) user1 = 76;
    
    while (user1>=5)                          // how meny full black carracters to draw
    {
      lcd.write(byte(4));                     // vol blokje
      user1=user1-5;
      user2++;
    }
    if (user1==5) lcd.write(byte(4));         // vol blokje
    else
    if (user1==4) lcd.write(byte(3));         // 4 streepjes
    else
    if (user1==3) lcd.write(byte(2));         // 3 streepjes
    else
    if (user1==2) lcd.write(byte(1));         // 2 streepjes
    else
    if (user1==1) lcd.write(byte(0));         // 1 streepje

    while (user2<=20)                         // erase to end of line
    {
      lcd.print(" ");
      user2++;
    } 
     
// Show power measurement in watt.

    float actualPowerMw = convertDbmToMilliWatt(actualPowerdBm);
    char pos = printPowerWatts(actualPowerMw);    
    lcd.setCursor(24-pos, 1);                 // to get fixed position
    lcd.print(printPowerWatts(actualPowerMw));
  }    
}


/*****************************************************************************
*            D I S P L A Y    C A L I B R A T I O N S    N O T E S   
******************************************************************************
*
* Display calibration point deleted notification
*/
void displayCalibrateDeletedMenuNotification() {
  lcd.home();  
  lcd.print(F("   Calibration points   "));    
  lcd.setCursor(0, 1);  
  lcd.print(F("        deleted!        "));    
  delay(2000);
}

/*
 * Display calibration saved notification
*/
void displayCalibrationSavedNotification()
{
  lcd.home();
  lcd.print(F("Slope and Intrcpt values"));
  lcd.setCursor(0, 1);
  lcd.print(F(" calculated and saved!  "));
  delay(2000);  
}

/*****************************************************************************
*                              S P L A S H   S C R E E N   
******************************************************************************
 * Display the splash screen
*/
void displaySplashScreen() 
{
  lcd.home();
  lcd.print(F(" PA0RWE  RF Power Meter "));
  lcd.setCursor(0, 1);
  lcd.print(F("  V.1.0   1MHZ - 10GHz  "));
  delay(2000);
  lcd.home();
  lcd.print(F("  Maximum input power   "));
  lcd.setCursor(0, 1);
  lcd.print(F("     10dBm / 10mW       "));  
  delay(2000);
}
