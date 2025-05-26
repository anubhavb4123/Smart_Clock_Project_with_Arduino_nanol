#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <DHT.h>
#include <EEPROM.h>
// Pin configuration
#define DHTPIN 2            // Pin connected to DHT sensor
#define DHTTYPE DHT11       // Change to DHT22 if using DHT22
#define LDR_PIN A0          // LDR connected to analog pin A0
#define BATTERY_PIN A1      // Battery voltage sensing pin
#define BACKLIGHT_PIN 9     // LCD backlight connected to PWM pin 9
#define VIBRATION_PIN 12    // Vibration motor connected to pin 12
#define HOU_BUTTON_LED 8    // LED for low battery warning
#define NIGHT_LIGHT 11      // Night light LED
#define ALARM_RESET_PIN 3   // Reset button for stopping alarm
#define ALARM_MIN_PIN 4     // Pin for setting minimum alarm time
#define ALARM_HOU_PIN 5     // Pin for setting maximum alarm time
#define BUZZER 6            // Buzzer connected to pin 6
#define BLINKING_LED 13     // Use built-in LED or another available pin
#define RESET_BUTTON_LED 10 // Hour button LED
#define MINUTE_BUTTON_LED 7 // Minute button LED
// Constants
#define MAX_BATTERY_VOLTAGE 4.2  // Max voltage (fully charged battery)
#define MIN_BATTERY_VOLTAGE 3.0  // Min voltage (fully discharged)
#define LOW_BATTERY_THRESHOLD 26// Battery % threshold for LED warning
#define TIME_DISPLAY_DELAY 5000  // Delay for time/date screen
#define TEMP_DISPLAY_DELAY 3000  // Delay for temp/humidity screen
#define BATT_DISPLAY_DELAY 2000  // Delay for battery volt and %
const int alarmHourAddr = 0;  // EEPROM address for alarm hour
const int alarmMinAddr = 1;   // EEPROM address for alarm minute
const int reminderhourAddr = 2;   // EEPROM address for reminder hour
const int reminderminAddr = 3;    // EEPROM address for reminder minute
const int reminderdayAddr = 4;    // EEPROM address for reminder day
const int remindermonthAddr = 5;  // EEPROM address for reminder month
const int setNightLightvalueAddr = 6;  // EEPROM address for night light value
RTC_DS1307 rtc;
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust I2C address (0x27 is common)
int lastVibratedHour = -1;  // Store last hour when the vibration was triggered
int lastAlarmMinute = -1; // Stores the last hour when the alarm was triggered
int lastVibratedHalfHour = -1; // Stores the last half hour when the vibration was triggered
int alarmHour = 7;
int alarmMinute = 30;
int reminderhour = 8;
int reminderminute = 30;
int lastReminder  = -1 ;// Stores the last reminder time
int reminderday = 1; // Reminder day of the month
int remindermonth = 1;
int timerMinutes = 0;
int timerSeconds = 0;
int setNightLightvalue = 0;
int timerMilliseconds = 0;
bool timerActive = false;
bool alarmActive = false;
bool nightLightActive = true;
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
// Function prototypes
void displayStartupAnimation();
void displayInitializingMessage();
void displayBattery(float batteryVoltage, int batteryPercentage);
void adjustBacklight();
void controlNightLight(int currentHour);
void controlVibration(int currentHour);
void controlAlarm(int currentHour);
void controlBuzzer(int currentHour);
void controlBlinkingLed(int currentHour);
void controlHourButtonLed(int currentHour);
void controlMinuteButtonLed(int currentHour);
void displayTimeDate();
void displayTempHumidity();
void displayBatteryVoltage();
void displayBatteryPercentage();
void activateAlarm();
void setTimer();
void startTimer();
void activateTimer();
void editreminder();
void setup() {
  Serial.begin(9600);
  analogWrite(BACKLIGHT_PIN, 210);
  lcd.init();
  lcd.backlight();
  pinMode(VIBRATION_PIN, OUTPUT);
  digitalWrite(VIBRATION_PIN, HIGH); // Ensure motor is OFF at start
  pinMode(HOU_BUTTON_LED, OUTPUT);
  digitalWrite(HOU_BUTTON_LED, HIGH); // LED OFF initially
  pinMode(NIGHT_LIGHT, OUTPUT);
  digitalWrite(NIGHT_LIGHT, HIGH); // Night light OFF initially
  pinMode(BUZZER, OUTPUT); // Buzzer OFF initially
  digitalWrite(BUZZER, HIGH);
  pinMode(BLINKING_LED, OUTPUT); // Blinking LED OFF initially
  digitalWrite(BLINKING_LED, HIGH);
  pinMode(RESET_BUTTON_LED,OUTPUT);
  digitalWrite(RESET_BUTTON_LED, HIGH); // Hour button LED OFF
  pinMode(MINUTE_BUTTON_LED,OUTPUT);
  digitalWrite(MINUTE_BUTTON_LED,HIGH);
  pinMode(A2,OUTPUT); // Alarm reset button LED
  digitalWrite(A2, HIGH); // Alarm reset button LED ON initially
  pinMode(A3,OUTPUT); // Alarm reset button LED
  digitalWrite(A3, HIGH); // Alarm reset button LED ON initially
  delay(500); // Wait for 0.5 seconds before starting the display animation
  digitalWrite(VIBRATION_PIN, LOW); // Turn off vibration motor
  digitalWrite(HOU_BUTTON_LED, LOW); // Turn on low battery LED
  digitalWrite(NIGHT_LIGHT, LOW);  // Turn on night light
  digitalWrite(BUZZER, LOW); // Turn on buzzer
  digitalWrite(BLINKING_LED, LOW);  // Turn on blinking LED
  digitalWrite(RESET_BUTTON_LED, LOW); // Turn on hour button LED
  digitalWrite(MINUTE_BUTTON_LED, LOW); // Turn on minute button LED
  digitalWrite(A2, LOW); // Alarm reset button LED OFF initially
  digitalWrite(A3,LOW);
  // Read stored alarm and reminder from EEPROM
  alarmHour = EEPROM.read(alarmHourAddr);
  alarmMinute = EEPROM.read(alarmMinAddr);
  reminderhour = EEPROM.read(reminderhourAddr);
  reminderminute = EEPROM.read(reminderminAddr);
  reminderday = EEPROM.read(reminderdayAddr);
  remindermonth = EEPROM.read(remindermonthAddr);
  setNightLightvalue = EEPROM.read(setNightLightvalueAddr);
  pinMode(ALARM_RESET_PIN, INPUT_PULLUP);
  pinMode(ALARM_MIN_PIN, INPUT_PULLUP);
  pinMode(ALARM_HOU_PIN, INPUT_PULLUP);
  displayStartupAnimation();
  displayInitializingMessage();
  if (!rtc.begin()) {
    lcd.print("RTC not found!");
    while (1);
  }
  if (!rtc.isrunning()) {
    lcd.print("RTC not running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Adjust to compile time
  }
  dht.begin();
  pinMode(BACKLIGHT_PIN, OUTPUT);
}
void loop() {
  adjustBacklight();
  digitalWrite(BLINKING_LED,HIGH);
  DateTime now = rtc.now();
  if (digitalRead(ALARM_RESET_PIN) == LOW && digitalRead(ALARM_HOU_PIN) != LOW && digitalRead(ALARM_MIN_PIN) != LOW) { // Only if Alarm reset button pressed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Edit Alarm time");
    delay(1000);
    editalarm();
  }
  if (digitalRead(ALARM_HOU_PIN) == LOW && digitalRead(ALARM_RESET_PIN) != LOW && digitalRead(ALARM_MIN_PIN) != LOW) { // Only if Alarm hour button pressed
    setTimer();
  }
  if (digitalRead(ALARM_HOU_PIN) == LOW && digitalRead(ALARM_RESET_PIN) == LOW && digitalRead(ALARM_MIN_PIN) != LOW) { // Only if Alarm hour and minute buttons
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Edit Rminder");
    delay(1000);
    editreminder();
  }
  if (digitalRead(ALARM_MIN_PIN) == LOW && digitalRead(ALARM_RESET_PIN) != LOW && digitalRead(ALARM_HOU_PIN) != LOW) { // Only if Alarm minute button pressed
    setNightLight();
  }
  if(digitalRead(ALARM_MIN_PIN) == LOW && digitalRead(ALARM_RESET_PIN) == LOW && digitalRead(ALARM_HOU_PIN) != LOW) { // Only if Alarm minute and hour buttons
    displayLDRvalue();
  }
  // Hourly vibration alert (one-time vibration)
  if (now.minute() == 0 && lastVibratedHour != now.hour()) {
    vibrateOnHour();
    lastVibratedHour = now.hour();
  }
  // Half -hourly vibration alert (repeated vibration)
  if (now.minute() == 30 && lastVibratedHalfHour != now.hour()) {
    digitalWrite(VIBRATION_PIN, HIGH);
    digitalWrite(BUZZER,HIGH);
    delay(50);  // Vibrate for 1.5 seconds
    digitalWrite(BUZZER,LOW);
    delay(450);
    digitalWrite(VIBRATION_PIN, LOW);
    lastVibratedHalfHour = now.hour();
  }
  // Active alarm
  if (now.hour() == alarmHour && now.minute() == alarmMinute && lastAlarmMinute != now.minute()) {
  activateAlarm();
   lastAlarmMinute = now.minute(); // Update last alarm hour
  }
  // Active reminder
  if (now.day() == reminderday && now.month() == remindermonth && lastReminder != now.day() && now.hour() == reminderhour && now.minute() == reminderminute) {
    activatereminder();
    lastReminder = now.day(); // Update last reminder day
  }
  if (digitalRead(ALARM_RESET_PIN) != LOW && digitalRead(ALARM_HOU_PIN) == LOW && digitalRead(ALARM_MIN_PIN) == LOW) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LED flash");
    delay(1000);
    ledFlash();
  }
  // Control Night Light
  controlNightLight(now.hour());
  digitalWrite(BLINKING_LED,LOW);
  displayTime(now);
  delay(TIME_DISPLAY_DELAY);
  if (digitalRead(ALARM_RESET_PIN) == LOW && digitalRead(ALARM_HOU_PIN) != LOW && digitalRead(ALARM_MIN_PIN) != LOW) { // Only if Alarm reset button pressed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Edit Alarm time");
    delay(1000);
    editalarm();
  }
  if (digitalRead(ALARM_HOU_PIN) == LOW && digitalRead(ALARM_RESET_PIN) != LOW && digitalRead(ALARM_MIN_PIN) != LOW) { // Only if Alarm hour button pressed
    setTimer();
  }
  if (digitalRead(ALARM_HOU_PIN) == LOW && digitalRead(ALARM_RESET_PIN) == LOW && digitalRead(ALARM_MIN_PIN) != LOW) { // Only if Alarm hour and minute buttons
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Edit Rminder");
    delay(1000);
    editreminder();
  }
  if (digitalRead(ALARM_MIN_PIN) == LOW && digitalRead(ALARM_RESET_PIN) != LOW && digitalRead(ALARM_HOU_PIN) != LOW) { // Only if Alarm minute button pressed
    setNightLight();
  }
  if(digitalRead(ALARM_MIN_PIN) == LOW && digitalRead(ALARM_RESET_PIN) == LOW && digitalRead(ALARM_HOU_PIN) != LOW) { // Only if Alarm minute and hour buttons
    displayLDRvalue();
  }
  if (digitalRead(ALARM_RESET_PIN) != LOW && digitalRead(ALARM_HOU_PIN) == LOW && digitalRead(ALARM_MIN_PIN) == LOW) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LED flash");
    delay(1000);
    ledFlash();
  }
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  displayTemperatureHumidity(temperature, humidity);
  delay(TEMP_DISPLAY_DELAY);
  if (humidity <= 30){
    lcd.clear();
    digitalWrite(A3,HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Low Humidity");
    lcd.setCursor(0,1);
    lcd.print("Use a humidifier");
    delay(500);
    digitalWrite(A3,LOW);
    delay(500);
  }
  if (digitalRead(ALARM_RESET_PIN) == LOW && digitalRead(ALARM_HOU_PIN) != LOW && digitalRead(ALARM_MIN_PIN) != LOW) { // Only if Alarm reset button pressed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Edit Alarm time");
    delay(1000);
    editalarm();
  }
  digitalWrite(BLINKING_LED,HIGH);
  if (digitalRead(ALARM_HOU_PIN) == LOW && digitalRead(ALARM_RESET_PIN) != LOW && digitalRead(ALARM_MIN_PIN) != LOW) { // Only if Alarm hour button pressed
    setTimer();
  }
  if (digitalRead(ALARM_HOU_PIN) == LOW && digitalRead(ALARM_RESET_PIN) == LOW && digitalRead(ALARM_MIN_PIN) != LOW) { // Only if Alarm hour and minute buttons
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Edit Rminder");
    delay(1000);
    editreminder();
  }
  if (digitalRead(ALARM_MIN_PIN) == LOW && digitalRead(ALARM_RESET_PIN) != LOW && digitalRead(ALARM_HOU_PIN) != LOW) { // Only if Alarm minute button pressed
    setNightLight();
  }
  if(digitalRead(ALARM_MIN_PIN) == LOW && digitalRead(ALARM_RESET_PIN) == LOW && digitalRead(ALARM_HOU_PIN) != LOW) { // Only if Alarm minute and hour buttons
    displayLDRvalue();
  }
  if (digitalRead(ALARM_RESET_PIN) != LOW && digitalRead(ALARM_HOU_PIN) == LOW && digitalRead(ALARM_MIN_PIN) == LOW) { // Only if Alarm reset button pressed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LED flash");
    delay(1000);
    ledFlash();
  }
  float batteryVoltage = analogRead(BATTERY_PIN) * (5.0 / 1023.0) * 2;
  int batteryPercentage = map(batteryVoltage * 1000, MIN_BATTERY_VOLTAGE * 1000, MAX_BATTERY_VOLTAGE * 1000, 0, 100);
  batteryPercentage = constrain(batteryPercentage, 0, 100);
  displayBattery(batteryVoltage, batteryPercentage);
  handleLowBattery(batteryPercentage);
  delay(BATT_DISPLAY_DELAY);
  digitalWrite(BLINKING_LED,LOW);
}
// *Vibrate Once on Every Hour*
void vibrateOnHour() {
  digitalWrite(VIBRATION_PIN, HIGH);
  digitalWrite(BUZZER,HIGH);
  delay(500);  // Vibrate for 1.5 seconds
  digitalWrite(BUZZER, LOW);
  delay(500);
  digitalWrite(VIBRATION_PIN, LOW);
}
// *Handle Low Battery LED Blinking*
void handleLowBattery(int batteryPercentage) {
  DateTime now = rtc.now();  // Make sure this is available from RTC
  int hour = now.hour();
  String period = "AM";

  if (hour == 0) {
    hour = 12;
  } else if (hour >= 12) {
    if (hour > 12) hour -= 12;
    period = "PM";
  }

  if (batteryPercentage < LOW_BATTERY_THRESHOLD && batteryPercentage > 5) {
    for (int i = 0; i < 5; i++) {
      digitalWrite(A2, HIGH);
      digitalWrite(VIBRATION_PIN, HIGH);
      delay(75);
      digitalWrite(VIBRATION_PIN, LOW);
      delay(100);
      digitalWrite(A2, LOW);
      delay(100);
    }
  } else {
    digitalWrite(A2, LOW);
  }

  while (batteryPercentage <= 15) {
    float batteryVoltage = analogRead(BATTERY_PIN) * (5.0 / 1023.0) * 2;
    batteryPercentage = map(batteryVoltage * 1000, MIN_BATTERY_VOLTAGE * 1000, MAX_BATTERY_VOLTAGE * 1000, 0, 100);
    batteryPercentage = constrain(batteryPercentage, 0, 100);

    displayBattery(batteryVoltage, batteryPercentage);

    digitalWrite(BLINKING_LED, LOW);
    analogWrite(BACKLIGHT_PIN, 10);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Low Battery : ");
    lcd.print(batteryPercentage);

    now = rtc.now();  // Refresh current time
    hour = now.hour();
    period = "AM";
    if (hour == 0) {
      hour = 12;
    } else if (hour >= 12) {
      if (hour > 12) hour -= 12;
      period = "PM";
    }

    lcd.setCursor(0, 1);
    lcd.print("T: ");
    lcd.print(hour);
    lcd.print(":");
    lcd.print(now.minute() < 10 ? "0" : "");
    lcd.print(now.minute());
    lcd.print(":");
    lcd.print(now.second() < 10 ? "0" : "");
    lcd.print(now.second());
    lcd.print(" ");
    lcd.print(period);

    delay(1000);  // Add a delay so the loop isn’t too fast
  }
}
// *Control Night Light and Print LDR Value*
void controlNightLight(int currentHour){
  int ldrValue = analogRead(LDR_PIN);
  int state = digitalRead(NIGHT_LIGHT);  // Read the pin state
  Serial.println(state);
  bool isDark = ldrValue < 30; // Adjust threshold based on actual readings

  if ((currentHour >= 21 || currentHour < 6) && isDark && nightLightActive){
    int brightness = 0; // Start at 0 brightness
    int fadeSpeed = 5; // Adjust fade speed (higher = faster)
    if(state == 0 ){
      for (brightness = 0; brightness <= 255; brightness += fadeSpeed) {
       analogWrite(NIGHT_LIGHT, brightness); // Set LED brightness
       delay(50); // Delay to control fade speed
      }
    }
    analogWrite(NIGHT_LIGHT, setNightLightvalue);
  }
  else {
    if(state == 1 && nightLightActive){ // true 
     int fadeSpeed = 5; // Adjust fade speed (higher = faster)
     int brightness1 = 255; // Start at full brightness
     for (brightness1 = 255; brightness1 >= 0; brightness1 -= fadeSpeed) {
       analogWrite(NIGHT_LIGHT, brightness1); // Set LED brightness
       delay(50); // Delay to control fade speed
      }
      digitalWrite(NIGHT_LIGHT, LOW);  // Turn off Night Light
    }
  }
}

// *Adjust Backlight Based on LDR*
void adjustBacklight() {
  int ldrValue = analogRead(LDR_PIN);
  Serial.print("Backlight LDR Value: ");
  Serial.println(ldrValue);
  int brightness = map(ldrValue, 0, 1023, 5, 255); // Prevent complete darkness
  analogWrite(BACKLIGHT_PIN, brightness);
}
void displayTime(DateTime now) { // Display time on serial monitor
  lcd.clear();
  int hour = now.hour();
  String period = "AM";
  if (hour == 0) {
    hour = 12;
  } else if (hour >= 12) {
    if (hour > 12) hour -= 12;
    period = "PM";
  }
  lcd.setCursor(0, 0);
  lcd.print("T: ");
  lcd.print(hour);
  lcd.print(":");
  lcd.print(now.minute() < 10 ? "0" : "");
  lcd.print(now.minute());
  lcd.print(":");
  lcd.print(now.second() < 10 ? "0" : "");
  lcd.print(now.second());
  lcd.print(" ");
  lcd.print(period);
  lcd.setCursor(0, 1);
  lcd.print("D: ");
  lcd.print(now.day());
  lcd.print("/");
  lcd.print(now.month());
  lcd.print("/");
  lcd.print(now.year());
  lcd.print(" ");
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
}

void displayTemperatureHumidity(float temperature, float humidity) { // Display temperature and humidity on serial monitor
  lcd.clear();
  lcd.setCursor(0, 0);
  if (isnan(temperature) || isnan(humidity)) {
    lcd.print("Sensor Error");
  } else {
    lcd.print("Temp: ");
    lcd.print(temperature, 1);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(humidity, 1);
    lcd.print(" %");
  }
}

void displayBattery(float batteryVoltage, int batteryPercentage) { // Display battery voltage and percentage on serial monitor
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Battery: ");
  lcd.print(batteryPercentage);
  lcd.print(" %");
  lcd.setCursor(0, 1);
  lcd.print("Voltage: ");
  lcd.print(batteryVoltage, 2);
  lcd.print(" V");
}

void displayStartupAnimation() { // Display a simple animation on the LCD during startup
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Starting Up...");
  byte progressChar[8] = {0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
  lcd.createChar(0, progressChar);
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 1);
    lcd.write((byte)0);
    delay(250);
  }
  delay(500);
  lcd.clear();
}

void displayInitializingMessage() { // Display a message while the system is initializing
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HELLO !!");
  lcd.setCursor(0, 1);
  lcd.print("Welcome Back :)");
  delay(2000);
  lcd.clear();
}

void activateAlarm() { // Alarm is activated when the temperature exceeds 30 degrees
  alarmActive = true;
  for (int i = 0; i < 30 && alarmActive; i++) {
    digitalWrite(VIBRATION_PIN, HIGH);
    digitalWrite(A2, HIGH);
    digitalWrite(RESET_BUTTON_LED,HIGH);
    digitalWrite(HOU_BUTTON_LED,LOW);
    digitalWrite(MINUTE_BUTTON_LED,LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RED TO STOP");
    lcd.setCursor(0, 1);
    lcd.print("GREEN TO SNOOZE");
    delay(500);
    digitalWrite(VIBRATION_PIN, LOW);
    digitalWrite(RESET_BUTTON_LED,LOW);
    digitalWrite(HOU_BUTTON_LED,HIGH);
    digitalWrite(MINUTE_BUTTON_LED,HIGH);
    delay(100);

    for (int i = 0; i < 2; i++) { // blink 2 times
    digitalWrite(RESET_BUTTON_LED,HIGH);
    digitalWrite(HOU_BUTTON_LED,LOW);
    digitalWrite(MINUTE_BUTTON_LED,LOW);
    tone(BUZZER, 4000, 75);
    delay(200);
    digitalWrite(RESET_BUTTON_LED,LOW);
    digitalWrite(HOU_BUTTON_LED,HIGH);
    digitalWrite(MINUTE_BUTTON_LED,HIGH);
    }
    delay(900);

    if (digitalRead(ALARM_HOU_PIN) == LOW || digitalRead(ALARM_MIN_PIN) == LOW) {
      alarmMinute = (alarmMinute + 5) % 60;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ALARM SNOOZED");
      lcd.setCursor(0, 1);
      lcd.print("5 MINUTES");
      delay(2000);
      digitalWrite(A2, LOW);
      digitalWrite(HOU_BUTTON_LED,LOW);
      digitalWrite(MINUTE_BUTTON_LED,LOW);
      break;
    }

    if (digitalRead(ALARM_RESET_PIN) == LOW) {
      stopAlarm();
      digitalWrite(A2, LOW);
      digitalWrite(BUZZER, LOW);
      digitalWrite(RESET_BUTTON_LED,LOW);
      alarmMinute = EEPROM.read(alarmMinAddr);
      digitalWrite(HOU_BUTTON_LED,LOW);
      digitalWrite(MINUTE_BUTTON_LED,LOW);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ALARM STOPPED");
      break; // Exit loop immediately after stopping alarm
    }

    digitalWrite(A2, LOW);
    digitalWrite(BUZZER, LOW);
    digitalWrite(RESET_BUTTON_LED,LOW);
    digitalWrite(HOU_BUTTON_LED,HIGH);
    digitalWrite(MINUTE_BUTTON_LED,HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALARM STOPED");
    alarmMinute = EEPROM.read(alarmMinAddr);
  }
  digitalWrite(RESET_BUTTON_LED,LOW);
  digitalWrite(HOU_BUTTON_LED,LOW);
  digitalWrite(MINUTE_BUTTON_LED,LOW);
}

void editalarm(){
  int i = 0;
  int brightness0 = 0; // Start at 0 brightness
  int fadeSpeed = 5; // Adjust fade speed (higher = faster)
  int brightness1 = 255; // Start at full brightness

  for (brightness0 = 0; brightness0 <= 255; brightness0 += fadeSpeed) { // Fade in from 0 to 255
    analogWrite(RESET_BUTTON_LED, brightness0); // Set LED brightness
    delay(50); // Delay to control fade speed
  }

  digitalWrite(HOU_BUTTON_LED,HIGH);
  digitalWrite(MINUTE_BUTTON_LED,HIGH);

  while (digitalRead(ALARM_RESET_PIN) != LOW && i < 40) { // Wait for reset button press
    digitalWrite(BLINKING_LED, HIGH);

    if (digitalRead(ALARM_HOU_PIN) == LOW) { // If hour button is pressed
    digitalWrite(VIBRATION_PIN, HIGH);
    digitalWrite(HOU_BUTTON_LED,LOW);
    alarmHour = (alarmHour + 1) % 24;
    delay(50);
    digitalWrite(VIBRATION_PIN, LOW);
    digitalWrite(HOU_BUTTON_LED,HIGH);
    i = 0;
    }

   if (digitalRead(ALARM_MIN_PIN) == LOW) { // If minute button is pressed
    digitalWrite(VIBRATION_PIN, HIGH);
    digitalWrite(MINUTE_BUTTON_LED,LOW);
    alarmMinute = (alarmMinute + 1) % 60;
    delay(50);
    digitalWrite(VIBRATION_PIN, LOW);
    digitalWrite(MINUTE_BUTTON_LED,HIGH);
    i = 0;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarm Hour: ");
    lcd.print(alarmHour);
    lcd.setCursor(0, 1);
    lcd.print("Alarm Minute: ");
    lcd.print(alarmMinute);
    delay(100);
    digitalWrite(BLINKING_LED, LOW);
    delay(400);
    i = i + 1;
  }
  EEPROM.update(alarmHourAddr, alarmHour); // Save alarm hour to EEPROM
  EEPROM.update(alarmMinAddr, alarmMinute); // Store alarm time in EEPROM

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("New Alarm Time: ");
  lcd.setCursor(4, 1);
  lcd.print(alarmHour);
  lcd.print(":");
  lcd.print(alarmMinute);
  lcd.print(":");
  lcd.print("00");
  delay(2000);
  digitalWrite(HOU_BUTTON_LED,LOW);
  digitalWrite(MINUTE_BUTTON_LED,LOW);

  for (brightness1 = 255; brightness1 >= 0; brightness1 -= fadeSpeed) {
    analogWrite(RESET_BUTTON_LED, brightness1); // Set LED brightness
    delay(50); // Delay to control fade speed
  }
  digitalWrite(RESET_BUTTON_LED,LOW);
}

void stopAlarm() { // Stop alarm function
  alarmActive = false;
  digitalWrite(VIBRATION_PIN,LOW);
}

void activatereminder() { // Activate reminder function
  for(int i = 0; i < 30 ; i++) {
    digitalWrite(VIBRATION_PIN, HIGH);
    digitalWrite(A2, HIGH);
    digitalWrite(RESET_BUTTON_LED,HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PESS BUTTON TO");
    lcd.setCursor(0, 1);
    lcd.print("STOP REMIN !! ;)");
    delay(500);
    digitalWrite(VIBRATION_PIN, LOW);
    digitalWrite(RESET_BUTTON_LED,LOW);
    delay(100);

    for (int i = 0; i < 2; i++) { // blink 2 times
    digitalWrite(RESET_BUTTON_LED,HIGH);
    tone(BUZZER, 4000, 75);
    delay(200);
    digitalWrite(RESET_BUTTON_LED,LOW);
    }

    delay(900);
    if (digitalRead(ALARM_RESET_PIN) == LOW) {
      stopAlarm();
      digitalWrite(A2, LOW);
      digitalWrite(BUZZER, LOW);
      digitalWrite(RESET_BUTTON_LED,LOW);
      break; // Exit loop immediately after stopping alarm
    }

    digitalWrite(A2, LOW);
    digitalWrite(BUZZER, LOW);
    digitalWrite(RESET_BUTTON_LED,LOW);
  }
}

void startTimer() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Timer Started!");
  delay(1000);
  while (timerActive && digitalRead(ALARM_RESET_PIN) != LOW) {
      if (timerMinutes == 0 && timerSeconds == 0) {
        activateTimer(); // Trigger alarm when timer ends
        timerActive = false;
        return;
      }

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Time Left: ");
      lcd.print(timerMinutes);
      lcd.print(":");
      lcd.print(timerSeconds < 10 ? "0" : "");
      lcd.print(timerSeconds);
      if (timerSeconds == 0) {
        timerMinutes--;
        timerSeconds = 59;
      } else {
        timerSeconds--;
      }

    delay(500);  // Wait for 0.5 seconds before updating display
    digitalWrite(RESET_BUTTON_LED,HIGH);
    digitalWrite(BLINKING_LED,LOW);
    delay(500);
    digitalWrite(RESET_BUTTON_LED,LOW);
    digitalWrite(BLINKING_LED,HIGH);
  }
  digitalWrite(BLINKING_LED,LOW);
}

void setTimer() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Timer:");
  int i = 0;
  int brightness0 = 0; // Start at 0 brightness
  int fadeSpeed = 5; // Adjust fade speed (higher = faster)
  int brightness1 = 255; // Start at full brightness
  delay(1000);
  for (brightness0 = 0; brightness0 <= 255; brightness0 += fadeSpeed) { // Fade in from 0 to 255
    analogWrite(RESET_BUTTON_LED, brightness0); // Set LED brightness
    delay(50); // Delay to control fade speed
  }

  digitalWrite(HOU_BUTTON_LED,HIGH);
  digitalWrite(MINUTE_BUTTON_LED,HIGH);
  while (digitalRead(ALARM_RESET_PIN) != LOW && i < 60) {
    if (digitalRead(ALARM_HOU_PIN) == LOW) { // Increase minutes
      timerMinutes = (timerMinutes + 1) % 60;
      digitalWrite(HOU_BUTTON_LED,LOW);
      digitalWrite(VIBRATION_PIN,HIGH);
      delay(100);
      digitalWrite(HOU_BUTTON_LED,HIGH);
      digitalWrite(VIBRATION_PIN,LOW);
      i = 0;
      }

    if (digitalRead(ALARM_MIN_PIN) == LOW) { // Increase seconds
      timerSeconds = (timerSeconds + 10) % 60;
      digitalWrite(MINUTE_BUTTON_LED,LOW);
      digitalWrite(VIBRATION_PIN,HIGH);
      delay(100);
      digitalWrite(MINUTE_BUTTON_LED,HIGH);
      digitalWrite(VIBRATION_PIN,LOW);
      i = 0 ;
    }
    lcd.setCursor(0, 1);
    lcd.print(timerMinutes);
    lcd.print(":");
    lcd.print(timerSeconds < 10 ? "0" : "");
    lcd.print(timerSeconds);
    digitalWrite(BLINKING_LED,HIGH);
    delay(200);
    digitalWrite(BLINKING_LED,LOW);
    i ++;
  }

  timerActive = true;
  digitalWrite(HOU_BUTTON_LED,LOW);
  digitalWrite(MINUTE_BUTTON_LED,LOW);
  for (brightness1 = 255; brightness1 >= 0; brightness1 -= fadeSpeed) {
    analogWrite(RESET_BUTTON_LED, brightness1); // Set LED brightness
    delay(50); // Delay to control fade speed
  }
  digitalWrite(RESET_BUTTON_LED,LOW);
  startTimer();
}

void activateTimer() { // Alarm is activated when the temperature exceeds 30 degrees
  alarmActive = true;
  for (int i = 0; i < 30 && alarmActive; i++) {
    digitalWrite(VIBRATION_PIN, HIGH);
    digitalWrite(A2, HIGH);
    digitalWrite(RESET_BUTTON_LED,HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Press Button");
    lcd.setCursor(0, 1);
    lcd.print("To Stop ;)");
    delay(500);
    digitalWrite(VIBRATION_PIN, LOW);
    digitalWrite(RESET_BUTTON_LED,LOW);
    delay(100);

    for (int i = 0; i < 2; i++) { // blink 2 times
    digitalWrite(RESET_BUTTON_LED,HIGH);
    tone(BUZZER, 4000, 75);
    delay(200);
    digitalWrite(RESET_BUTTON_LED,LOW);
    }

    delay(900);
    if (digitalRead(ALARM_RESET_PIN) == LOW) {
      stopAlarm();
      digitalWrite(A2, LOW);
      digitalWrite(BUZZER, LOW);
      digitalWrite(RESET_BUTTON_LED,LOW);
      break; // Exit loop immediately after stopping alarm
    }

    digitalWrite(A2, LOW);
    digitalWrite(BUZZER, LOW);
    digitalWrite(RESET_BUTTON_LED,LOW);
  }
}

void editreminder(){
  int brightness0 = 0; // Start at 0 brightness
  int fadeSpeed = 5; // Adjust fade speed (higher = faster)
  int brightness1 = 255; // Start at full brightness
  for (brightness0 = 0; brightness0 <= 255; brightness0 += fadeSpeed) { // Fade in from 0 to 255
    analogWrite(RESET_BUTTON_LED, brightness0); // Set LED brightness
    delay(50); // Delay to control fade speed
  }

  digitalWrite(HOU_BUTTON_LED,HIGH);
  digitalWrite(MINUTE_BUTTON_LED,HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Edit remin Date");
  delay(1000);
  int i = 0;

  while(digitalRead(ALARM_RESET_PIN) != LOW && i < 20) { // Wait for reset button press
    digitalWrite(BLINKING_LED, HIGH);
    if (digitalRead(ALARM_HOU_PIN) == LOW) { // If hour button is pressed
    digitalWrite(VIBRATION_PIN, HIGH);
    digitalWrite(HOU_BUTTON_LED,LOW);
    remindermonth = (remindermonth + 1) % 13;
    delay(50);
    digitalWrite(VIBRATION_PIN, LOW);
    digitalWrite(HOU_BUTTON_LED,HIGH);
    i = 0;
    }

   if (digitalRead(ALARM_MIN_PIN) == LOW) { // If minute button is pressed
    digitalWrite(VIBRATION_PIN, HIGH);
    digitalWrite(MINUTE_BUTTON_LED,LOW);
    reminderday = (reminderday + 1) % 32;
    delay(50);
    digitalWrite(VIBRATION_PIN, LOW);
    digitalWrite(MINUTE_BUTTON_LED,HIGH);
    i = 0;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Remin Month: ");
    lcd.print(remindermonth);
    lcd.setCursor(0, 1);
    lcd.print("Remin Day: ");
    lcd.print(reminderday);
    delay(100);
    digitalWrite(BLINKING_LED, LOW);
    delay(400);
    i = i + 1;
  }
  EEPROM.update(remindermonthAddr, remindermonth);
  EEPROM.update(reminderdayAddr, reminderday);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Edit remin Time");
  delay(1000);
  i = 0;
  while (digitalRead(ALARM_RESET_PIN) != LOW && i < 20) { // Wait for reset button press
    digitalWrite(BLINKING_LED, HIGH);
    if (digitalRead(ALARM_HOU_PIN) == LOW) { // If hour button is pressed
    digitalWrite(VIBRATION_PIN, HIGH);
    digitalWrite(HOU_BUTTON_LED,LOW);
    reminderhour = (reminderhour + 1) % 24;
    delay(50);
    digitalWrite(VIBRATION_PIN, LOW);
    digitalWrite(HOU_BUTTON_LED,HIGH);
    i = 0;
    }

   if (digitalRead(ALARM_MIN_PIN) == LOW) { // If minute button is pressed
    digitalWrite(VIBRATION_PIN, HIGH);
    digitalWrite(MINUTE_BUTTON_LED,LOW);
    reminderminute = (reminderminute + 1) % 60;
    delay(50);
    digitalWrite(VIBRATION_PIN, LOW);
    digitalWrite(MINUTE_BUTTON_LED,HIGH);
    i = 0;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Remin Hour: ");
    lcd.print(reminderhour);
    lcd.setCursor(0, 1);
    lcd.print("Remin Minute: ");
    lcd.print(reminderminute);
    delay(100);
    digitalWrite(BLINKING_LED, LOW);
    delay(400);
    i = i + 1;
  }
  EEPROM.update(reminderhourAddr, reminderhour);
  EEPROM.update(reminderminuteAddr, reminderminute);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("New Reminder: ");
  lcd.setCursor(0, 1);
  lcd.print(reminderhour);
  lcd.print(":");
  lcd.print(reminderminute);
  lcd.print(":");
  lcd.print("00");
  lcd.print(" ");
  lcd.print(reminderday);
  lcd.print("/");
  lcd.print(remindermonth);
  lcd.print("/");
  lcd.print("25");
  delay(2000);
  digitalWrite(HOU_BUTTON_LED,LOW);
  digitalWrite(MINUTE_BUTTON_LED,LOW);

  for (brightness1 = 255; brightness1 >= 0; brightness1 -= fadeSpeed) {
    analogWrite(RESET_BUTTON_LED, brightness1); // Set LED brightness
    delay(50); // Delay to control fade speed
  }
  digitalWrite(RESET_BUTTON_LED,LOW);
}

void setNightLight(){
  int i = 0;
  int brightness0 = 0; // Start at 0 brightness
  int fadeSpeed = 5; // Adjust fade speed (higher = faster)
  int brightness1 = 255; // Start at full brightness

  for (brightness0 = 0; brightness0 <= 255; brightness0 += fadeSpeed) { // Fade in from 0 to 255
    analogWrite(RESET_BUTTON_LED, brightness0); // Set LED brightness
    delay(50); // Delay to control fade speed
  }

  digitalWrite(HOU_BUTTON_LED,HIGH);
  digitalWrite(MINUTE_BUTTON_LED,HIGH);
  while (digitalRead(ALARM_RESET_PIN) != LOW && i < 20) { // Wait for reset button press
    digitalWrite(BLINKING_LED, HIGH);
    if (digitalRead(ALARM_HOU_PIN) == LOW) { // If hour button is pressed
     digitalWrite(VIBRATION_PIN, HIGH);
     digitalWrite(HOU_BUTTON_LED,LOW);
     setNightLightvalue = abs((setNightLightvalue + 10)) % 255;
     EEPROM.update(setNightLightvalueAddr, setNightLightvalue);
     delay(50);
     digitalWrite(VIBRATION_PIN, LOW);
     digitalWrite(HOU_BUTTON_LED,HIGH);
     i = 0;
    }

   if (digitalRead(ALARM_MIN_PIN) == LOW) { // If minute button is pressed
     digitalWrite(VIBRATION_PIN, HIGH);
     digitalWrite(MINUTE_BUTTON_LED,LOW);
     setNightLightvalue = abs((setNightLightvalue - 10)) % 255;
     EEPROM.update(setNightLightvalueAddr, setNightLightvalue);
     delay(50);
     digitalWrite(VIBRATION_PIN, LOW);
     digitalWrite(MINUTE_BUTTON_LED,HIGH);
    i = 0;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Night light");
    lcd.setCursor(0, 1);
    lcd.print("value : ");
    lcd.print(setNightLightvalue);
    delay(100);
    digitalWrite(BLINKING_LED, LOW);
    delay(200);
    i = i + 1;
    analogWrite(NIGHT_LIGHT, setNightLightvalue);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Night light");
  lcd.setCursor(0, 1);
  lcd.print("Value :");
  lcd.print(setNightLightvalue);
  delay(2000);
  digitalWrite(HOU_BUTTON_LED,LOW);
  digitalWrite(MINUTE_BUTTON_LED,LOW);

  for (brightness1 = 255; brightness1 >= 0; brightness1 -= fadeSpeed) {
    analogWrite(RESET_BUTTON_LED, brightness1); // Set LED brightness
    delay(50); // Delay to control fade speed
  }
  digitalWrite(RESET_BUTTON_LED,LOW);
  analogWrite(NIGHT_LIGHT, LOW);
}

void displayLDRvalue(){
  int i = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LDR value");
  delay(1000);

  while(digitalRead(ALARM_RESET_PIN) != LOW && i < 120) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LDR Value : ");
    int LDRvalue = analogRead(LDR_PIN);
    lcd.print(LDRvalue);
    i=i+1;
    delay(500);
  }
}

void ledFlash(){
  int i = 0;
  while(digitalRead(ALARM_RESET_PIN) != LOW && i < 120) { // Wait for reset button press
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("R TO LED FLASHING");
    lcd.setCursor(0, 1);
    lcd.print("L TO LED ON/OFF");
    digitalWrite(RESET_BUTTON_LED,HIGH);
    digitalWrite(HOU_BUTTON_LED, HIGH);
    digitalWrite(MINUTE_BUTTON_LED, HIGH);
    delay(1000);
    if(digitalRead(ALARM_HOU_PIN) == LOW){
      int i = 0;
      digitalWrite(RESET_BUTTON_LED,HIGH);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("LED FLASHING");
      delay(1000);
      while(digitalRead(ALARM_RESET_PIN) != LOW && i < 240) { 
       lcd.setCursor(0, 1);
       lcd.print(i);
       digitalWrite(NIGHT_LIGHT, HIGH);  // Turn the LED on
       delay(100);              // Wait for 500ms
       digitalWrite(NIGHT_LIGHT, LOW);   // Turn the LED off
       delay(100);
       i=i+1;
      }
      delay(1000);
    }
    if(digitalRead(ALARM_MIN_PIN) == LOW){
      int i = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("LED ON/OFF");
      delay(1000);
      while(digitalRead(ALARM_RESET_PIN) != LOW && i < 240){   
       if(digitalRead(NIGHT_LIGHT) == LOW && digitalRead(ALARM_MIN_PIN) == LOW){
         digitalWrite(NIGHT_LIGHT, HIGH);  // Turn the LED on
         lcd.clear();
         lcd.setCursor(0, 0);
         lcd.print("LED ON");
         nightLightActive = false;
         delay(1000);
        }    
       if(digitalRead(ALARM_MIN_PIN) == LOW && digitalRead(NIGHT_LIGHT) == HIGH){
         digitalWrite(NIGHT_LIGHT, LOW);
         lcd.clear();
         lcd.setCursor(0, 0);
         lcd.print("LED OFF");
         nightLightActive = true;
         delay(1000);  // Turn the LED off
        }
       i=i+1;
       delay(100);
      }
    }
    digitalWrite(RESET_BUTTON_LED,LOW);
    digitalWrite(HOU_BUTTON_LED,LOW);
    digitalWrite(MINUTE_BUTTON_LED,LOW);
    i = i+1;
    delay(100);
  }
}
