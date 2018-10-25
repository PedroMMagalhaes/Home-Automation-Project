/*
  Board: ESP8266
  Sensor : Rele Shield + Sensor Solo Humidity + Movement 
  Author : Pedro Magalhães
  More: https://www.linkedin.com/in/pedromagalhães

                Polytechnic of Leiria

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "ESS"
#define wifi_password "Teste123"

#define mqtt_server "10.1.1.102"
#define mqtt_user "esp"        // if exist user
#define mqtt_password "esp123" // password

#define rele_topic1 "sensor/switch1" //Topic rele ch1
#define pinCh1 D2

#define movement_topic "sensor/movement"
#define humidity_topic "sensor/solo_humidity" //Topic humidity
int pirPin = D1;
int val;
String mov;

float Humidity;
int PercentualHumidityFinal;
char FielHumidity[11];

//Buffer to decode MQTT messages
char message_buff[100];
long lastMsg = 0;
long lastRecu = 0;
bool debug = false; //Display log message if  = True

// Create abjects
WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  Serial.begin(9600);
  setup_wifi();                        //Connect to Wifi network
  client.setServer(mqtt_server, 1883); // Configure MQTT connection
  client.setCallback(callback);        // callback function ( MQTT message )
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

  //initialize the switch as an output and set to LOW (off)
  pinMode(pinCh1, OUTPUT); // Relay Switch 1
  digitalWrite(pinCh1, LOW);
}

//Reconnexion
void reconnect()
{

  while (!client.connected())
  {
    Serial.print("Connecting to MQTT broker ...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password))
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

  long now = millis();
  // Send a message every 4/minute, change 20 to 60 for 1 minute
  // if (now - lastMsg > 1000 * 5) {
  // lastMsg = now;

  //}
  if (now - lastRecu > 100)
  {
    lastRecu = now;
    client.subscribe(rele_topic1); //sub topic rele1
  }

  printSolo();
  printMov();
}

// MQTT callback function
void callback(char *topic, byte *payload, unsigned int length)
{

  int i = 0;
  if (debug)
  {
    Serial.println("Message recu =>  topic: " + String(topic));
    Serial.print(" | longueur: " + String(length, DEC));
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

  if (msgString == "ON")
  {
    digitalWrite(pinCh1, HIGH);
    Serial.println("ON Rele1");
  }
  else
  {
    digitalWrite(pinCh1, LOW);
    Serial.println("OFF Rele1");
  }
}
void printMov()
{
  mov = movement();

  Serial.println(mov);

  client.publish(movement_topic, String(mov).c_str(), true);
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
  else if (val == HIGH)
  {
    movement = "Motion detected";
  }

  return movement;
}

void printSolo()
{

  Humidity = FazLeituraHumidade();
  PercentualHumidityFinal = (int)Humidity; //trunca umidade como número inteiro
  Serial.println(PercentualHumidityFinal);
  client.publish(humidity_topic, String(PercentualHumidityFinal).c_str(), true); // Publish  humidity
}

float FazLeituraHumidade(void)
{
  int ValorADC;
  float PercentualHumidity;

  ValorADC = analogRead(0); //978 -> 3,3V
  Serial.print("[ ADC - testar valor de entrada] ");
  Serial.println(ValorADC);

  //Quanto maior o numero lido do ADC, menor a umidade.
  //Sendo assim, calcula-se a porcentagem de umidade por:
  //
  //   Valor lido                 Umidade percentual
  //      _    0                           _ 100
  //      |                                |
  //      |                                |
  //      -   ValorADC                     - PercentualHumidity
  //      |                                |
  //      |                                |
  //     _|_  978                         _|_ 0
  //
  //   (PercentualHumidity-0) / (100-0)  =  (ValorADC - 978) / (-978)
  //      Logo:
  //      PercentualHumidity = 100 * ((978-ValorADC) / 978)

  PercentualHumidity = 100 * ((978 - (float)ValorADC) / 978);
  Serial.print("[Percentual Humidity] ");
  Serial.print(PercentualHumidity);
  Serial.println("%");

  return PercentualHumidity;
}
