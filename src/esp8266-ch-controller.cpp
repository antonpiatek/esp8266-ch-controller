#include <Arduino.h>
#include "config.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#ifndef WIFI_SSID
  #error "WIFI_SSID must be defined in config"
#endif

#ifndef WIFI_PASS
  #error "WIFI_PASS must be defined in config"
#endif

#ifndef BROKER
  #error "BROKER must be defined in config"
#endif

#ifndef BROKER_PASS
  #error "BROKER_PASS must be defined in config"
#endif

#ifndef RELAY_1_PIN
  #error "RELAY_1_PIN must be defined in config"
#endif

#ifndef RELAY_1_TOPIC
  #error "RELAY_1_TOPIC must be defined in config"
#endif

#ifndef RELAY_2_PIN
  #error "RELAY_2_PIN must be defined in config"
#endif

#ifndef RELAY_2_TOPIC
  #error "RELAY_2_TOPIC must be defined in config"
#endif

#ifndef MAX_ON_MIN
  #error "MAX_ON_MIN must be defined in config"
#endif

// Main program
WiFiClient espClient;
PubSubClient client(espClient);
String clientId;

unsigned long timeR1TurnedOn;
unsigned long timeR2TurnedOn;
bool r1State = false;
bool r2State = false;

String topic1_pub;
String topic2_pub;
String topic1_sub;
String topic2_sub;
String willTopic;

void setRelay1(boolean state);
void setRelay2(boolean state);
void checkComms();
void logRelay(int id, bool state);
void _setRelayN(int pin, bool state);
void _setLedN(int pin, bool state);
bool setTarget(char state, char relay);
void log(String l);
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  setRelay1(false);
  setRelay2(false); 

  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);

  Serial.begin(9600);
  
  log("init...");
  pinMode(LED_BUILTIN, OUTPUT);

  //set wifi
  Serial.println();
  Serial.print("WIFI configured to ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  topic1_pub = String(TOPIC_BASE)+"/"+String(RELAY_1_TOPIC)+"/status";
  topic2_pub = String(TOPIC_BASE)+"/"+String(RELAY_2_TOPIC)+"/status";
  topic1_sub = String(TOPIC_BASE)+"/"+String(RELAY_1_TOPIC);
  topic2_sub = String(TOPIC_BASE)+"/"+String(RELAY_2_TOPIC);

  //TODO
  willTopic = String(TOPIC_BASE)+"/status";
  Serial.println("will TOPIC: "+willTopic);
  Serial.println("TOPIC1: "+topic1_sub);
  Serial.println("TOPIC1: "+topic1_pub);
  Serial.println("TOPIC2: "+topic2_sub);
  Serial.println("TOPIC2: "+topic2_pub);

  //set mqtt
  clientId = WiFi.macAddress();
  client.setServer(BROKER, 1883);
  client.setCallback(callback);

  checkComms();

  log("ready");
  log("# max timeout "+String(MAX_ON_MIN)+" min");

  client.loop();
  setRelay1(false);
  setRelay2(false); 
}


void checkComms(){
    if(WiFi.status() != WL_CONNECTED){
        while (WiFi.status() != WL_CONNECTED) {
            Serial.print("waiting for wifi, rc= ");
            Serial.println(WiFi.status());
            digitalWrite(LED_BUILTIN, HIGH); 
            delay(500);
            //TODO while connecting flash led
            Serial.print(".");
            digitalWrite(LED_BUILTIN, LOW); 
            delay(500);
        }
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
    if(!client.connected()){
        Serial.print("MQTT connecting...");
        while (!client.connected()) {
            digitalWrite(LED_BUILTIN, HIGH); 
            Serial.print(".");
            if (client.connect(clientId.c_str(), "use-token-auth", BROKER_PASS, willTopic.c_str(), 0, 1, "0")) {
                Serial.println("connected");
                client.publish (willTopic.c_str(), "1", true);
                client.subscribe(topic1_sub.c_str());
                client.subscribe(topic2_sub.c_str());
                digitalWrite(LED_BUILTIN, LOW); 
            }else {
                Serial.print("failed, rc=");
                Serial.println(client.state());
            }
        }
    }
    digitalWrite(LED_BUILTIN, LOW); 
}


void loop() {
  checkComms();
  client.loop();
  
  if(r1State){
    unsigned long diff = (millis() - timeR1TurnedOn)/1000/60;
    if(diff >= MAX_ON_MIN){
      log("Timeout hit r1");
      setRelay1(false);
    }
  }
  if(r2State){
    unsigned long diff =(millis() - timeR2TurnedOn)/1000/60;
    if(diff >= MAX_ON_MIN){
      log("Timeout hit r2");
      setRelay2(false);
    }
  }
  delay(10);
}

void log(String l){
  Serial.println("# "+l);
}

void setRelay1(bool state){
  logRelay(1,state);
  if(state){
    timeR1TurnedOn = millis();
  }
  r1State=state;
  _setRelayN(RELAY_1_PIN, state);
  _setLedN(LED_1_PIN, state);
  client.publish (topic1_pub.c_str(), String(state).c_str(), true);
}

void setRelay2(bool state){
  logRelay(2,state);
  if(state){
    timeR2TurnedOn = millis();
  }
  r2State=state;
  _setRelayN(RELAY_2_PIN, state);
  _setLedN(LED_2_PIN, state);
  client.publish (topic2_pub.c_str(), String(state).c_str(), true);
}

void logRelay(int id, bool state){
  //debug log
  log("setting relay "+String(id)+" to "+String(state));
  //machine parseable
  Serial.print("S ");
  Serial.print(id);
  Serial.print(" ");
  Serial.println(state);
}

void _setLedN(int pin, bool state){ 
  if(state){
    // ground led pinm i.e. on
    digitalWrite(pin, LOW); 
  }else{
    // +v led pin - i,e, off
    digitalWrite(pin, HIGH); 
  }
}

void _setRelayN(int pin, bool state){ 
  if(state){
    //pull to gnd to activate relay
    pinMode(pin, OUTPUT); 
  }else{
    //set input to float pin and disable relay
    pinMode(pin, INPUT);
  }
}

bool setTarget(char state, char relay){
  bool setState;
  if(state == '1'){
    setState = true;
  }else if(state == '0'){
    setState = false;
  }else{
    return false;
  }
  
  if(relay == '1'){
      setRelay1(setState);
  }else if (relay == '2'){
      setRelay2(setState);
  }else{
    return false;
  }
  return true;
}



void callback(char* topic, byte* payload, unsigned int length) {
  char content[10];
  
  Serial.print("Message arrived on ");
  Serial.println(topic);
  Serial.println((char*)payload);
  
  if (length != 1){
    Serial.print("expected 1 bytes, got ");
    Serial.println(length);
    return;
  }


  bool setState;
  char state = (char)payload[0];
  if(state == '1'){
    setState = true;
  }else if(state == '0'){
    setState = false;
  }else{
    Serial.println("unknown payload");
    return;
  }

  if(String(topic) == topic1_sub){
    setRelay1(setState);
  }else if(String(topic) == topic2_sub){
    setRelay2(setState);
  }else{
    Serial.println("unknown topic");
    return;
  }
}


