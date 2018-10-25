/*
  Board: ESP8266
  Sensor : Water Temp Sensor
  Author : Pedro Magalhães
  More: https://www.linkedin.com/in/pedromagalhães

                Polytechnic of Leiria

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define wifi_ssid "*****"
#define wifi_password "*****"

#define mqtt_server "10.1.1.4"
#define mqtt_user "teste"      // if exist user
#define mqtt_password "teste"  // password


#define ONE_WIRE_BUS    2      // DS18B20 pin

//#define temperature_topic "sensor/temperature"  //Topic temperature
#define watertemp_topic "sensor/water_temperature"        //Topic humidity

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);



//Buffer to decode MQTT messages
char message_buff[100];

char temperatureString[6];
long lastMsg = 0;
long lastRecu = 0;
bool debug = false;  //Display log message if  = True

// Create abjects
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);

  setup_wifi();           //Connect to Wifi network
  client.setServer(mqtt_server, 1883);    // Configure MQTT connection
  client.setCallback(callback);// callback function ( MQTT message )
  // setup OneWire bus
  DS18B20.begin();

}


void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi OK ");
  Serial.print("=> ESP8266 IP address: ");

}

//Reconnexion
void reconnect() {

  while (!client.connected()) {
    Serial.print("Connecting to MQTT broker ...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("OK");
    } else {
      Serial.print("KO, error : ");
      Serial.print(client.state());
      Serial.println(" Wait 5 secondes before retry");

      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  // Send a message every 4/minute, change 20 to 60 for 1 minute
  if (now - lastMsg > 1000 * 5) {
    lastMsg = now;


    float temperature = getTemperature();
    // convert temperature to a string with two digits before the comma and 2 digits for precision
    dtostrf(temperature, 2, 2, temperatureString);

    // send temperature to the serial console
    Serial.println(temperatureString);

    if ( debug ) {

      Serial.print(" | Temp : ");
      Serial.println(temperature);
    }

    client.publish(watertemp_topic, String(temperature).c_str(), true);      // Publish  humidity
  }
  if (now - lastRecu > 100 ) {
    lastRecu = now;
    client.subscribe("homeassistant/switch1");
  }
}

// MQTT callback function
void callback(char* topic, byte* payload, unsigned int length) {

  int i = 0;
  if ( debug ) {
    Serial.println("Message recu =>  topic: " + String(topic));
    Serial.print(" | longueur: " + String(length, DEC));
  }
  // create character buffer with ending null terminator (string)
  for (i = 0; i < length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';

  String msgString = String(message_buff);
  if ( debug ) {
    Serial.println("Payload: " + msgString);
  }

  
}

float getTemperature() {

  float temp;

  DS18B20.requestTemperatures();
  temp = DS18B20.getTempCByIndex(0);


  return temp;
}
