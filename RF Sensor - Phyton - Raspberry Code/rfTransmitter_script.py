#coding=utf-8

#  Board: Raspberry PI
#  Sensor : RF Emissor and Transmitter - MQTT && Arduino 
#  Author : Pedro Magalhães
#  More: https://www.linkedin.com/in/pedromagalhães
#        https://github.com/PedroMMagalhaes
  
#                Polytechnic of Leiria
   

import time
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import serial

Broker = "10.1.1.102"
sub_topic = "sensor/rfreceiver"    # receive messages on this topic
pub_topic = "sensor/rftransmitter"       # send messages to this topic


############### Action Arduino ##################


ser = serial.Serial('/dev/ttyACM0', 9600)
time.sleep(3)




############### MQTT section ##################

# when connecting to mqtt do this;

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe(sub_topic)

# when receiving a mqtt message do this;
def on_message(client, userdata, msg):
    message = str(msg.payload)
    print(msg.topic+" "+message)
    
    if message == "a" :
        ser.write('a')

    #print(message)

def on_publish(mosq, obj, mid):
    print("mid: " + str(mid))


client = mqtt.Client()

auth = {
  'username':"teste1",
  'password':"teste"
}


client.on_connect = on_connect
client.on_message = on_message
client.connect(Broker, 1883, 60)
client.loop_start()

while True:
    
    time.sleep(1*60)