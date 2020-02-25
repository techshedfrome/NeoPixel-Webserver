# Experimenting With ESP Webserver & NeoPixel

Code has been patched together from examples to experiment with options for making animated lighting effects that respond to events, data values or user actions on the network.

Initially, it's just using the ESP8266 async webserver, (with mDNS to make it available on the LAN as http://disco.local), driving a NeoPixel ring light, toggling whether the animation is active via HTTP GET requests to a specific path (currently http://disco.local/spin).

Put together by [Techshed Frome](https://techshedfrome.org).


## Notes
* The intention is to read data from an InfluxDB instance to show the state of sensor data
  * we could forward the statuses to the ESP via MQTT
  * but it's good to have some user-control over HTTP, so probably an HTTP API will remain on the ESP (not sure if we can combine that with an MQTT listener/subscriber, but we'll give it a go!)
* We could also pull the last reading directly form the device via `http://[device-ip-address]/values, but with the influx values, we can avg out the data etc.
* Only tested with NodeMCU esp8266 board so far
* Neopixel set up is for a 12 LED ring, but easy to change

## PlatformIO
* Built using [PlatformIO](https://platformio.org/) (Extension for [VSCode](https://code.visualstudio.com/) that supports development for the Arduino platform)
  * More of a full IDE experience than Arduino IDE
  * Importantly, the ESP Async Webserver libarary is not available in the Arduino IDE any more, but is in PlatformIO!
* Had a problem with AsyncTCP library - turned out that the ESP32 library was being pulled in rather than the ESP8266 version (listed as ESPAsyncTCP)
  * uninstalled the ESP32 version and it built/worked
* In order to make use of the serial monitor we need to add `monitor_speed = 115200` to the `platformio.ini` file tom match the baud speed set via `Serial.begin(115200);` in the `main.cpp` file
* key bindings in VSCode: https://docs.platformio.org/en/latest/ide/vscode.html#key-bindings
* CLI setup - https://docs.platformio.org/en/latest/installation.html#piocore-install-shell-commands
* debug log output is controlled via `build_flags` in `platformio.ini` allowing debug levels to be set in PlatformIO in a similar way to the Arduino IDE
  * Specific library debug flags can be turned on individually.  The below enables debug output on ESP8266 for the following libraries [CORE, HTTP_CLIENT] with output over serial port.
  * `build_flags = -DDEBUG_ESP_CORE -DDEBUG_ESP_HTTP_CLIENT -DDEBUG_ESP_PORT=Serial`
* in case of build conflicts with WiFi client calls:
  * add the following to `platformio.ini` `lib_ignore = WiFi`
  * https://community.platformio.org/t/solved-first-defined-here-error-on-building-esp8266webserver/8164
* Problems with the HTTP Client library are actually related to making the call from an async callback
  * HTTP Client uses WiFiClient's calls which don't appear to be working from the async webserver's callbacks
    * applied temporary fix inspired by this: https://community.platformio.org/t/debugging-wificlient-with-an-esp8266-on-platformio/9891/21
    * root cause may be similar to: https://github.com/espressif/arduino-esp32/issues/1595

## AdrionoJSON Library
We've imported ArduinoJSON to deserialise the data we fetch from InfluxDb.
* https://arduinojson.org/book/deserialization_tutorial6.pdf#page=10

## References Credits:
* Async webserver example code: https://github.com/me-no-dev/ESPAsyncWebServer/tree/master/examples/simple_server
* Basic light ring animation:  https://github.com/JonnyBanana/NEOPIXEL_WS2812_5050_SKETCHS/blob/master/Code/Like_Disco/Like_Disco.ino
* Awesome dummy API returning various JSON objects for testing purposes: http://jsonplaceholder.typicode.com
* Breathe animation inspired by https://arduino.stackexchange.com/questions/21312/neopixel-ring-how-to-breathe-in-a-circle-motion
