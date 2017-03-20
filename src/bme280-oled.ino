/*******************************************************************************************
 BME280 sensor data display on OLED SSD1306 using Particle Photon.
  There is a command to publish the data to Particle.io cloud.
 Hardware:
  BME280 sensor (I2C) - http://www.ebay.com.au/itm/like/371887618940?chn=ps
  OLED SSD1306 128x64 display (I2C) - https://www.adafruit.com/product/938 or clone
  Particle Photon (with headers) - https://www.adafruit.com/product/2721
  See bme280-oled.fzz (Fritzing circuit) for connections
 Software:
  Adafruit_SSD1306_RK Particle library (1.1.2)
  Adafruit_BME280 Particle library (1.1.3)
 Particle Functions:
  disp_text (text_to_display) - Shows custom text on OLED display
  disp_clear (delay_seconds) - Clears OLED display
  i2c_scan (optional) - Utility to identify I2C addresses
  bme280 (command)    - Read and displa BME280 sensor data;
                        command is either: (Particle)publish or (OLED)display(default)
 Particle Variables:
  temperature - degrees Celsius
  humidity - relative humidity in %
  pressure - atmospheric pressure in hPa
  sealevelpressure - sea level pressure in hPa
  altitude - in meters
********************************************************************************************/
#include <Wire.h>
#include <Adafruit_SSD1306_RK.h>
#include <Adafruit_BME280.h>
#include <string>

#include "font.h"
#include "settings.h"

int loop_display = 0;
int loop_publish = 0;
double temperature = 0.0F;
double pressure = 0.0F;
double sealevelpressure = 0.0F;
double humidity = 0.0F;
double altitude = 0.0F;

Adafruit_BME280 bme; // I2C
Adafruit_SSD1306 display(-1);

void setup()   {
  Serial.begin(9600);

  // Initialize and powerup our display
  display.begin(SSD1306_SWITCHCAPVCC, SSD1306_ADDRESS);
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();

  Particle.function("disp_text", displayText);
  Particle.function("disp_clear", displayClear);
  Particle.function("i2c_scan", i2cScan);
  Particle.function("bme280", bme280Read);

  Particle.variable("temp", temperature);
  Particle.variable("pressure", pressure);
  Particle.variable("seapressure", sealevelpressure);
  Particle.variable("humidity", humidity);
  Particle.variable("altitude", altitude);

  if (!bme.begin()) {
      displayText("Could not find a valid BME280 sensor, check wiring!");
  } else {
    Serial.print("Found BME280 I2C address: 0x");
    Serial.print(BME280_ADDRESS,HEX);
    Serial.println(" !");
    loop_display = 1;   //Display sensor data immediately
  }
}

void loop() {
  if (loop_display > 0) {
    bme280Read("display");
  }
  if (loop_publish > 0) {
    bme280Read("publish");
  }
  delay(REFRESH_INTERVAL);
}

int bme280Read(String command) {
  temperature = bme.readTemperature();
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  pressure =  ( bme.readPressure()/100.0F );
  sealevelpressure = bme.seaLevelForAltitude(altitude, pressure);
  humidity = bme.readHumidity();
  if (command == "publish") {
    Particle.publish("temperature", String::format("%.02f °C", temperature));
    Particle.publish("pressure", String::format("%.02f hPa", pressure));
    Particle.publish("sealevelpressure", String::format("%.02f hPa", sealevelpressure));
    Particle.publish("altitude", String::format("%.02f m", altitude));
    Particle.publish("humidity", String::format("%.02f %%", humidity));
    loop_publish = 1;
    return 2;
  } else { //Default display to OLED
    String data = "";
    data.concat("Temperature: "+String::format("%.02f C\n", temperature));
    data.concat("Humidity: "+String::format("%.02f %%\n", humidity));
    data.concat("Pressure: "+String::format("%.02f hPa\n", pressure));
    data.concat("Sealevel: "+String::format("%.02f hPa\n", sealevelpressure));
    data.concat("Altitude: "+String::format("%.02f m\n", altitude));
    displayText(data);
    loop_display = 1;
    return 1;
  }
}

int i2cScan(String command) {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning...");

  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done.\n");

  delay(5000);           // wait 5 seconds for next scan
  return nDevices;
}
int displayClear(String command) {
  int start = atoi(command);
  if (start > 0) {
    Serial.print("Clearing display in ");
    Serial.print(start);
    Serial.println(" seconds...\n");
    delay(start * 1000);
  }
  display.clearDisplay();
  display.display();
  loop_display = 0;
  Serial.println("Display cleared.");
  return 1;
}
int displayText(String text) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.setFont(&Monospaced_plain_8);
  display.println(text);
  display.display();
  display.setFont();
  return 1;
}
