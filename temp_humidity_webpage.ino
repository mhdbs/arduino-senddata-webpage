
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include "DHT.h"

#define DHTTYPE DHT11
#define DHTPIN D7

DHT dht(DHTPIN, DHTTYPE, 11);

const int analogIn = A0;
int mVperAmp = 185; // use 100 for 20A Module and 66 for 30A Module
int RawValue = 0;
int ACSoffset = 2500;
double Voltage = 0;
double Amps = 0;

float temp, humid;

long duration, distance;
int volt = 230;

#define TRIGGERPIN D1
#define ECHOPIN D2

#define buz D3

const char *ssid = "MHDBS";
const char *password = "bilal";
char server[] = "www.mhdbs.com"; //eg: 192.168.0.222

WiFiClient client;

void setup()
{
  Serial.begin(9600);
  delay(10);
  SPI.begin();
  pinMode(analogIn, INPUT);
  pinMode(buz, OUTPUT);
  pinMode(TRIGGERPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);

  dht.begin();
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  //  server.begin();
  Serial.println("Server started");
  Serial.print(WiFi.localIP());
  delay(1000);
  Serial.println("connecting...");
}
void loop()
{
  RawValue = analogRead(analogIn);
  Voltage = (RawValue / 1024.0) * 5000; // Gets you mV
  Amps = ((Voltage - ACSoffset) / mVperAmp);
  Amps = -Amps;
  delay(50);

  temp = dht.readTemperature();
  humid = dht.readHumidity();

  digitalWrite(TRIGGERPIN, LOW);
  delayMicroseconds(3);

  digitalWrite(TRIGGERPIN, HIGH);
  delayMicroseconds(12);

  digitalWrite(TRIGGERPIN, LOW);
  duration = pulseIn(ECHOPIN, HIGH);
  distance = (duration / 2) / 29.1;
  Serial.print(distance);
  Serial.println("Cm");
  Serial.print("Present Current");
  Serial.println(Amps);
  Serial.println("Present temperature:");
  Serial.println(temp);
  Serial.println("persent humidity:");
  Serial.println(humid);
  if (distance >= 15)
  {
    digitalWrite(buz, LOW);
  }

  else if (distance <= 80)
  {
    digitalWrite(buz, HIGH);
  }

  delay(2000);

  Sending_To_DB();
}

void Sending_To_DB() //CONNECTING WITH MYSQL
{
  if (client.connect(server, 80))
  {
    Serial.println("connected");
    String url = "http://mhdbs.com/user/getdata/data?s1=";
    url += Amps;
    url += "&s2=";
    url += volt;
    url += "&s3=";
    url += temp;
    url += "&s4=";
    url += distance;
    // This will send the request to the server
    Serial.print("Requesting URL: ");
    Serial.println(url);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");
  }
  else
  {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
  client.stop();
}
