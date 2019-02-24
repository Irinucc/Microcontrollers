const int outPin = 2;
int ok = 0;
// 1299bps=833us/bit   lng=70%=583     sht=30%=250     cor=correction for pinset/pinclear
#define        ON    0
#define        OFF   1

#define FRAMEREPEAT 15
#define cor  4
#define lng 583 - cor
#define sht 250 - cor

#define rfBIT_H {digitalWrite(outPin,HIGH); delayMicroseconds(sht); digitalWrite(outPin,LOW); delayMicroseconds(lng-5);}
#define rfBIT_L {digitalWrite(outPin, HIGH); delayMicroseconds(lng); digitalWrite(outPin, LOW ); delayMicroseconds(sht-5);}

unsigned long wait = 60000;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

void Send_RF_frame(byte channel , byte ON_OFF) {  
  byte rfFrame = B00000001;

  if ( ON_OFF == OFF ) {                                // Code On Off bit in frame
    rfFrame = B00000011;
  }
  
  switch (channel) {                                   // Code channel bits in frame 
    case 1: { rfFrame |= B00000001;   break; }         // button 1
    case 2: { rfFrame |= B00010001;   break; }         // button 2
    case 3: { rfFrame |= B00001001;   break; }         // button 3
    case 4: { rfFrame |= B00011001;   break; }         // button 4
    case 5: { rfFrame |= B00000101;   break; }         // button ALL
    default:{ rfFrame  = B00000001;   break; }
  }
                                                       //-----------------------------
  for ( byte j=0;j<FRAMEREPEAT;j++) {                  // Frame repeats
    for ( byte i=0;i<20;i++) {rfBIT_H}                 // 20 preamble bits
    if (rfFrame & B00010000) {rfBIT_H} else {rfBIT_L}  // ChannelBit2
    if (rfFrame & B00001000) {rfBIT_H} else {rfBIT_L}  // ChannelBit1
    if (rfFrame & B00000100) {rfBIT_H} else {rfBIT_L}  // ChannelBit0
    if (rfFrame & B00000010) {rfBIT_H} else {rfBIT_L}  // On Off Bit
    if (rfFrame & B00000001) {rfBIT_H} else {rfBIT_H}  // stopbit always 1
                          
    delayMicroseconds(4600);                           // interFrame delay
  }

}
String menuItems[] = {"Priza 1", "Priza 2", "Priza 3", "Toate prizele"};

// Navigation button variables
int readKey;
int savedDistance = 0;
 
// Menu control variables
int menuPage = 0;
int maxMenuPages = 2; //round(((sizeof(menuItems) / sizeof(String)) / 2) + .5);
int cursorPosition = 0;
 
// Creates 3 custom characters for the menu display
byte downArrow[8] = {
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b10101, // * * *
  0b01110, //  ***
  0b00100  //   *
};
 
byte upArrow[8] = {
  0b00100, //   *
  0b01110, //  ***
  0b10101, // * * *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100  //   *
};
 
byte menuCursor[8] = {
  B01000, //  *
  B00100, //   *
  B00010, //    *
  B00001, //     *
  B00010, //    *
  B00100, //   *
  B01000, //  *
  B00000  //
};
 
#include <Wire.h>
#include <LiquidCrystal.h>
 
// Setting the LCD shields pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
 
void setup() { 
  // Initializes serial communication
  Serial.begin(9600);

  pinMode(outPin, OUTPUT);
  digitalWrite(outPin, LOW);
  delay(2000);
  
  // Initializes and clears the LCD screen
  lcd.begin(16, 2);
  lcd.clear();
 
  // Creates the byte for the 3 custom characters
  lcd.createChar(0, menuCursor);
  lcd.createChar(1, upArrow);
  lcd.createChar(2, downArrow);
}
 
void loop() {
  mainMenuDraw();
  drawCursor();
  operateMainMenu();
}

void mainMenuDraw() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(menuItems[menuPage]);
  lcd.setCursor(1, 1);
  lcd.print(menuItems[menuPage + 1]);
  if (menuPage == 0) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  } else if (menuPage > 0 and menuPage < maxMenuPages) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  } else if (menuPage == maxMenuPages) {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
}

void drawCursor() {
  for (int x = 0; x < 2; x++) {     // Erases current cursor
    lcd.setCursor(0, x);
    lcd.print(" ");
  }
  if (menuPage % 2 == 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is even and the cursor position is even that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is even and the cursor position is odd that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
  }
  if (menuPage % 2 != 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is odd and the cursor position is even that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is odd and the cursor position is odd that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
  }
}

void operateMainMenu() {
  int activeButton = 0;
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) { // If the value is over 790, it's a glitch, so it has to be ignored
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 0: // When button returns as 0 there is no action taken
        break;
      case 1:   // This case will execute if the "forward" button is pressed
        button = 0;
        switch (cursorPosition) { // The case that is selected here is dependent on which menu page you are on and where the cursor is.
          case 0:
            menuItem1();
            break;
          case 1:
            menuItem2();
            break;
          case 2:
            menuItem3();
            break;
          case 3:
            menuItem4();
            break;  
        }
        activeButton = 1;
        mainMenuDraw();
        drawCursor();
        break;
      case 2:   // This case will execute if the "up" button is pressed
        button = 0;
        if (menuPage == 0) {
          cursorPosition = cursorPosition - 1;
          cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        }
        if (menuPage % 2 == 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }
 
        if (menuPage % 2 != 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }
 
        cursorPosition = cursorPosition - 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
 
        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
      case 3:   // This case will execute if the "down" button is pressed
        button = 0;
        if (menuPage % 2 == 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }
 
        if (menuPage % 2 != 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }
 
        cursorPosition = cursorPosition + 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
      case 4:   // This case will execute if the "backward" button is pressed
        button=0;
        menuPage=0;
        mainMenuDraw();
        cursorPosition = 0;
        drawCursor();
        activeButton = 1;
        break;
    }
  }
}
 
int evaluateButton(int x) {
  int result = 0;
  if (x < 50) {
    result = 1; // right
  } else if (x < 195) {
    result = 2; // up
  } else if (x < 380) {
    result = 3; // down
  } else if (x < 790) {
    result = 4; // left
  }
  return result;
}

void menuItem1() {
  int activeButton = 0;
 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(0));
  lcd.setCursor(1, 0);
  lcd.print("Press");
  lcd.setCursor(7, 0);
  lcd.write(byte(1));
  lcd.setCursor(9, 0);
  lcd.print("for ON");
  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.setCursor(1, 1);
  lcd.print("Press");
  lcd.setCursor(7, 1);
  lcd.write(byte(2));
  lcd.setCursor(9, 1);
  lcd.print("for OFF");
 
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 1:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Timer set: 1min");
        Send_RF_frame(1,ON);
        ok=0;
        while (ok==0)
          {    
            currentMillis = millis();
            if ((unsigned long)(currentMillis-previousMillis)>=wait)
              { lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("PRIZA 1 OFF");
                Send_RF_frame(1,OFF);
                previousMillis=millis();
                ok=1;
              }
          }
        ok=0;
        break;
      case 2:
        Send_RF_frame(1,ON);
        break;
      case 3:
        Send_RF_frame(1,OFF);
        break;
      case 4: 
        button = 0;
        activeButton = 1;
        break;
    }
  }
}
 
void menuItem2() { 
  int activeButton = 0;
 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(0));
  lcd.setCursor(1, 0);
  lcd.print("Press");
  lcd.setCursor(7, 0);
  lcd.write(byte(1));
  lcd.setCursor(9, 0);
  lcd.print("for ON");
  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.setCursor(1, 1);
  lcd.print("Press");
  lcd.setCursor(7, 1);
  lcd.write(byte(2));
  lcd.setCursor(9, 1);
  lcd.print("for OFF");
 
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 1:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Timer set: 1min");
        Send_RF_frame(2,ON);
        ok=0;
        while (ok==0)
          { currentMillis = millis();
            if ((unsigned long)(currentMillis-previousMillis)>=wait)
              { lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("PRIZA 2 OFF");
                Send_RF_frame(2,OFF);
                previousMillis=millis();
                ok=1;
              }
          }
        ok=0;
        break;
      case 2:
        Send_RF_frame(2,ON);
        break;
      case 3:
        Send_RF_frame(2,OFF);
        break;
      case 4:  
        button = 0;
        activeButton = 1;
        break;
    }
  }
}
 
void menuItem3() { 
  int activeButton = 0;
 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(0));
  lcd.setCursor(1, 0);
  lcd.print("Press");
  lcd.setCursor(7, 0);
  lcd.write(byte(1));
  lcd.setCursor(9, 0);
  lcd.print("for ON");
  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.setCursor(1, 1);
  lcd.print("Press");
  lcd.setCursor(7, 1);
  lcd.write(byte(2));
  lcd.setCursor(9, 1);
  lcd.print("for OFF");
 
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 1:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Timer set: 1min");
        Send_RF_frame(3,ON);
        ok=0;
        while (ok==0)
          { currentMillis = millis();
            if ((unsigned long)(currentMillis-previousMillis)>=wait)
              { lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("PRIZA 3 OFF");
                Send_RF_frame(3,OFF);
                previousMillis=millis();
                ok=1;
              }
          }
        ok=0;
        break;
      case 2:
        Send_RF_frame(3,ON);
        break;
      case 3:
        Send_RF_frame(3,OFF);
        break;
      case 4:  // 
        button = 0;
        activeButton = 1;
        break;
    }
  }
}
 
void menuItem4() { 
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(0));
  lcd.setCursor(1, 0);
  lcd.print("Press");
  lcd.setCursor(7, 0);
  lcd.write(byte(1));
  lcd.setCursor(9, 0);
  lcd.print("for ON");
  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.setCursor(1, 1);
  lcd.print("Press");
  lcd.setCursor(7, 1);
  lcd.write(byte(2));
  lcd.setCursor(9, 1);
  lcd.print("for OFF");

  
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 1:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Timer set: 1min");
        Send_RF_frame(5,ON);
        ok=0;
        while (ok==0)
          { currentMillis = millis();
            if ((unsigned long)(currentMillis-previousMillis)>=wait)
              { lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("TOATE PRIZELE");
                lcd.setCursor(0, 1);
                lcd.print("OFF");
                Send_RF_frame(5,OFF);
                previousMillis=millis();
                ok=1;
              }
          }
        ok=0;
        break;
      case 2:
        Send_RF_frame(5,ON);
        break;
      case 3:
        Send_RF_frame(5,OFF);
        break;
      case 4:  
        button = 0;
        activeButton = 1;
        break;
    }
  }
}
