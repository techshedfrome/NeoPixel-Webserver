# Experimenting With ESP Webserver & NeoPixel

Code has been patched together from examples to experiment with options for making animated lighting effects that respond to events, data values or user actions on the network.

Initially, it's just using the ESP8266 async webserver, (with mDNS to make it available on the LAN as http://disco.local), driving a NeoPixel ring light, toggling whether the animation is active via HTTP GET requests to a specific path (currently http://disco.local/spin).

Put together by [Techshed Frome](https://techshedfrome.org).


## Notes

* Built using [PlatformIO](https://platformio.org/) (Extension for [VSCode](https://code.visualstudio.com/) that supports development for the Arduino platform)
  * More of a full IDE experience than Arduino IDE
  * Importantly, the ESP Async Webserver libarary is not available in the Arduino IDE any more, but is in PlatformIO!
* Had a problem with AsyncTCP library - turned out that the ESP32 library was being pulled in rather than the ESP8266 version (listed as ESPAsyncTCP)
  * uninstalled the ESP32 version and it built/worked
* The intention is to read data from an InfluxDB instance to show the state of sensor data
  * we could forward the statuses to the ESP via MQTT
  * but it's good to have some user-control over HTTP, so probably an HTTP API will remain on the ESP (not sure if we can combine that with an MQTT listener/subscriber, but we'll give it a go!)


## References Credits:
* Async webserver example code: https://github.com/me-no-dev/ESPAsyncWebServer/tree/master/examples/simple_server
* Basic light ring animation:  https://github.com/JonnyBanana/NEOPIXEL_WS2812_5050_SKETCHS/blob/master/Code/Like_Disco/Like_Disco.ino
