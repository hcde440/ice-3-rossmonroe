/*ICE #3.1
 * Program Description
  This app takes data from a DHT22 and MPL115 then prints it to the serial, an OLED display, and then passes the data it to adafruit io.
*/
#include "config.h" //wifi and adafruit configuration

#include <Wire.h> //i2c inlcude

//ML115A2 includes
#include <Adafruit_MPL115A2.h>
Adafruit_MPL115A2 mpl115a2;

//DHT includes
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHT_PIN 12 // pin connected to DH22 data line
DHT_Unified dht(DHT_PIN, DHT22); // create DHT22 instance

//OLED Display includes
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//OLED display settings
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// set up feeds for adafruit io
AdafruitIO_Feed *temperature = io.feed("temperature");
AdafruitIO_Feed *humidity = io.feed("humidity");
AdafruitIO_Feed *MPLTemperature = io.feed("MPLTemperature");
AdafruitIO_Feed *MPLPressure = io.feed("MPLPressure");

void setup() {
  Serial.begin(115200); // start the serial connection
  while (! Serial); // wait for serial monitor to open
  adafruitConnect(); // create connection to adafruit io
  Serial.println("Before Display");
  startDisplay(); // initialize display
  Serial.println("After Display");
  mpl115a2.begin(); // start MPL sensor
}

void loop() {
  io.run(); // continually checkin with adafruit io
  Serial.println("Before Pressure");
  getPressure(); // get the data from the MPL sensor, print to serial, print to display, and post to adafruit io.
  Serial.println("Before Humi");
  getHumiTemp();  // get the data from the DHT sensor, print to serial, print to display, and post to adafruit io. 
  delay(1000); // short delay to avoid going over api limit.
}

void adafruitConnect(){
  Serial.print("Connecting to Adafruit IO"); // connect to io.adafruit.com
  io.connect();
  while (io.status() < AIO_CONNECTED) { // wait for a connection
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println(io.statusText()); // we are connected
}

void getPressure(){
  Serial.println("");
  Serial.println("--------- MPL115A2 ---------"); //titles section the sensor name with a space above to make it clear where this data is coming from.
  float pressureKPA = 0, temperatureC = 0; //creates float variables for pressure and temperature
  pressureKPA = mpl115a2.getPressure(); //assigns the pressure data from the MPL sensor to the float variable
  temperatureC = mpl115a2.getTemperature(); //assigns the temperature data from the MPL sensor to the float variable
  Serial.print("Pressure: "); Serial.print(pressureKPA); Serial.println(" kPa"); // rints pressure data to serial
  Serial.print("Temp: "); Serial.print(temperatureC); Serial.println(" C"); //prints temperature data to serial
  MPLTemperature->save(temperatureC); //saves temperature data to adafruit io feed.
  MPLPressure->save(pressureKPA); //saves pressure data to adafruit io feed.
  displayPressTemp(temperatureC, pressureKPA); //sends temperature and pressure data to be printed to the OLED display.
}

void getHumiTemp(){ 
  sensors_event_t event; // an event listener to listen for sensor data from the DHT.
  dht.temperature().getEvent(&event); // calls the temperature data from the event listener
  dht.humidity().getEvent(&event); // calls the humidity data from the event listener
  float celsius = event.temperature; //assigns event temperature to float variable.
  float fahrenheit = (celsius * 1.8) + 32; // converts celsius temperature to fahrenheit.
  float humi = event.relative_humidity; // assigns humidity event data to a float variable.
  Serial.println("");
  Serial.println("--------- DHT22 ---------"); // titles section the sensor name with a space above to make it clear where this data is coming from.
  Serial.print("celsius: "); Serial.print(celsius); Serial.println("C"); // print temperature in celsius to serial.
  Serial.print("fahrenheit: "); Serial.print(fahrenheit); Serial.println("F"); // print temperature in fahrenheit to serial.
  Serial.print("humidity: "); Serial.print(humi); Serial.println("%"); // print humidity data to serial.
  temperature->save(fahrenheit); // save fahrenheit (or celsius) to Adafruit IO
  humidity->save(humi); // save humidity to Adafruit IO
  displayTempHumi(fahrenheit, humi); // send temperature and humidity to be printed on the OLED display.
}

void setDisplay() { // this function clears the display, sets the text size and color, then the start point of the text.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
}

void startDisplay() { // The initiallizes the display at the start of the program.
  displaySetup();
  setDisplay();
  display.println(F("Hello!"));
  display.display();
  delay(1000);
}

void displaySetup(){ //Initializes OLED display.
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.display();
}

void displayTempHumi(float fahrenheit, float humi) { //passes in temperature and humidity.
  setDisplay(); //sets initial display information and start point.
  display.print(F("Temperature: ")); display.print(fahrenheit, 1); display.println(" F"); //structure of first line
  display.display(); //assigns the printed data to the display
  display.setCursor(0, 20); //create new start point for second line.
  display.print(F("Humidity: ")); display.print(humi, 1); display.println(" %"); // structure of information for second line.
  display.display(); //prints to display
  delay(3000); //delay to keep information on screen long enough to read.
}
void displayPressTemp(float temperatureC, float pressureKPA) { //passes in termperature and pressure.
  setDisplay(); //set initial display values.
  display.print(F("Pressure: ")); display.print(pressureKPA, 1); display.println(" kPa"); //structure for first line to be displayed.
  display.display(); // posts to diplsay.
  display.setCursor(0, 20); //moves to second line
  display.print(F("Temperature: ")); display.print(temperatureC, 1); display.println(" C"); //second line of information
  display.display(); //posts line to display.
  delay(3000); //delay so information can be read.
}
