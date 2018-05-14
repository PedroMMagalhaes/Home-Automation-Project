/*
  Board: ESP8266
  Sensor : Pir Sensor
  Author : Pedro Magalhães
  More: https://www.linkedin.com/in/pedromagalhães

                Polytechnic of Leiria

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "XXXXX"
#define wifi_password "XXXXXX"

#define mqtt_server "XXXXX"
#define mqtt_user "XXXXX"      // if exist user
#define mqtt_password "XXXX"  // password

#define movement_topic "sensor/movement" 

int pirPin = D7;
int val;


//Buffer to decode MQTT messages
char message_buff[100];

// lowest and highest sensor readings:
const int sensorMin = 0;     // sensor minimum
const int sensorMax = 1024;  // sensor maximum

String mov;

long lastMsg = 0;
long lastRecu = 0;
bool debug = false;  //Display log message if  = True

// Create abjects
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  pinMode(D2, OUTPUT);    //Pin 2 for LED
  setup_wifi();           //Connect to Wifi network
  client.setServer(mqtt_server, 1883);    // Configure MQTT connection
  client.setCallback(callback);           // callback function ( MQTT message )

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

//Reconnect
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

   
    if ( debug ) {
      Serial.print(" | Mov status : ");
      Serial.println(mov);
    }

     mov = movement();
    Serial.println(mov);
    
    client.publish(movement_topic, String(mov).c_str(), true);      // Publish Movement status (its not necessary conversion to string , but in case of spaces is required)
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

  if ( msgString == "ON" ) {
    digitalWrite(D2, HIGH);
  } else {
    digitalWrite(D2, LOW);
  }
}

String movement(void)
{

  val = digitalRead(pirPin);
  String movement;
  //low = no motion, high = motion

  if (val == LOW)
  {
    movement = "No motion";
  }
  else if(val == HIGH)
  {
    movement = "Motion detected";
  }

  return movement;
}

