#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <EEPROM.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Address 0x27, 16 columns, 2 rows

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; // connect to R1-R4
byte colPins[COLS] = {5, 4, 3, 2}; // connect to C1-C4
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Servo and security
Servo myServo;
int servoPin = 10;
int buzzer = 11;
int greenLED = 12;
int redLED = 13;

String inputPassword = "";
const int maxAttempts = 3;
int attemptCount = 0;

String masterPassword = "1234";  // Master Password
String defaultUserPassword = "4321"; // Default Normal User Password

void setup() {
  Serial.begin(9600);
  myServo.attach(servoPin);
  pinMode(buzzer, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.print(" Door Lock Sys ");
  delay(2000);
  lcd.clear();
  lcd.print("Enter Password:");
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    if (key == '#') {
      processPassword();
    } else if (key == '*') {
      inputPassword = "";
      lcd.clear();
      lcd.print("Cleared! Re-enter");
    } else {
      inputPassword += key;
      lcd.setCursor(0, 1);
      for (int i = 0; i < inputPassword.length(); i++) {
        lcd.print("*");
      }
    }
  }
}

void processPassword() {
  lcd.clear();
  if (inputPassword == masterPassword) {
    lcd.print("Master Access");
    grantAccess();
    logAccess(true);
  } else if (inputPassword == getUserPassword()) {
    lcd.print("User Access");
    grantAccess();
    logAccess(false);
  } else {
    lcd.print("Access Denied");
    digitalWrite(redLED, HIGH);
    tone(buzzer, 1000, 500);
    delay(1000);
    digitalWrite(redLED, LOW);
    attemptCount++;
    if (attemptCount >= maxAttempts) {
      lcd.clear();
      lcd.print("LOCKED OUT!");
      while (1); // Lock system
    }
  }
  inputPassword = "";
  delay(1500);
  lcd.clear();
  lcd.print("Enter Password:");
}

void grantAccess() {
  digitalWrite(greenLED, HIGH);
  myServo.write(90);  // Unlock
  delay(3000);        // Wait 3 sec
  myServo.write(0);   // Lock again
  digitalWrite(greenLED, LOW);
}

String getUserPassword() {
  String pass = "";
  for (int i = 0; i < 4; i++) {
    pass += char(EEPROM.read(i));
  }
  if (pass == "" || pass == "FFFF") {
    return defaultUserPassword;
  }
  return pass;
}

void logAccess(bool isMaster) {
  int addr = 10; // Example EEPROM address
  EEPROM.write(addr, isMaster ? 'M' : 'U');
  addr++;
  EEPROM.write(addr, attemptCount); // You can expand this to add timestamps etc.
}
