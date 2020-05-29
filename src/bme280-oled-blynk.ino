/*******************************************************************************************
 BME280 sensor data display on OLED SSD1306 using Particle Photon.
  There is a `blynk` command to publish data to Blynk server/cloud.
  There's optional `battery` stat display when connecting a LiPo battery.
 Hardware:
  BME280 sensor (I2C) - http://www.ebay.com.au/itm/like/371887618940?chn=ps
  OLED SSD1306 128x64 display (I2C) - https://www.adafruit.com/product/938 or clone
  Particle Photon (with headers) - https://www.adafruit.com/product/2721
    * See bme280-oled.fzz (Fritzing circuit) for connections
  SparkFun Battery Shield for Photon (optional) - to power Photon using LiPo battery
 Software:
  Adafruit_SSD1306_RK Particle library (1.1.2)
  Adafruit_BME280 Particle library (1.1.3)
  SparkFunMAX17043 Particle library (1.1.2)
  blynk Particle library (0.4.6)
 Particle Functions:
  text (text_to_display) - Shows custom text on OLED display
  clear (delay_seconds) - Clears OLED display
  sensor (command)    - Display or publish BME280 sensor data;
                        Use `blynk` command to publish to Blynk server,
                        otherwise display on OLED, `stop` to stop all readings
  battery (command)   - Display or publish Battery information;
                        Use `blynk` command to publish, otherwise display on OLED
 Particle Variables:
  temperature - degrees Celsius
  humidity - relative humidity in %
  pressure - atmospheric pressure in hPa
  sealevelpressure - sea level pressure in hPa
  altitude - in meters
  voltage - battery supply output Voltage (Vout)
  soc - remaining battery (%)
  alert - battery alert status
********************************************************************************************/
#include <Adafruit_SSD1306_RK.h>
#include "settings.h"
#include <Adafruit_BME280.h>
#include <string>
#include "font.h"

#include "SparkFunMAX17043.h" // Include the SparkFun MAX17043 library

#define BLYNK_HEARTBEAT 60
#define BLYNK_PRINT     Serial  // Set serial output for debug prints
#define BLYNK_IP        IPAddress(192,168,0,5) //Local Blynk server IP

#include <blynk.h>
char auth[] = "<Enter your Authorization Code Here>"; //Blynk Authorization

double voltage = 0; // Variable to keep track of LiPo voltage
double soc = 0; // Variable to keep track of LiPo state-of-charge (SOC)
bool alert; // Variable to keep track of whether alert has been triggered

int oled_display = 0;
int blynk_publish = 1;

//BME Sensor variables
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
  // Since the buffer is initialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();

  Particle.function("text", displayText);
  Particle.function("clear", displayClear);
  Particle.function("sensor", readSensor);
  Particle.function("battery", getBatteryInfo);

  Particle.variable("temp", temperature);
  Particle.variable("pressure", pressure);
  Particle.variable("seapressure", sealevelpressure);
  Particle.variable("humidity", humidity);
  Particle.variable("altitude", altitude);

  if (!bme.begin(0x76)) {
      displayText("Could not find a valid BME280 sensor, check wiring!");
  } else {
    Serial.println("Found BME280 I2C address: 0x76!");
    displayText("+ Device ready! \n\n(4) commands found:\nsensor, battery, text, clear\n");
    displayClear("");
  }

  // Set up Spark variables (voltage, soc, and alert):
	Particle.variable("voltage", voltage);
	Particle.variable("soc", soc);
	Particle.variable("alert", alert);
	// To read the values from a browser, go to:
	// http://api.particle.io/v1/devices/{DEVICE_ID}/{VARIABLE}?access_token={ACCESS_TOKEN}

	// Set up the MAX17043 LiPo fuel gauge:
	lipo.begin(); // Initialize the MAX17043 LiPo fuel gauge

	// Quick start restarts the MAX17043 in hopes of getting a more accurate
	// guess for the SOC.
	lipo.quickStart();

	// We can set an interrupt to alert when the battery SoC gets too low.
	// We can alert at anywhere between 1% - 32%:
	lipo.setThreshold(20); // Set alert threshold to 20%.

  Blynk.begin(auth);  //blocking call, should be last in setup(), pass 2nd paramer BLYNK_IP for private server
}

void loop() {
  Blynk.run();

  if (oled_display > 0) {
    readSensor("display");
  }
  if (blynk_publish > 0) {
    bool result = Blynk.connected();
    if (!result) {
      Blynk.connect();
      Blynk.syncAll();
    }
    readSensor("blynk");
    getBatteryInfo("blynk");
  }

  delay(REFRESH_INTERVAL);
}

int readSensor(String command) {
  temperature = bme.readTemperature();
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  pressure =  ( bme.readPressure()/100.0F );
  sealevelpressure = bme.seaLevelForAltitude(altitude, pressure);
  humidity = bme.readHumidity();
  if (command == "blynk") {
    Blynk.virtualWrite(V0, String::format("%.02f", temperature));
    Blynk.virtualWrite(V1, String::format("%.02f", humidity));
    Blynk.virtualWrite(V2, String::format("%.02f", pressure));
    Blynk.virtualWrite(V3, String::format("%.02f", sealevelpressure));
    Blynk.virtualWrite(V4, String::format("%.02f", altitude));
    return 2;
  } else if (command == "stop") {
    oled_display = 0;
    blynk_publish = 0;
    return 0;
  } else { //Default display to OLED
    String data = "";
    data.concat("Temperature: "+String::format("%.02f C\n", temperature));
    data.concat("Humidity: "+String::format("%.02f %%\n", humidity));
    data.concat("Pressure: "+String::format("%.02f hPa\n", pressure));
    data.concat("Sealevel: "+String::format("%.02f hPa\n", sealevelpressure));
    data.concat("Altitude: "+String::format("%.02f m\n", altitude));
    displayText(data);
    oled_display = 1;
    return 1;
  }
}

int getBatteryInfo(String command) {
  voltage = lipo.getVoltage();
  soc = lipo.getSOC();
  alert = lipo.getAlert();
  String s_data = String::format("Voltage: %.02f \nSOC:  %.02f \nAlert:  %d\n", voltage, soc, alert);
  if (command == "blynk") {
    Blynk.virtualWrite(V5, String::format("%.02f", voltage));
    Blynk.virtualWrite(V6, String::format("%.02f", soc));
    Blynk.virtualWrite(V7, String::format("%d", alert));
  }  else {
    displayText(s_data);
    displayClear("");
  }
  return 1;
}
int displayClear(String command) {
  if (command == "") {
    Serial.print("Clearing display in ");
    Serial.print(SCREEN_CLEAR/1000);
    Serial.println(" seconds...\n");
    delay(SCREEN_CLEAR);
  } else {
    int _delay = atoi(command);
    if (_delay > 0) {
      delay(_delay);
    }
  }
  display.clearDisplay();
  display.display();
  oled_display = 0;
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
