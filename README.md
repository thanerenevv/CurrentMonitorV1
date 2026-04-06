## Current Monitor

A project where it uses an ESP32-C3 and a ACS712 30A with a SSD1306 0.96 OLED for measuring how many currents a device eats in mains voltage.

Important!! Please edit config.h if your main voltage is not 230V AC, but if your mains is 230V AC then you can flash at https://thanerenevv.github.io/CurrentMonitorV1/

<img width="500" alt="Screenshot 2569-04-06 at 19 43 56" src="https://github.com/user-attachments/assets/4900e833-3c54-4f5b-b02c-1ed37d149a67" />

Wiring Diagram 
| Component | Pin | ESP32-C3 Pin |
|-----------|-----|--------------|
| ACS712 | VCC | 3.3V |
| ACS712 | GND | GND |
| ACS712 | OUT | GPIO 0 (ADC) |
| SSD1306 OLED | VCC | 3.3V |
| SSD1306 OLED | GND | GND |
| SSD1306 OLED | SCL | GPIO 9 |
| SSD1306 OLED | SDA | GPIO 8 |
| Capacitor 100nF | Pin 1 | ACS712 OUT |
| Capacitor 100nF | Pin 2 | GND |
