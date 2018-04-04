#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "SPI.h"
#include "MFRC522.h"
#define SS_PIN 15
#define RST_PIN 5
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

const char* ssid ="jay19";
const char* password ="12345678";
const char* mqtt_server = "broker.mqtt-dashboard.com";


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[75];
int value = 0;
char temp[50];

void setup_wifi() 
{ Serial.println("Connecting to ");
 
  Serial.println(ssid);
  WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)  {
  delay(500);
  Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } 
  else {
       digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
      }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("Marutopic", "Mayank");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  Serial.print("Enter the SSID: ");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
}

void loop() {
 if (!client.connected()) 
{  reconnect();
 }
 client.loop();
 if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
   return;

 // Serial.print(F("PICC type: "));
 MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
 Serial.println(rfid.PICC_GetTypeName(piccType));

 // Check is the PICC of Classic MIFARE type
 if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
   piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
   piccType != MFRC522::PICC_TYPE_MIFARE_4K)
   {
   Serial.println(F("Your tag is not of type MIFARE Classic."));
   return;
  }
 

 String strID = "";
 for (byte i = 0; i < 4; i++) {
   strID +=  (rfid.uid.uidByte[i] < 0x10 ? "0" : "") + String(rfid.uid.uidByte[i], HEX) + (i!=3 ? ":" : "");
 }
 strID.toUpperCase();
Serial.print("Tap card key: ");
 Serial.println(strID);

 long now = millis();
  if (now - lastMsg > 200) 
  {
    lastMsg = now;
    ++value;
    snprintf (msg, 75,&strID[0]);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("Marutopic", msg);
  }
     
 rfid.PICC_HaltA();
 rfid.PCD_StopCrypto1();

 
}





