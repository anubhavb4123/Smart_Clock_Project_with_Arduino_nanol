# Smart_Clock_Project_with_Arduino_nano

## Description
An Arduino-based smart clock system that combines multiple functionalities like real-time clock, environmental monitoring, reminders, alarms, and battery management — all displayed on an LCD interface.

## Features
- 🕒 Real-time clock with date using RTC DS1307
- 🌡️ Temperature and humidity display via DHT11 sensor
- ⏰ Configurable alarm and reminder system with EEPROM memory
- 🔋 Battery voltage and percentage monitoring
- 💡 Ambient light-based LCD backlight and night light control
- 📟 LCD display cycles between time, temp/humidity, and battery stats
- 🔔 Buzzer and vibration motor for alerts

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
