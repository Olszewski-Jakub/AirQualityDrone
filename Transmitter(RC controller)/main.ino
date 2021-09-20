#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <esp_wifi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <WiFi.h>

#include "display.h"
#include "led_functions.h"
/*    VARIBLES    */

//NRF-24L01
#define CE 12
#define CSN 14
#define *        26
#define MOSI 27
#define MISO 25

///YOISTICK 1
#define JOY1_X 0
#define JOY1_Y 0
#define JOY1_SW 0

///YOISTICK 2
#define JOY2_X 0
#define JOY2_Y 0
#define JOY2_SW 0

///Buttons
#define jB1 0 // Joystick button 1
#define jB2 0 // Joystick button 2
#define t1 0  // Toggle switch 1
#define t2 0  // Toggle switch 1
#define b1 0  // Button 1
#define b2 0  // Button 2
#define b3 0  // Button 3
#define b4 0  // Button 4

///Display
#define OLED_SDA 0
#define OLED_SCL 0

///LED
#define DRONE_CONNECTION_RGB_RED 0
#define DRONE_CONNECTION_RGB_GREEN 0
#define DRONE_CONNECTION_RGB_BLUE 0

#define PHONE_CONNECTION_RGB_RED 0
#define PHONE_CONNECTION_RGB_GREEN 0
#define PHONE_CONNECTION_RGB_BLUE 0

//AP SETTINGS
const char *ssid = "AirQualityDrone";
const char *password = "123456789";

int LED_DRONE_CONNECTION_STATUS[3] = {
    DRONE_CONNECTION_RGB_BLUE,
    DRONE_CONNECTION_RGB_GREEN,
    DRONE_CONNECTION_RGB_BLUE};

int LED_PHONE_CONNECTION_STATUS[3] = {
    PHONE_CONNECTION_RGB_RED,
    PHONE_CONNECTION_RGB_GREEN,
    PHONE_CONNECTION_RGB_BLUE};

unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

RF24 radio(CE, CSN, SCK, MISO, MOSI);
const byte address[6] = "00001"; // Address
// Max size of this struct is 32 bytes - NRF24L01 buffer limit
struct Data_Package
{
    byte j1PotX;
    byte j1PotY;
    byte j1Button;
    byte j2PotX;
    byte j2PotY;
    byte j2Button;
    byte pot1;
    byte pot2;
    byte tSwitch1;
    byte tSwitch2;
    byte button1;
    byte button2;
    byte button3;
    byte button4;
};

struct Sensor_Package
{
    float temperature;
    float humidity;
    float preasure;
    float PM10;
    float PM25;
    float latitude;
    float longtitude;
    float battery;
    float speed;
};

Data_Package data; //Create a variable with the above structure
Sensor_Package sensor;

///WiFi
WiFiServer server(80);
WiFiUDP Udp;
unsigned int localUdpPort = 4210;

void setup()
{
    Serial.begin(9600);

    // Define the radio communication
    radio.begin();
    radio.openWritingPipe(address);
    radio.setAutoAck(false);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_LOW);

    // Activate the Arduino internal pull-up resistors
    pinMode(jB1, INPUT_PULLUP);
    pinMode(jB2, INPUT_PULLUP);
    pinMode(t1, INPUT_PULLUP);
    pinMode(t2, INPUT_PULLUP);
    pinMode(b1, INPUT_PULLUP);
    pinMode(b2, INPUT_PULLUP);
    pinMode(b3, INPUT_PULLUP);
    pinMode(b4, INPUT_PULLUP);

    //Set RGB diods parapeters
    setOutput(LED_DRONE_CONNECTION_STATUS);
    setOutput(LED_PHONE_CONNECTION_STATUS);

    setRgbColor("red", LED_DRONE_CONNECTION_STATUS);
    setRgbColor("red", LED_PHONE_CONNECTION_STATUS);

    // Set initial default values
    data.j1PotX = 127; // Values from 0 to 255. When Joystick is in resting position, the value is in the middle, or 127. We actually map the pot value from 0 to 1023 to 0 to 255 because that's one BYTE value
    data.j1PotY = 127;
    data.j2PotX = 127;
    data.j2PotY = 127;
    data.j1Button = 1;
    data.j2Button = 1;
    data.pot1 = 1;
    data.pot2 = 1;
    data.tSwitch1 = 1;
    data.tSwitch2 = 1;
    data.button1 = 1;
    data.button2 = 1;
    data.button3 = 1;
    data.button4 = 1;

    sensor.temperature = 0.0;
    sensor.humidity = 0.0;
    sensor.preasure = 0.0;
    sensor.PM10 = 0.0;
    sensor.PM25 = 0.0;
    sensor.latitude = 0.0;
    sensor.longtitude = 0.0;
    sensor.battery = 0.0;
    sensor.speed = 0.0;

    initDisplay();
    clearScreen();
}

void loop()
{
    currentTime = millis();
    if (currentTime - lastReceiveTime > 1000)
    {                // If current time is more then 1 second since we have recived the last data, that means we have lost connection
        resetData(); // If connection is lost, reset the data. It prevents unwanted behavior, for example if a drone jas a throttle up, if we lose connection it can keep flying away if we dont reset the function
    }

    // Read all analog inputs and map them to one Byte value
    data.j1PotX = map(analogRead(JOY1_X), 0, 1023, 0, 255); // Convert the analog read value from 0 to 1023 into a BYTE value from 0 to 255
    data.j1PotY = map(analogRead(JOY1_Y), 0, 1023, 0, 255);
    data.j2PotX = map(analogRead(JOY2_X), 0, 1023, 0, 255);
    data.j2PotY = map(analogRead(JOY2_Y), 0, 1023, 0, 255);
    data.pot1 = map(analogRead(JOY1_SW), 0, 1023, 0, 255);
    data.pot2 = map(analogRead(JOY2_SW), 0, 1023, 0, 255);
    // Read all digital inputs
    data.j1Button = digitalRead(jB1);
    data.j2Button = digitalRead(jB2);
    data.tSwitch2 = digitalRead(t2);
    data.button1 = digitalRead(b1);
    data.button2 = digitalRead(b2);
    data.button3 = digitalRead(b3);
    data.button4 = digitalRead(b4);

    // Send the whole data from the structure to the receiver
    radio.write(&data, sizeof(Data_Package));

    if (radio.available())
    {
        setRgbColor("green", LED_DRONE_CONNECTION_STATUS);
        radio.read(&sensor, sizeof(Sensor_Package));
        lastReceiveTime = millis();
    }
}

void getDevicesInfo(String message)
{
    wifi_sta_list_t wifi_sta_list;
    tcpip_adapter_sta_list_t adapter_sta_list;

    memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
    memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));

    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);

    for (int i = 0; i < adapter_sta_list.num; i++)
    {
        tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
        String ip = ip4addr_ntoa(&(station.ip));
        sendViaUDP(ip, message);
    }
}

void sendViaUDP(String ip, String Message)
{
  setRgbColor("green", LED_PHONE_CONNECTION_STATUS);
  Serial.println(ip + " " + Message);
  IPAddress ipSend(getValue(ip, '.', 0).toInt(), getValue(ip, '.', 1).toInt(), getValue(ip, '.', 2).toInt(), getValue(ip, '.', 3).toInt());
  //IPAddress ipSend(192,168,4,1);

  Udp.beginPacket(ipSend, localUdpPort);
  Udp.print(Message);
  Udp.endPacket();
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length();

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void resetData()
{
    // Reset the values when there is no radio connection - Set initial default values
    data.j1PotX = 127;
    data.j1PotY = 127;
    data.j2PotX = 127;
    data.j2PotY = 127;
    data.j1Button = 1;
    data.j2Button = 1;
    data.pot1 = 1;
    data.pot2 = 1;
    data.tSwitch1 = 1;
    data.tSwitch2 = 1;
    data.button1 = 1;
    data.button2 = 1;
    data.button3 = 1;
    data.button4 = 1;
}


