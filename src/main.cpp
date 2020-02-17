

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
#define STASSID ""
#define STAPSK ""
#define MDNS_HOSTNAME "disco"
#define LED_PIN 2
#define LED_COUNT 12
#endif

#define LOGGING true

const char *ssid = STASSID;
const char *password = STAPSK;
const char *hostname = MDNS_HOSTNAME;
bool lightIsOn = false;

const int led = 16;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

WiFiClient wifi;

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

void httpTest()
{
  HTTPClient http;
  WiFiClient client;

  Serial.print("[HTTP Client] begin");
  String url = "http://jsonplaceholder.typicode.com/todos/1";
  // String url = "http://techshed.local:8086/query?q=show%20databases";
  // String url = "http://10.10.1.116:8086/query?q=show%20databases";
  if (http.begin(client, url))
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
        Serial.println(payload);
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

void simpleHttp()
{
  // char *host = "192.168.1.89";
  // const IPAddress ip = IPAddress(192,168,1,89);
  // uint16_t port = 8888;
  // char *path = "/gogs/";

  // HttpClient client = HttpClient(wifi, ip, 8888);
  // Serial.println("making GET request");
  // client.get(path);

  // // read the status code and body of the response
  // int statusCode = client.responseStatusCode();
  // Serial.printf("[HTTP Client] Response code: %d\n", statusCode);
  // if (statusCode > 0)
  // { // HTTP header has been send and Server response header has been handled
  //   String response = client.responseBody();
  //   Serial.println(response);
  //   Serial.println(response);
  // }
  // else
  // {
  //   Serial.printf("[HTTP Client] Fatal Internal Error");
  // }
}

void showGif(AsyncWebServerRequest *request)
{
  static const uint8_t gif[] PROGMEM = {
      0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x10, 0x00, 0x10, 0x00, 0x80, 0x01,
      0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x2c, 0x00, 0x00, 0x00, 0x00,
      0x10, 0x00, 0x10, 0x00, 0x00, 0x02, 0x19, 0x8c, 0x8f, 0xa9, 0xcb, 0x9d,
      0x00, 0x5f, 0x74, 0xb4, 0x56, 0xb0, 0xb0, 0xd2, 0xf2, 0x35, 0x1e, 0x4c,
      0x0c, 0x24, 0x5a, 0xe6, 0x89, 0xa6, 0x4d, 0x01, 0x00, 0x3b};
  char gif_colored[sizeof(gif)];
  memcpy_P(gif_colored, gif, sizeof(gif));
  // Set the background to a random set of colors
  gif_colored[16] = millis() % 256;
  gif_colored[17] = millis() % 256;
  gif_colored[18] = millis() % 256;

  request->send(200, "image/gif", gif_colored);
}

bool makeCall = false;
void setup()
{
  Serial.begin(115200);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  strip.show();            // Turn OFF all pixels ASAP

  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "hello from esp8266!");
  });
  server.on("/test1", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(led, 1);
    // httpTest();
    makeCall = true;
    request->send(200, "text/plain", "httpTest");
    digitalWrite(led, 0);
  });
  server.on("/test2", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(led, 1);
    simpleHttp();
    request->send(200, "text/plain", "simpleHttp");
    digitalWrite(led, 0);
  });

  server.on("/gif", HTTP_GET, showGif);

  // Send a GET request to <IP>/get?message=<message>
  server.on("/spin", HTTP_GET, [](AsyncWebServerRequest *request) {
    String message;
    lightIsOn = !lightIsOn;
    if (lightIsOn)
    {
      message = "Disco!";
    }
    else
    {
      message = "No Disco :(";
    }
    request->send(200, "text/html", "<h1>" + message + "</h1>");
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request) {
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
  if (lightIsOn)
    theaterChaseTwo(strip.Color(127, 127, 127), 100); // set brightness to 100
  if (lightIsOn)
    theaterChaseTwo(strip.Color(255, 255, 0), 100);
  if (lightIsOn)
    theaterChaseTwo(strip.Color(127, 0, 0), 100);
  if (lightIsOn)
    theaterChaseTwo(strip.Color(0, 255, 0), 100);
  if (lightIsOn)
    theaterChaseTwo(strip.Color(0, 0, 127), 100);
  if (lightIsOn)
    theaterChaseTwo(strip.Color(143, 0, 255), 100);

  if (lightIsOn)
    theaterChase(strip.Color(127, 127, 127), 50); // 50 = half brightness
  if (lightIsOn)
    theaterChase(strip.Color(255, 255, 0), 50);
  if (lightIsOn)
    theaterChase(strip.Color(127, 0, 0), 50);
  if (lightIsOn)
    theaterChase(strip.Color(0, 255, 0), 50);
  if (lightIsOn)
    theaterChase(strip.Color(0, 0, 127), 50);
  if (lightIsOn)
    theaterChase(strip.Color(143, 0, 255), 50);

  if (lightIsOn)
    theaterChase(strip.Color(127, 127, 127), 25); // set brightness to 25
  if (lightIsOn)
    theaterChase(strip.Color(255, 255, 0), 25);
  if (lightIsOn)
    theaterChase(strip.Color(127, 0, 0), 25);
  if (lightIsOn)
    theaterChase(strip.Color(0, 255, 0), 25);
  if (lightIsOn)
    theaterChase(strip.Color(0, 0, 127), 25);
  if (lightIsOn)
    theaterChase(strip.Color(143, 0, 255), 25);
}

void callOnce()
{
  httpTest();
  makeCall = false;
}

void loop()
{
  MDNS.update();
  disco();
  if (makeCall)
    callOnce();
}
