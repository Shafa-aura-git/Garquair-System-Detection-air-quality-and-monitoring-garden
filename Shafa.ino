#include <WiFi.h>
#include <PubSubClient.h>
const char* ssid = "Sap";
const char* password= "sapa020077";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;


#include "MQUnifiedsensor.h"
#define BOARD ("ESP-32")
#define PIN (33)
#define TYPE ("MQ-7")
#define VOLTAGE_RESOLUTION (3.3)
#define ADC_BIT_RESOLUTION (12)
#define RATIO_MQ7_CLEANAIR (27.5)
MQUnifiedsensor MQ7(BOARD, VOLTAGE_RESOLUTION, ADC_BIT_RESOLUTION, PIN, TYPE);
float mq7Value = 0;
unsigned long starttime;
unsigned long sampletime_ms = 5 * 1000; // Smaple time 1s

#include "MQ135.h"
#define esp32_sample_rate 4096
#define arduino_sample_rate 1024
#define esp32_pin_no 35
#define MQ135_PULLDOWNRES 10000         
#define default_ppm_CO2 408
#define default_ppm_NH4 5


#include "DHT.h"
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#include "MQ135.h"
#define esp32_sample_rate 4096
#define arduino_sample_rate 1024
#define esp32_pin_no 35
#define MQ135_PULLDOWNRES 10000         
#define default_ppm_CO2 408
#define default_ppm_NH4 5

#include <math.h>

int i = 0;
double rs;
double ro_co2;
double ro_nh4;
double rs_ro_ratio_co2;
double rs_ro_ratio_nh4;
int co2_ppm;
float nh4_ppm;
int adcread;

#define relay 13
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setup_wifi();

client.setServer(mqtt_server,1883);
  client.setCallback(callback);

  pinMode(relay,OUTPUT);

  dht.begin();

  MQ7.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ7.setA(99.042);
  MQ7.setB(-1.518); // Configure the equation to to calculate H2 concentration
  MQ7.init();

  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++)
  {
    MQ7.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ7.calibrate(RATIO_MQ7_CLEANAIR);
    Serial.print(".");
  }
  MQ7.setR0(calcR0 / 10);
  Serial.println("  done!.");

  if (isinf(calcR0))
  {
    Serial.println("Warning: Masalah koneksi, R0 tidak terbatas (Sirkuit terbuka terdeteksi), periksa kabel dan suplai Anda");
    while (1)
      ;
  }
  if (calcR0 == 0)
  {
    Serial.println("Warning: Masalah koneksi, R0 adalah nol (pin analog pendek ke ground), periksa kabel dan suplai Anda");
    while (1)
      ;
  }

  MQ7.serialDebug(true); // uncomment if you want to print the table on the serial port
  
  int avg = 0;
  for(int i=0; i<10;i++){
  avg += analogRead(esp32_pin_no);
  delay(2000);
  }
  avg = avg/10;
  MQ135_init(esp32_sample_rate,avg);
}





void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  MQ7.update();
  float CO = MQ7.readSensor();

  int adc = analogRead(esp32_pin_no);
  int CO2 = read_CO2(esp32_sample_rate,adc);
  int NH4 = read_NH4(esp32_sample_rate,adc);

  int soilmois = analogRead(32);
  soilmois = map(soilmois, 0, 4095, 0, 100);
  soilmois = (soilmois - 100) * -1;

  float h = dht.readHumidity();
    //read temperature as Celcius (the default)
  float t= dht.readTemperature();
  Serial.print(NH4); Serial.print(' ');Serial.print(CO2); Serial.print(' ');
  Serial.print(h); Serial.print(' '); Serial.print(t);Serial.print(' ');
  Serial.print(soilmois); Serial.print(' ');
  Serial.println(CO);

  char soilString[8];
      dtostrf(soilmois, 1, 0, soilString);
      client.publish("/mqtt-esp32/soilmois/shafa", soilString);
  
  delay(1000);

}

