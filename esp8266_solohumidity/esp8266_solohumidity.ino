/*
  Board: ESP8266
  Sensor : Solo Humidity Sensor
  Author : Pedro Magalhães
  More: https://www.linkedin.com/in/pedromagalhães

                Polytechnic of Leiria

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "CASA_MAG"
#define wifi_password "#CASA-COM#"

#define mqtt_server "10.1.1.4"
#define mqtt_user "teste2"      // if exist user
#define mqtt_password "teste"  // password

//#define temperature_topic "sensor/temperature"  //Topic temperature
#define humidity_topic "sensor/solo_humidity"        //Topic humidity

//Buffer to decode MQTT messages
char message_buff[100];
float UmidadePercentualLida;
int UmidadePercentualTruncada;
char FieldUmidade[11];

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

    // nothing to send
    UmidadePercentualLida = FazLeituraUmidade();
    UmidadePercentualTruncada = (int)UmidadePercentualLida; //trunca umidade como número inteiro
    Serial.println(UmidadePercentualTruncada);

    if ( debug ) {

      Serial.print(" | Humidity : ");
      Serial.println(UmidadePercentualTruncada);
    }

    client.publish(humidity_topic, String(UmidadePercentualTruncada).c_str(), true);      // Publish  humidity
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
float FazLeituraUmidade(void)
{
  int ValorADC;
  float UmidadePercentual;

  ValorADC = analogRead(0);   //978 -> 3,3V
  Serial.print("[ ADC - testar valor de entrada] ");
  Serial.println(ValorADC);

  //Quanto maior o numero lido do ADC, menor a umidade.
  //Sendo assim, calcula-se a porcentagem de umidade por:
  //
  //   Valor lido                 Umidade percentual
  //      _    0                           _ 100
  //      |                                |
  //      |                                |
  //      -   ValorADC                     - UmidadePercentual
  //      |                                |
  //      |                                |
  //     _|_  978                         _|_ 0
  //
  //   (UmidadePercentual-0) / (100-0)  =  (ValorADC - 978) / (-978)
  //      Logo:
  //      UmidadePercentual = 100 * ((978-ValorADC) / 978)

  UmidadePercentual = 100 * ((978 - (float)ValorADC) / 978);
  Serial.print("[Percentual Humidity] ");
  Serial.print(UmidadePercentual);
  Serial.println("%");

  return UmidadePercentual;
}


