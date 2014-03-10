
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
byte ip[]     = { 172, 16, 0, 37 };

void callback(char* topic, byte* payload, unsigned int length)
{
  
  if(length == 2 && payload[0] == 'o' && payload[1] == 'n')
  {
    on(led);
  }
  
  if(length == 3 && payload[0] == 'o' && payload[1] == 'f' && payload[2] == 'f')
  {
    off(led);
  }

  if(length == 0 || length == 6 && payload[0] == 't' && payload[1] == 'o' && payload[2] == 'g' && payload[3] == 'g' && payload[4] == 'l' && payload[5] == 'e')
  {
    toggle(led);
  }

}
EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

void setup()
{
  EEPROM_readAnything(0, ledConfig);
  
  if(ledConfig.state)
  {
    high(ledConfig.pin, false);
  }
  else
  {
    low(ledConfig.pin, false);
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
    high(pin, true);
  }
}

void off(int pin)
{
  if(digitalRead(pin) == HIGH)
  {
    low(pin, true);
  }
}

void toggle(int pin)
{
  if(digitalRead(pin) == LOW)
  {
    high(pin, true);
  }
  else
  {
    low(pin, true);
  }
}

void saveState(int pin, boolean state)
{
  ledConfig.pin = pin;
  ledConfig.state = state;
  EEPROM_writeAnything(0, ledConfig);
}

void high(int pin, boolean publishState)
{
  
  digitalWrite(pin, HIGH);
  
  if(publishState)
  {
    client.publish("vw/heater/state", "on");
  }
   
   saveState(pin, HIGH);
   
}

void low(int pin, boolean publishState)
{
  
  digitalWrite(pin, LOW);
  
  if(publishState)
  {
    client.publish("vw/heater/state", "off");
  }
   
   saveState(pin, LOW);
   
}



