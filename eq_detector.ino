
int vib_pin=4;
int beep_pin=15;
int gap=1000;
int playing = 0;
int preval = 0;
int vibration_timeout_left_millis=0;
int vibration_timeout_millis=2500;


#include "Display.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include "Storage.h"


AsyncWebServer server(80);
Storage storage;
Display display;

const char *soft_ap_ssid          = "EQ Detector";    /*Create a SSID for ESP32 Access Point*/
const char *soft_ap_password      = "123456789";      /*Create Password for ESP32 AP*/

const char* PARAM_SSID = "inputSsid";
const char* PARAM_PASS = "inputPass";
const char* PARAM_URL = "inputUrl";
const char* PARAM_DEVICENAME = "inputDeviceName";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script>
    function submitMessage() {
      alert("Saved value to ESP SPIFFS");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
  </script></head><body>
  <form action="/get" target="hidden-form">
    Device Name : <input type="text" name="inputDeviceName" value="%deviceName%">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>
  <form action="/get" target="hidden-form">
    SSID : <input type="text" name="inputSsid" value="%ssid%">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>
  <form action="/get" target="hidden-form">
    SSID Pass : <input type="text " name="inputPass" value="%ssidpass%">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>
  <form action="/get" target="hidden-form">
    Request URL : <input type="text " name="inputUrl" value="%requesturl%">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form>
  <iframe style="display:none" name="hidden-form"></iframe>
</body></html>)rawliteral";

void setup() {
  Serial.begin(115200);
  display.begin();

  display.status("Booting");
  display.notify("AP MODE INIT   ");
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(soft_ap_ssid, soft_ap_password);
  display.notify(String(WiFi.softAPIP()).c_str());

    
  connectWifi();

  //WEB SERVER
  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam(PARAM_DEVICENAME)) {
      inputMessage = request->getParam(PARAM_DEVICENAME)->value();
      storage.write("devicename",inputMessage);
    }
    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam(PARAM_SSID)) {
      inputMessage = request->getParam(PARAM_SSID)->value();
      storage.write("ssid",inputMessage);

    }
    // GET inputInt value on <ESP_IP>/get?inputInt=<inputMessage>
    else if (request->hasParam(PARAM_PASS)) {
      inputMessage = request->getParam(PARAM_PASS)->value();
      storage.write("ssid_pass",inputMessage);
    }
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_URL)) {
      inputMessage = request->getParam(PARAM_URL)->value();
      storage.write("request_url",inputMessage);
    }
    else {
      inputMessage = "No message sent";
    }
    connectWifi();
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();

  pinMode(vib_pin,INPUT);
  pinMode(beep_pin,OUTPUT);
  

}

void loop() {
  int val;
  val=digitalRead(vib_pin);
  if(val != preval){
    if(vibration_timeout_left_millis == 0){
      Serial.println("vibration detected triggered");
      vibrationDetected();
      preval = val; 
    }
   }else{
    display.status("     STANDBY    ");
   }
}



void twotone()
{
  int f_max=1500;
  int f_min=1000;
  int delay_time=500;
  tone(beep_pin,f_max);
  delay(delay_time);
  tone(beep_pin,f_min);
  delay(delay_time);
  
}

void tone(byte pin, int freq) {
  ledcSetup(0, 2000, 8); // setup beeper
  ledcAttachPin(pin, 0); // attach beeper
  ledcWriteTone(0, freq); // play tone
  playing = pin; // store pin
}
void noTone() {
  tone(playing, 0);
}

void vibrationDetected(){
  vibration_timeout_left_millis = vibration_timeout_millis;
  
  display.status(" ALARM  TRIGGER ");
  sendPostRequest();
  while (vibration_timeout_left_millis > 0) {
    
    twotone();
    vibration_timeout_left_millis= vibration_timeout_left_millis-1000;
  }
  noTone();
  
  vibration_timeout_left_millis=0;
  
}
void sendPostRequest(){
   Serial.println("URL REPORT REQUEST");
   String request_url = storage.read("request_url");
   String device_name = storage.read("devicename");

   if(device_name ==""){
     device_name="undefined device";
   }
   if(request_url !="" && WiFi.status() == WL_CONNECTED){
     Serial.println("URL REPORT STARTED");
     display.notify(" SERVER NOTIFY  ");
     HTTPClient http;

     http.begin(request_url);
     http.addHeader("Content-Type", "application/json");
     int httpResponseCode = http.POST("{\"vibrated\":true,\"device_name\":\""+device_name+"\"}");
     Serial.print("HTTP Response code: ");
     Serial.println(httpResponseCode);
     if(httpResponseCode == 200){
       display.notify(" NOTIFY SUCCESS ");
     }else{
       display.notify("SERVER CODE:"+httpResponseCode);
     }
     
      // Free resources
     http.end();
   }
}





// Replaces placeholder with stored values
String processor(const String& var){
  //Serial.println(var);
  if(var == "ssid"){
    return storage.read("ssid");
  }
  else if(var == "ssidpass"){
    return storage.read("ssid_pass");
  }
  else if(var == "requesturl"){
    return storage.read("request_url");
  }
  else if(var == "deviceName"){
    return storage.read("devicename");
  }
  return String();
}


void connectWifi(){
  String ssid = storage.read("ssid");
  String ssid_pass = storage.read("ssid_pass");
 

  if(ssid != "" && ssid_pass != ""){
    Serial.println("WIFI CONNECTING ");
    display.notify("WIFI CONNECTING ");
    delay(2000);
    
    WiFi.begin(ssid.c_str(), ssid_pass.c_str());
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      display.notify(" WIFI CONNECTED ");
      Serial.println("WIFI CONNECTED");
    }else{
      display.notify(" WIFI CONN FAIL ");
      Serial.println(" WIFI CONN FAIL ");
    }
    
    

    Serial.println(WiFi.localIP());  
  }
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}
