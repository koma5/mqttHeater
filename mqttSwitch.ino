
#include <Ethernet.h>
#include <PubSubClient.h>
#include <SPI.h>

// EEPROM - keeping state
#include <EEPROM.h>
#include "EEPROMAnything.h"

#define ACTOR_COUNT 2

typedef struct {
  int pin;

  String topic;
  String stateTopic;
} actor;

struct mysavedState {
  boolean state;
} savedState;

actor actors[ACTOR_COUNT] = {
  {
    9,
    "vw/heater",
    "vw/heater/state"
  },
  {
    8,
    "vw/something",
    "vw/something/state"
  }
};

char stringBuf[25];

// Update these with values suitable for your network.
byte mac[]    = {  0x90, 0xA2, 0xDA, 0x0D, 0xB9, 0x13 };
byte server[] = { 172, 16, 42, 70 };
byte ip[]     = { 172, 16, 42, 37 };

void callback(char* topic, byte* payload, unsigned int length)
{
  
  payload[length] = '\0';
  String message = (char *) payload;
  String myTopic =  topic;

  for (int actor = 0; actor < ACTOR_COUNT; actor++) {
    if(actors[actor].topic == myTopic) {
      if(length == 2 && message == "on")
      {
        on(actor);
      }

      if(length == 3 && message == "off")
      {
        off(actor);
      }

      if(length == 0 || length == 6 && message == "toggle")
      {
        toggle(actor);
      }

      if(length == 5 && message == "state")
      {
        pubState(actor, digitalRead(actors[actor].pin));
      }
    }
  }


}
EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

void setup()
{
  for (int actor = 0; actor < ACTOR_COUNT; actor++) {
    EEPROM_readAnything(actor, savedState);

    pinMode(actors[actor].pin, OUTPUT);

    if(savedState.state)
    {
      changeState(HIGH, actor, false);
    }
    else
    {
      changeState(LOW, actor, false);
    }

  }

  Ethernet.begin(mac, ip);
}

void loop()
{
  
  if(client.connected())
  {
      client.loop();   
  }
  else
  {
    client.connect("mqttSwitch");
    delay(5000);
    for (int actor = 0; actor < ACTOR_COUNT; actor++) {
      actors[actor].topic.toCharArray(stringBuf, 25);
      client.subscribe(stringBuf);
    }
  }
 
}

void on(int actor)
{
  if(digitalRead(actors[actor].pin) == LOW)
  {
    changeState(HIGH, actor, true);
  }
}

void off(int actor)
{
  if(digitalRead(actors[actor].pin) == HIGH)
  {
    changeState(LOW, actor, true);
  }
}

void toggle(int actor)
{
  changeState(!digitalRead(actors[actor].pin), actor, true);
}

void pubState(int actor, boolean state)
{
  if(state) {
    actors[actor].stateTopic.toCharArray(stringBuf, 25);
    client.publish(stringBuf, "on");
  }
  else {
    actors[actor].stateTopic.toCharArray(stringBuf, 25);
    client.publish(stringBuf, "off");
  }
}

void saveState(int actor, boolean state)
{
  savedState.state = state;
  EEPROM_writeAnything(actor, savedState);
}

void changeState(boolean state, int actor, boolean publishState)
{

  digitalWrite(actors[actor].pin, state);
  
  if(publishState)
  {
    pubState(actor, state);
  }
   
  saveState(actor, state);
   
}



