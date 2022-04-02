#include <Arduino.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <Ethernet.h>
#include <EthernetClient.h>

#include <Wire.h>

char str[7];
char code[7]="2345";
/************************* Ethernet Client Setup *****************************/
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(172, 17, 0, 2);
IPAddress myDns(8, 8, 8, 8);


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME  "yourusername"       //Replace this with your username
#define AIO_KEY       "youradafruitiokey"  //Replace this with your Adafruit IO key

/************ Global State (you don't need to change this!) ******************/

//Set up the ethernet client
EthernetClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }


/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Subscribe key = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/lock");

/*************************** Sketch Code ************************************/

void setup() {
  Wire.begin();
  Serial.begin(115200);
  pinMode(7, OUTPUT);

  Serial.println(F("Smart lock"));

  // Initialise the Client
  Serial.print(F("\nInit the Client..."));
  Ethernet.begin(mac, ip, myDns);
  delay(1000); //give the ethernet a second to initialize

  mqtt.subscribe(&key);
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  int i=0;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &key) {
       //Serial.print(F("Got: "));
       char *received = (char *) key.lastread;
       str[i] = *received; 
       Serial.println(received);
//       Serial.println(key.lastread[0]);
       if(str[3]!=0){
        i=0;
        Serial.print("Received Code: ");
        for(int i=0;i<4;i++){
          Serial.print(str[i]);
        }
        Serial.println("");
        if(!strcmp(str,code)){
          Serial.println("Code Match");
          digitalWrite(7, HIGH);
        }
        else{
          Serial.println("Code Unmatch");
          digitalWrite(7, LOW);
        }
        for(int i=0;i<4;i++){
          str[i]=0;
        }
      }
      Serial.println("");
      i++;
    }
  }

  // ping the server to keep the mqtt connection alive
  if (! mqtt.ping()) {
    mqtt.disconnect();
  }
  delay(7000);
}
