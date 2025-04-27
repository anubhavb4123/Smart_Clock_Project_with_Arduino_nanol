# Smart_Clock_Project_with_Arduino_nano

## Description
An Arduino-based smart clock system that combines multiple functionalities like real-time clock, environmental monitoring, reminders, alarms, and battery management — all displayed on an LCD interface.

  Condition                                                               Action                	Function Called

          
ALARM_RESET_PIN == LOW && ALARM_HOU_PIN != LOW && ALARM_MIN_PIN != LOW	  ->  Edit Alarm Time	  -->>   editalarm()

ALARM_HOU_PIN == LOW && ALARM_RESET_PIN != LOW && ALARM_MIN_PIN != LOW	  ->  Set Timer	        -->>   setTimer()

ALARM_HOU_PIN == LOW && ALARM_RESET_PIN == LOW && ALARM_MIN_PIN != LOW    ->	 Edit Reminder      -->>   editreminder()

ALARM_MIN_PIN == LOW && ALARM_RESET_PIN != LOW && ALARM_HOU_PIN != LOW	  ->  Set Night Light	  -->>   setNightLight()

ALARM_MIN_PIN == LOW && ALARM_RESET_PIN == LOW && ALARM_HOU_PIN != LOW    ->	Display LDR Value   -->>	displayLDRvalue()

ALARM_RESET_PIN != LOW && ALARM_HOU_PIN == LOW && ALARM_MIN_PIN == LOW    ->  LED Flash	        -->>   ledFlash()


## Features
- 🕒 Real-time clock with date using RTC DS1307
- 🌡️ Temperature and humidity display via DHT11 sensor
- ⏰ Configurable alarm and reminder system with EEPROM memory
- 🔋 Battery voltage and percentage monitoring
- 💡 Ambient light-based LCD backlight and night light control
- 📟 LCD display cycles between time, temp/humidity, and battery stats
- 🔔 Buzzer and vibration motor for alerts
- ✅ Time & Date Display

Uses RTC DS1307 for real-time clock.

Displays 12-hour AM/PM format.

Shows day, date, month, and year on LCD (I2C 16x2).

✅ Alarm System

User-configurable alarm time (hour & minute).

Buzzer & vibration motor activate on alarm.

LED indicator blinks when alarm triggers.

Alarm Reset Button stops the alarm.

Supports reminder alarms (custom date, hour, minute).

✅ Night Light Control

LDR sensor detects ambient light.

Night light automatically turns on between 9 PM - 6 AM if it's dark.

Smooth fading effect for light transition.

✅ Temperature & Humidity Monitoring

Uses DHT11 sensor to measure temperature & humidity.

Displays temperature (°C) and humidity (%) on LCD.

Error alert if sensor fails to read data.

✅ Battery Monitoring System

Voltage sensing for battery level measurement.

Displays battery percentage & voltage (V) on LCD.

Low battery warning LED blinks if battery is below 21%.

Vibrates 5 times as a low-battery warning.

✅ Hourly Vibration Alert

Vibration motor & buzzer activate every new hour.

Prevents repeated alerts in the same hour.

✅ Adjustable LCD Backlight

LDR sensor auto-adjusts LCD brightness.

Ensures optimal visibility based on ambient light.

✅ Interactive Alarm & Reminder Settings

Hour & minute buttons to set alarm/reminder.

LED indicators assist in setting time.

Smooth LED brightness transition while adjusting time.

✅ LED & Buzzer Control

Blinking LED for alarm indication.

Buzzer beeps at different frequencies for different alerts.

✅ Startup Animation & Welcome Message

LCD displays "Starting Up..." animation at power-on.

Shows custom welcome message ("Hello! Welcome Back :)").

## Components Used
- Arduino UNO/Nano
- DHT11 Temperature and Humidity Sensor
- DS1307 RTC Module
- I2C LCD Display (16x2)
- Buzzer
- Vibration Motor
- LDR (Light Dependent Resistor)
- LEDs and Push Buttons
- Battery and Voltage Divider Circuit

## How it Works
1. **Startup**:
   - Displays initialization animation and status on the LCD.
   - Retrieves saved settings (alarm, reminders, night light brightness) from EEPROM.

2. **Clock Functionality**:
   - Continuously displays current time and date from the RTC.
   - Rotates between different screens (Time/Date, Temp/Humidity, Battery Level).

3. **Environment Monitoring**:
   - DHT11 reads temperature and humidity every few seconds.
   - Display updates accordingly.

4. **Alarm & Reminders**:
   - Alarm time is stored in EEPROM.
   - When the alarm time matches the current time, the buzzer and vibration motor activate.
   - A separate reminder system is also triggered based on saved date/time.

5. **Battery Monitoring**:
   - Reads analog voltage from battery pin.
   - Calculates and displays battery percentage.
   - If below a threshold, activates a warning LED.

6. **Lighting Control**:
   - LDR checks ambient light.
   - Adjusts LCD backlight and night light brightness based on environment and time.

## Getting Started
1. Connect all components according to the pin definitions in the `.ino` file.
2. Open the `.ino` file in the Arduino IDE.
3. Select the appropriate board and COM port.
4. Upload the sketch.
5. Use push buttons to configure alarm and reminder times.

## Pin Configuration
Refer to the pin mappings in the code comments at the top of the `.ino` file.
