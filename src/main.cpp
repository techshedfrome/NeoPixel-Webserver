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
#define LED_PIN 14
#define LED_COUNT 12
#endif

#define LOGGING true

const char *ssid = STASSID;
const char *password = STAPSK;
const char *hostname = MDNS_HOSTNAME;
bool runAnimation = true;

const int onboardLed = 16;

int showR = 0;
int showG = 0;
int showB = 0;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
WiFiClient wifi;

void initialiseNeopixel(){
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(10); // Set BRIGHTNESS (max = 255)
  strip.show();            // Turn OFF all pixels ASAP
}

void setUpOnboardLed(){
  pinMode(onboardLed, OUTPUT);
  digitalWrite(onboardLed, 0);
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
  digitalWrite(onboardLed, 1);
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
  digitalWrite(onboardLed, 0);
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

void setAllPixels(byte R, byte G, byte B)
{
  for (int i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, R, G, B);
  }
}

void fade(bool fadeUp, byte targetR, byte targetG, byte targetB, int pause, int steps)
{
  int tmpR, tmpG, tmpB;
  for (int s = 1; s <= steps; s++)
  {
    int directionalMultiplier = fadeUp ? s : steps - s;
    tmpR = (float)targetR / (float)steps * (float)directionalMultiplier;
    tmpG = (float)targetG / (float)steps * (float)directionalMultiplier;
    tmpB = (float)targetB / (float)steps * (float)directionalMultiplier;

    setAllPixels(tmpR, tmpG, tmpB);

    strip.show();
    delay(pause);
  }
}

void breathe(int pause, int steps, byte R, byte G, byte B)
{
  fade(true , R, G, B, pause, steps);
  fade(false, R, G, B, pause, steps);
  delay(pause * 4);
}

bool showBreathe = false;
void showColors(int r, int g, int b)
{
  showBreathe = false;
  showR = r;
  showG = g;
  showB = b;
  showBreathe = true;
}

void showNormal() { showColors(13, 7, 89); }
void showWarning() { showColors(95, 87, 12); }
void showAlert() { showColors(98, 7, 24); }

bool callHttpGet = false;
void setupWebServer(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "hello from esp8266!");
  });
  server.on("/getsomething", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(onboardLed, 1);
    callHttpGet = true;
    request->send(200, "text/plain", "httpTest");
    digitalWrite(onboardLed, 0);
  });

  server.on("/spin", HTTP_GET, spin);
  server.on("/normal", HTTP_GET, [](AsyncWebServerRequest *request) { showNormal(); request->send(200, "text/plain", "showing NORMAL state"); });
  server.on("/warning", HTTP_GET, [](AsyncWebServerRequest *request) { showWarning(); request->send(200, "text/plain", "showing WARNING state"); });
  server.on("/alert", HTTP_GET, [](AsyncWebServerRequest *request) { showAlert(); request->send(200, "text/plain", "showing ALERT state"); });
  server.on("/clear", HTTP_GET, [](AsyncWebServerRequest *request) { showBreathe = false; request->send(200, "text/plain", "clear state"); });

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

void setPixels(uint32_t color, int startIndex, int skip)
{
  strip.clear();
  for (int pixel = startIndex; pixel < strip.numPixels(); pixel += skip)
      strip.setPixelColor(pixel, color);
  strip.show();
}

void rotatePixels(uint32_t color, int rotationStep, int animationStepDelay)
{
  for (int b = 0; b < rotationStep; b++)
  {
    setPixels(color, b, strip.numPixels()/4);
    delay(animationStepDelay);
  }
}

void animate(uint32_t color, int animationStepSize, int animationStepDelay, int iterations)
{
  for (int a = 0; a < iterations; a++)
      rotatePixels(color, animationStepSize, animationStepDelay);
}

void animateThroughColors(int animationStepSize, int animationStepDelay, int iterationsPerColor)
{
  if (runAnimation) animate(strip.Color(127, 127, 127), animationStepSize, animationStepDelay, iterationsPerColor);
  if (runAnimation) animate(strip.Color(255, 255, 000), animationStepSize, animationStepDelay, iterationsPerColor);
  if (runAnimation) animate(strip.Color(127, 000, 000), animationStepSize, animationStepDelay, iterationsPerColor);
  if (runAnimation) animate(strip.Color(000, 255, 000), animationStepSize, animationStepDelay, iterationsPerColor);
  if (runAnimation) animate(strip.Color(000, 000, 127), animationStepSize, animationStepDelay, iterationsPerColor);
  if (runAnimation) animate(strip.Color(143, 000, 255), animationStepSize, animationStepDelay, iterationsPerColor);
}



void disco()
{
  // strip.clear();
  // strip.show();

  // animateThroughColors(3, 100,  5);
  // animateThroughColors(4,  50,  5);
  // animateThroughColors(3,  25, 10);
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
  if (showBreathe)
    breathe(10, 30, showR, showG, showB);

  if (callHttpGet){
    makeHttpGetRequest();
    callHttpGet = false;
  }

}
