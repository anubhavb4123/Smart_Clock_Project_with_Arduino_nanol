#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <DHT.h>

// Pin configuration
#define DHTPIN 2        // Pin connected to DHT sensor
#define DHTTYPE DHT11   // Change to DHT22 if using DHT22
#define LDR_PIN A0      // LDR connected to analog pin A0
#define BATTERY_PIN A1  // Battery voltage sensing pin
#define BACKLIGHT_PIN 9 // LCD backlight connected to PWM pin 9
#define VIBRATION_PIN 12 // Vibration motor connected to pin 12
#define HOU_BUTTON_LED 8  // LED for low battery warning
#define NIGHT_LIGHT 11   // Night light LED
#define ALARM_RESET_PIN 3 // Reset button for stopping alarm
#define ALARM_MIN_PIN 4  // Pin for setting minimum alarm time
#define ALARM_HOU_PIN 5 // Pin for setting maximum alarm time
#define BUZZER 6  // Buzzer connected to pin 6
#define BLINKING_LED 13  // Use built-in LED or another available pin
#define RESET_BUTTON_LED 10 // Hour button LED
#define MINUTE_BUTTON_LED 7 // Minute button LED

// Constants
#define MAX_BATTERY_VOLTAGE 4.2  // Max voltage (fully charged battery)
#define MIN_BATTERY_VOLTAGE 3.0  // Min voltage (fully discharged)
#define LOW_BATTERY_THRESHOLD 21 // Battery % threshold for LED warning
#define TIME_DISPLAY_DELAY 5000  // Delay for time/date screen
#define TEMP_DISPLAY_DELAY 3000  // Delay for temp/humidity screen
#define BATT_DISPLAY_DELAY 2000  // Delay for battery volt and %

RTC_DS1307 rtc;
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust I2C address (0x27 is common)

int lastVibratedHour = -1;  // Store last hour when the vibration was triggered
int lastAlarmHour = -1; // Stores the last hour when the alarm was triggered
int alarmHour = 7;
int alarmMinute = 30;
bool alarmActive = false;
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

  delay(500); // Wait for 0.5 seconds before starting the display animation
  digitalWrite(VIBRATION_PIN, LOW); // Turn off vibration motor
  digitalWrite(HOU_BUTTON_LED, LOW); // Turn on low battery LED
  digitalWrite(NIGHT_LIGHT, LOW);  // Turn on night light
  digitalWrite(BUZZER, LOW); // Turn on buzzer
  digitalWrite(BLINKING_LED, LOW);  // Turn on blinking LED
  digitalWrite(RESET_BUTTON_LED, LOW); // Turn on hour button LED  
  digitalWrite(MINUTE_BUTTON_LED, LOW); // Turn on minute button LED
  digitalWrite(A2, LOW); // Alarm reset button LED OFF initially

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
  DateTime now = rtc.now();
  
  if (digitalRead(ALARM_RESET_PIN) == LOW) { // Alarm reset button pressed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Edit Alarm time");
    lcd.setCursor(4, 1);
    lcd.print("#$%^&*=!");
    delay(1000);
    editalarm();
  }

  // Hourly vibration alert (one-time vibration)
  if (now.minute() == 0 && lastVibratedHour != now.hour()) {
    vibrateOnHour();
    lastVibratedHour = now.hour();
  }
  if (now.hour() == alarmHour && now.minute() == alarmMinute && lastAlarmHour != now.hour()) {
   activateAlarm();
   lastAlarmHour = now.hour(); // Update last alarm hour
  }
  // Control Night Light
  controlNightLight(now.hour());
  displayTime(now);
  delay(TIME_DISPLAY_DELAY);

  if (digitalRead(ALARM_RESET_PIN) == LOW) { // Check if reset button is pressed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Edit Alarm time");
    lcd.setCursor(4, 1);
    lcd.print("#$%^&*=!");
    delay(1000);
    editalarm();
  }

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  displayTemperatureHumidity(temperature, humidity);
  delay(TEMP_DISPLAY_DELAY);

  if (digitalRead(ALARM_RESET_PIN) == LOW) { // Check if reset button is pressed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Edit Alarm time");
    lcd.setCursor(4, 1);
    lcd.print("#$%^&*=!");
    delay(1000);
    editalarm();
  }

  float batteryVoltage = analogRead(BATTERY_PIN) * (5.0 / 1023.0) * 2;
  int batteryPercentage = map(batteryVoltage * 1000, MIN_BATTERY_VOLTAGE * 1000, MAX_BATTERY_VOLTAGE * 1000, 0, 100);
  batteryPercentage = constrain(batteryPercentage, 0, 100);

  displayBattery(batteryVoltage, batteryPercentage);
  handleLowBattery(batteryPercentage);
  delay(BATT_DISPLAY_DELAY);
}

// *Vibrate Once on Every Hour*
void vibrateOnHour() {
  digitalWrite(VIBRATION_PIN, HIGH);
  delay(1000);  // Vibrate for 1.5 seconds
  digitalWrite(VIBRATION_PIN, LOW);
  digitalWrite(BUZZER, HIGH);
  delay(500);
  digitalWrite(BUZZER, LOW);
}

// *Handle Low Battery LED Blinking*
void handleLowBattery(int batteryPercentage) {
  if (batteryPercentage < LOW_BATTERY_THRESHOLD) {
    for (int i = 0; i < 5; i++) {  // Blink 5 times
      digitalWrite(A2, HIGH);
      digitalWrite(VIBRATION_PIN, HIGH);
      delay(75);
      digitalWrite(VIBRATION_PIN, LOW);
      delay(100);
      digitalWrite(A2, LOW);
      delay(100);
    }
  } else {
    digitalWrite(A2, LOW); // Keep LED OFF if battery is normal
  }
}

// *Control Night Light and Print LDR Value*
void controlNightLight(int currentHour) {
  int ldrValue = analogRead(LDR_PIN);
  Serial.print("LDR Value: ");
  Serial.println(ldrValue);
  int state = digitalRead(NIGHT_LIGHT);  // Read the pin state
  Serial.println(state);
  bool isDark = ldrValue < 30; // Adjust threshold based on actual readings

  if ((currentHour >= 21 || currentHour < 6) && isDark) {
   int brightness = 0; // Start at 0 brightness
   int fadeSpeed = 5; // Adjust fade speed (higher = faster)
  if(state == 0 ){
   for (brightness = 0; brightness <= 255; brightness += fadeSpeed) {
     analogWrite(NIGHT_LIGHT, brightness); // Set LED brightness
     delay(50); // Delay to control fade speed
    }
  }
  digitalWrite(NIGHT_LIGHT, HIGH); // Turn on Night Light
  } else {
   if(state == 1 ){ 
     int fadeSpeed = 5; // Adjust fade speed (higher = faster)
     int brightness1 = 255; // Start at full brightness
     for (brightness1 = 255; brightness1 >= 0; brightness1 -= fadeSpeed) {
       analogWrite(NIGHT_LIGHT, brightness1); // Set LED brightness
       delay(50); // Delay to control fade speed
      }
    }
  digitalWrite(NIGHT_LIGHT, LOW);  // Turn off Night Light
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
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Press Button to");
    lcd.setCursor(0, 1);
    lcd.print("Stop Alarm ;)");
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

  while (digitalRead(ALARM_RESET_PIN) != LOW && i < 20) { // Wait for reset button press
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