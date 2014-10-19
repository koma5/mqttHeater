
#include <Ethernet.h>
#include <PubSubClient.h>
#include <SPI.h>

// EEPROM - keeping state
#include <EEPROM.h>
#include "EEPROMAnything.h"

int led = 9;

struct config_led
{
    int pin;
    boolean state;
} ledConfig;


// Update these with values suitable for your network.
byte mac[]    = {  0x90, 0xA2, 0xDA, 0x0D, 0xB9, 0x13 };
byte server[] = { 172, 16, 0, 70 };
byte ip[]     = { 172, 16, 42, 37 };

void callback(char* topic, byte* payload, unsigned int length)
{
  
  payload[length] = '\0';
  String message = (char *) payload;
  
  if(length == 2 && message == "on")
  {
    on(led);
  }
  
  if(length == 3 && message == "off")
  {
    off(led);
  }

  if(length == 0 || length == 6 && message == "toggle")
  {
    toggle(led);
  }
  
  if(length == 5 && message == "state")
  {
    if(digitalRead(led) == HIGH)
    {
      pubState(led, true);
    }
    else {
      pubState(led, false);
    }
  }

}
EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

void setup()
{
  EEPROM_readAnything(0, ledConfig);
  
  if(ledConfig.state)
  {
    changeState(HIGH, ledConfig.pin, false);
  }
  else
  {
    changeState(LOW, ledConfig.pin, false);
  }  
  
  Ethernet.begin(mac, ip);
  pinMode(led, OUTPUT);
}

void loop()
{
  
  if(client.connected())
  {
      client.loop();   
  }
  else
  {
    client.connect("arduinoVwHeaterControl");
    delay(5000);
    client.subscribe("vw/heater");
  }
 
}

void on(int pin)
{
  if(digitalRead(pin) == LOW)
  {
    changeState(HIGH, pin, true);
  }
}

void off(int pin)
{
  if(digitalRead(pin) == HIGH)
  {
    changeState(LOW, pin, true);
  }
}

void toggle(int pin)
{
  if(digitalRead(pin) == LOW)
  {
    changeState(HIGH, pin, true);
  }
  else
  {
    changeState(LOW, pin, true);
  }
}

void pubState(int pin, boolean state)
{
  if(state) {
    client.publish("vw/heater/state", "on");
  }
  else {
    client.publish("vw/heater/state", "off");
  }
}

void saveState(int pin, boolean state)
{
  ledConfig.pin = pin;
  ledConfig.state = state;
  EEPROM_writeAnything(0, ledConfig);
}

void changeState(boolean state, int pin, boolean publishState)
{
  
  digitalWrite(pin, state);
  
  if(publishState)
  {
    pubState(pin, state);
  }
   
   saveState(pin, state);
   
}



