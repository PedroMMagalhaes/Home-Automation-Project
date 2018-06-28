/*
  Board: ESP8266
  Sensor : DHT22 , WaterTemp, RainDropSensor
  Author : Pedro Magalhães
  More: https://www.linkedin.com/in/pedromagalhães
        https://github.com/PedroMMagalhaes

                Polytechnic of Leiria

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h" // Library for DHT sensors
#include <OneWire.h>
#include <DallasTemperature.h>

#define wifi_ssid "###" //wifi SSID 
#define wifi_password "#####" // WIFi password

#define mqtt_server "IP_MQTT_SERVER"
#define mqtt_user "#####"     // if exist user
#define mqtt_password "#####" // password

#define temperature_topic "sensor/temperature"     //Topic temperature
#define humidity_topic "sensor/humidity"           //Topic humidity
#define humidity_topic_solo "sensor/solo_humidity" //Topic humidity
#define watertemp_topic "sensor/water_temperature" //Topic watertemp
#define raindrops_topic "sensor/raindrops"         //Topic raindrops

/* ±±±±± Var's ±±±±± */
char temperatureString[6];

// lowest and highest sensor readings:
const int sensorMin = 0;    // sensor minimum
const int sensorMax = 1024; // sensor maximum
String rain;

//Buffer to decode MQTT messages
char message_buff[100];

float HumidityPercentualRead;
int HumidityFinal;
char FieldHumidity[11];

long lastMsg = 0;
long lastRecu = 0;
bool debug = false; //Display log message if  = True

int teste = 0; //control the sleep mode

#define DHTPIN 5       // DHT Pin
#define ONE_WIRE_BUS 2 // DS18B20 pin

//Water Sensor init *
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

// Type of sensor pin's
//#define DHTTYPE DHT11       // DHT 11
#define DHTTYPE DHT22 // DHT 22  (AM2302)

// Create abjects
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  Serial.begin(9600);
  setup_wifi();                        //Connect to Wifi network
  client.setServer(mqtt_server, 1883); // Configure MQTT connection
  client.setCallback(callback);        // callback function ( MQTT message )

  dht.begin();
  // setup OneWire bus
  DS18B20.begin();
}

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi OK ");
  Serial.print("=> ESP8266 IP address: ");
}

//Reconnect
void reconnect()
{

  while (!client.connected())
  {
    Serial.print("Connecting to MQTT broker ...");
    if (client.connect("ESP8266Client2", mqtt_user, mqtt_password))
    {
      Serial.println("OK");
    }
    else
    {
      Serial.print("KO, error : ");
      Serial.print(client.state());
      Serial.println(" Wait 5 secondes before retry");

      delay(5000);
    }
  }
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  /* ## Methods Sensors Publish and Print ## */

  if (teste == 0 || teste < 5)
  {
    rainSensor();
    dhtSensor();
    waterTemperatureSensor();
    teste++;
    delay(1000);
  }
  else
  {
    teste = 0;
    sleep(); //30sec
  }

  //Deep Sleep
}

/* ####  ####  ####  #### #### #### #### #### ####  */

void sleep()
{
  Serial.println("Deep Sleep Mode Activated ...");
  ESP.deepSleep(3 * 1000000); //Sleep for 30 ~ seconds)
}
// MQTT callback function
void callback(char *topic, byte *payload, unsigned int length)
{

  int i = 0;
  if (debug)
  {
    Serial.println("Message rec =>  topic: " + String(topic));
    Serial.print(" | test: " + String(length, DEC));
  }
  // create character buffer with ending null terminator (string)
  for (i = 0; i < length; i++)
  {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';

  String msgString = String(message_buff);
  if (debug)
  {
    Serial.println("Payload: " + msgString);
  }
}

void dhtSensor()
{

  //long now = millis();
  // Send a message every 4/minute, change 20 to 60 for 1 minute
  //if (now - lastMsg > 1000 * 5) {
  //lastMsg = now;
  // Read humidity
  float h = dht.readHumidity();
  // Read temperature in Celcius
  float t = dht.readTemperature();
  Serial.println("***** Debug *****");
  Serial.println(h);
  Serial.println(t);

  // nothing to send
  if (isnan(t) || isnan(h))
  {
    Serial.println("KO, Please check DHT sensor !");
    return;
  }

  if (debug)
  {
    Serial.print("Temperature : ");
    Serial.print(t);
    Serial.print(" | Humidity : ");
    Serial.println(h);
  }
  client.publish(temperature_topic, String(t).c_str(), true); // Publish temperature on temperature_topic
  client.publish(humidity_topic, String(h).c_str(), true);    // and humidity
}

void soloHumiditySensor()
{

  // nothing to send
  HumidityPercentualRead = DoReadHumidity();
  HumidityFinal = (int)HumidityPercentualRead; //
  Serial.println(HumidityFinal);

  if (debug)
  {

    Serial.print(" | Humidity : ");
    Serial.println(HumidityFinal);
  }

  client.publish(humidity_topic_solo, String(HumidityFinal).c_str(), true); // Publish  humidity
}

float DoReadHumidity(void)
{
  int ValueADC;
  float Humidity;

  ValueADC = analogRead(0); //978 -> 3,3V
  Serial.print("[ ADC - testar valor de entrada] ");
  Serial.println(ValueADC);

  
  //   (Humidity-0) / (100-0)  =  (ValueADC - 978) / (-978)
  //      
  //      Humidity = 100 * ((978-ValueADC) / 978)

  Humidity = 100 * ((978 - (float)ValueADC) / 978);
  Serial.print("[Percentual Humidity] ");
  Serial.print(Humidity);
  Serial.println("%");

  return Humidity;
}

void waterTemperatureSensor()
{

  float temperature = getWaterTemperature();
  // convert temperature to a string with two digits before the comma and 2 digits for precision
  dtostrf(temperature, 2, 2, temperatureString);

  // send temperature to the serial console
  Serial.println(temperatureString);

  if (debug)
  {

    Serial.print(" | Temp : ");
    Serial.println(temperature);
  }

  client.publish(watertemp_topic, String(temperature).c_str(), true); // Publish  humidity
}

float getWaterTemperature()
{

  float temp;

  DS18B20.requestTemperatures();
  temp = DS18B20.getTempCByIndex(0);

  return temp;
}

String raining(void)
{

  // read the sensor on analog A0:
  int sensorReading = analogRead(A0);
  // map the sensor range (four options):
  // ex: 'long int map(long int, long int, long int, long int, long int)'
  int range = map(sensorReading, sensorMin, sensorMax, 0, 3);
  String rainstatus = "Not Raining";

  // range value:
  switch (range)
  {
  case 0: // Sensor getting wet
    rainstatus = "Flood";
    break;
  case 1: // Sensor getting wet
    rainstatus = "Rain Warning";
    break;
  case 2: // Sensor dry - To shut this up delete the " Serial.println("Not Raining"); " below.
    rainstatus = "Not Raining";
    break;
  }

  return rainstatus;
}

void rainSensor()
{

  // nothing to send
  rain = raining();

  Serial.println(rain);

  if (debug)
  {

    Serial.print(" | Rain status : ");
    Serial.println(rain);
  }

  client.publish(raindrops_topic, String(rain).c_str(), true); // Publish rainstatus (its not necessary conversion to string , but in case of spaces is required)
}
