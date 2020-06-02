# Particle Photon - Internet of Things Project

Projects for IoT using [Particle Photon](Core).

![Particle Photon Pinout](/res/photon_pinout.png)

## Sensors
1. BME280 - an environmental sensor that reads temperature, barometric pressure and humidity.

## Displays
1. SSD1306 OLED Monochrome display

## Services
1. [Particle] - Full-stack Internet of Things (IoT) device platform that gives you everything you need to securely and reliably connect your IoT devices to the web.
2. [Blynk] - The most popular mobile app for the IOT.

[Particle Photon]: https://docs.particle.io/guide/getting-started/intro/core/
[Particle]: https://www.particle.io/
[Blynk]: https://blynk.cc

### Notes
1. If you plan to use the Blynk integration, rename or delete `bme280-oled.ino` when compiling and vice-versa (`bme280-oled-blynk.ino`). Only 1 instance of the *.ino file should exist.


![Testboard Image](/res/photon_bme280_oled_testboard.jpg)
