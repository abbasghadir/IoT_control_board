#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
// #include <time.h> //for rtc
#include <pre_def.h>

const char* endpoint_url = "http://192.168.1.5/IoT_input_detection.php";

unsigned long last_time = 0;

//WiFi setting
const char *ssid = "SSID";
const char *password = "PASSWORD";
const char* time_sync_cmd = "successfully";

WiFiUDP ntpUDP;

const char *ntp_server_url = "pool.ntp.org";
const uint offset_time = 12600; //in Iran
const uint ntp_update_time = 6000; //update time In msec
NTPClient NTP_timeClient(ntpUDP,ntp_server_url,offset_time,0);

// time_t RTC; //for rtc

void setup() {
  Serial.begin(115200);
  Serial.println("start project!");
  wificonfig();
  NTP_timeClient.begin();
  time_update();
}

void loop() {
  time_update_event();
  if (uart_reader() != "N")
  {
    http_post(uart_reader());
  }
}

void wificonfig(){
  printf("Connecting to: %s\n", ssid);
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected!");
  Serial.print("Local IP address is:");
  Serial.println(WiFi.localIP());  
}

String uart_reader(){
  if (Serial.available())
  {
    String input_serial= Serial.readStringUntil('\n');
    Serial.print("Responce: ");
    Serial.println(input_serial);
    return input_serial;
  }else return "N";
}

void time_update_event(){
  if (millis() - last_time >= ntp_update_time)
  {
    time_update();    
  }
}

void time_update(){
  Serial.println("Waiting for NTP update");
  unsigned long epochTime;
  while (1)
  {
    if (NTP_timeClient.update())
    {
      epochTime = NTP_timeClient.getEpochTime();
      Serial.print("epoch:");
      Serial.println(epochTime);
      delay(100);
      if (uart_reader() == time_sync_cmd)
      {
        last_time = millis();
        break;
      }
    }else system_restart();
  }
}
void http_post(String input_serial){
  JsonDocument doc;
  doc["Result"] = input_serial;
  String jsonpayload;
  serializeJson(doc,jsonpayload); 

  HTTPClient http;
  WiFiClient client;

  http.begin(client,endpoint_url);
  http.addHeader("Content-Type", "application/json");
  int httpresponse = http.POST(jsonpayload);
  if (httpresponse>0)
  {
    printf("HTTP response code: %s\n",String(httpresponse));
    printf("Response is: %s\n",http.getString());
  }else printf("Error on send data: ", String(httpresponse));
  http.end();  
}