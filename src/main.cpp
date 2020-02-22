#include <ArduinoJson.h>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#ifdef ESP32
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#elif defined(ESP8266)
#include <ESP8266mDNS.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#else
#endif

AsyncWebServer server(80);
const char *PARAM_MESSAGE = "message";

#ifndef STASSID
#define STASSID "Remakery"
#define STAPSK "Remakery"
#define MDNS_HOSTNAME "disco"
#define LED_PIN 2
#define LED_COUNT 12
#endif

#define LOGGING true

const char *ssid = STASSID;
const char *password = STAPSK;
const char *hostname = MDNS_HOSTNAME;
bool runAnimation = false;

const int led = 16;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
WiFiClient wifi;

void initialiseNeopixel(){
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  strip.show();            // Turn OFF all pixels ASAP
}

void setUpOnboardLed(){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
}

void setupNetworking(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }
  if (MDNS.begin(hostname))
  {
    Serial.println("MDNS responder started");
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void notFound(AsyncWebServerRequest *request)
{
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URL: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";
  for (uint8_t i = 0; i < request->args(); i++)
  {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  request->send(404, "text/plain", "Not found");
  digitalWrite(led, 0);
}

void spin(AsyncWebServerRequest *request)
{
  String message;
  runAnimation = !runAnimation;
  if (runAnimation)
  {
    message = "Disco!";
  }
  else
  {
    message = "No Disco :(";
  }
  request->send(200, "text/html", "<h1>" + message + "</h1>");
}

bool callHttpGet = false;
void setupWebServer(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "hello from esp8266!");
  });
  server.on("/getsomething", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(led, 1);
    callHttpGet = true;
    request->send(200, "text/plain", "httpTest");
    digitalWrite(led, 0);
  });

  server.on("/spin", HTTP_GET, spin);

  // handle a POST request to <IP>/ with a form field message set to <message>
  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam(PARAM_MESSAGE, true))
    {
      message = request->getParam(PARAM_MESSAGE, true)->value();
    }
    else
    {
      message = "No message sent";
    }
    request->send(200, "text/plain", "Hello, POST: " + message);
  });

  server.onNotFound(notFound);

  server.begin();
  Serial.println("HTTP server started");
}

void setup()
{
  Serial.begin(115200);
  initialiseNeopixel();
  setUpOnboardLed();
  setupNetworking();
  setupWebServer();
}

void theaterChase(uint32_t color, int wait)
{
  for (int a = 0; a < 10; a++)
  {
    for (int b = 0; b < 3; b++)
    {
      strip.clear();

      for (int c = b; c < strip.numPixels(); c += 3)
      {
        strip.setPixelColor(c, color);
      }
      strip.show();
      delay(wait);
    }
  }
}

void theaterChaseTwo(uint32_t color, int wait)
{
  for (int a = 0; a < 5; a++)
  {
    for (int b = 0; b < 4; b++)
    {
      strip.clear();

      for (int c = b; c < strip.numPixels(); c += 3)
      {
        strip.setPixelColor(c, color);
      }
      strip.show();
      delay(wait);
    }
  }
}

void theaterChaseThree(uint32_t color, int wait)
{
  for (int a = 0; a < 10; a++)
  {
    for (int b = 0; b < 2; b++)
    {
      strip.clear();

      for (int c = b; c < strip.numPixels(); c += 3)
      {
        strip.setPixelColor(c, color);
      }
      strip.show();
      delay(wait);
    }
  }
}

void disco()
{
  strip.clear();
  strip.show();
  if (runAnimation)
    theaterChaseTwo(strip.Color(127, 127, 127), 100); // set brightness to 100
  if (runAnimation)
    theaterChaseTwo(strip.Color(255, 255, 0), 100);
  if (runAnimation)
    theaterChaseTwo(strip.Color(127, 0, 0), 100);
  if (runAnimation)
    theaterChaseTwo(strip.Color(0, 255, 0), 100);
  if (runAnimation)
    theaterChaseTwo(strip.Color(0, 0, 127), 100);
  if (runAnimation)
    theaterChaseTwo(strip.Color(143, 0, 255), 100);

  if (runAnimation)
    theaterChase(strip.Color(127, 127, 127), 50); // 50 = half brightness
  if (runAnimation)
    theaterChase(strip.Color(255, 255, 0), 50);
  if (runAnimation)
    theaterChase(strip.Color(127, 0, 0), 50);
  if (runAnimation)
    theaterChase(strip.Color(0, 255, 0), 50);
  if (runAnimation)
    theaterChase(strip.Color(0, 0, 127), 50);
  if (runAnimation)
    theaterChase(strip.Color(143, 0, 255), 50);

  if (runAnimation)
    theaterChase(strip.Color(127, 127, 127), 25); // set brightness to 25
  if (runAnimation)
    theaterChase(strip.Color(255, 255, 0), 25);
  if (runAnimation)
    theaterChase(strip.Color(127, 0, 0), 25);
  if (runAnimation)
    theaterChase(strip.Color(0, 255, 0), 25);
  if (runAnimation)
    theaterChase(strip.Color(0, 0, 127), 25);
  if (runAnimation)
    theaterChase(strip.Color(143, 0, 255), 25);
}

void makeHttpGetRequest()
{
  HTTPClient http;

  Serial.print("[HTTP Client] begin");
  //String url = "http://jsonplaceholder.typicode.com/todos/1";

  // Fetch the single newest record from InfluxDb
  String url = "http://techshed.local:8086/query?db=luftdaten&q=select%20*%20from%20feinstaub%20order%20by%20time%20desc%20%20limit%201";
  if (http.begin(wifi, url))
  {
    Serial.printf("\n[HTTP Client] GET - %s\n", url.c_str());
    // start connection and send HTTP header
    int httpCode = http.GET();
    if (httpCode > 0)
    { // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP Client] Response code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        String payload = http.getString();
        StaticJsonDocument<10000> doc;

        DeserializationError error = deserializeJson(doc, payload);
        if (error)
          Serial.printf("[JSON] Error: %s\n", error.c_str());

        JsonObject results = doc["results"][0];
        JsonObject series = results["series"][0];
        String values = series["values"][0];
        String time = series["values"][0][0];

        Serial.println(payload);
        Serial.println(time);
      }
    }
    else
    {
      Serial.printf("[HTTP Client] Fatal Internal Error: %s %d\n", http.errorToString(httpCode).c_str(), httpCode);
    }
    http.end();
  }
  else
  {
    Serial.printf("[HTTP Client] Unable to connect\n");
  }
}

void loop()
{
  MDNS.update();
  disco();

  if (callHttpGet){
    makeHttpGetRequest();
    callHttpGet = false;
  }

}
