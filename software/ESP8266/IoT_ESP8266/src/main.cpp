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
const char *ssid = "HA6400_9F1A";
const char *password = "5fa845f3bgas";

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
  read_from_serial_and_send();
}

void print_time(){
  Serial.print("Time: ");
  Serial.println(NTP_timeClient.getFormattedTime());
}

void print_date(){
  unsigned long epochTime = NTP_timeClient.getEpochTime();
  time_t rawtime = epochTime;
  struct tm * ti = localtime(&rawtime);
  char dateStr[12];
  snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d", 
           ti->tm_year + 1900, ti->tm_mon + 1, ti->tm_mday);
  Serial.print("Date: ");
  Serial.println(dateStr);
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

void time_update_event(){
  if (millis() - last_time >= ntp_update_time)
  {
    time_update();    
  }
  //update NTP time
  
  //set rtc by ntp epoch time
  // RTC = NTP_timeClient.getEpochTime(); //for rtc

  // struct timeval tv = {.tv_sec = RTC, .tv_usec = 0}; //for rtc
  // settimeofday(&tv, NULL); //for rtc
}

void time_update(){
  if (NTP_timeClient.update())
  {
    print_time();
    print_date();  
    last_time = millis();
  }else system_restart();    
  //update NTP time
  
  //set rtc by ntp epoch time
  // RTC = NTP_timeClient.getEpochTime(); //for rtc

  // struct timeval tv = {.tv_sec = RTC, .tv_usec = 0}; //for rtc
  // settimeofday(&tv, NULL); //for rtc
}

void read_from_serial_and_send(){
  if (Serial.available())
  {
    String input_serial= Serial.readStringUntil('\n');
    Serial.print("Responce: ");
    Serial.println(input_serial);
    http_post(input_serial);
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