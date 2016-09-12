#include <Wire.h>
#include <Adafruit_HTU21DF.h>
//#include "DHT.h"
#include <aREST.h>

#define HTU_COEFF_TEMP  -0.15f
//#define DHTPIN          2
//#define DHTTYPE         DHT11
#define VOLTS_SLOPE     0.02786f   // (5.0/1023)*(1.0/(1.0 + 4.7)

Adafruit_HTU21DF htu = Adafruit_HTU21DF();
//DHT dht(DHTPIN, DHTTYPE);

aREST rest = aREST();

float temp, humidity, volts;

/*-----------------------------------------------------------------------------
  setup
-----------------------------------------------------------------------------*/
void setup() {
  Serial.begin(9600);
  
  if(!htu.begin()) {
    Serial.println(F("couldn't initialize htu"));
  }
//  dht.begin();
  
  rest.variable("temp", &temp);
  rest.variable("humidity", &humidity);
  rest.variable("volts", &volts);
  
  rest.set_id("42");
  rest.set_name("solar-fan");
}

/*-----------------------------------------------------------------------------
  loop
-----------------------------------------------------------------------------*/
void loop() {
  rest.handle(Serial);
  
  temp = htu.readTemperature();
  humidity = htu.readHumidity() + (25.0f - temp)*HTU_COEFF_TEMP;
//  temp = dht.readTemperature();
//  humidity = dht.readHumidity();
  
  volts = (float)analogRead(A0)*VOLTS_SLOPE;

//  Serial.print(F("\ntemp =\t"));
//  Serial.print(temp, 2);
//  Serial.print(F("\nhumidity =\t"));
//  Serial.print(humidity, 2);
//  Serial.print(F("\nvolts =\t"));
//  Serial.print(volts, 2);
//  Serial.print(F("\n"));
//  Serial.flush();
}
