/*
 * WebSocketClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
//WifiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>   

#include <WebSocketsClient.h>

#include <Hash.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

#define NUM_LEDS 8 //change to number of pixels

WebSocketsClient webSocket;
CRGBArray<NUM_LEDS> leds;

#define USE_SERIAL Serial

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

	switch(type) {
		case WStype_DISCONNECTED:
			USE_SERIAL.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED: {
			USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);

			// send message to server when Connected
			//webSocket.sendTXT("Connected");
		}
			break;
		case WStype_TEXT:
			USE_SERIAL.printf("[WSc] get text: %s\n", payload);
			if (strcmp("green", (const char *)payload) == 0) {
				leds(0,NUM_LEDS-1)=CRGB::Green;
				FastLED.show();
			}
			if (strcmp("red", (const char *)payload) == 0) {
				leds(0,NUM_LEDS-1)=CRGB::Red;
				FastLED.show();
			}
			if (strcmp("blue", (const char *)payload) == 0) {
				leds(0,NUM_LEDS-1)=CRGB::Blue;
				FastLED.show();
			}
			// send message to server
			// webSocket.sendTXT("message here");
			break;
		case WStype_BIN:
			USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
			hexdump(payload, length);

			// send data to server
			// webSocket.sendBIN(payload, length);
			break;
        case WStype_PING:
            // pong will be send automatically
            USE_SERIAL.printf("[WSc] get ping\n");
            break;
        case WStype_PONG:
            // answer to a ping we send
            USE_SERIAL.printf("[WSc] get pong\n");
            break;
    }

}

void setup() {
	USE_SERIAL.begin(115200);
    //initialize ledstrip on D2 of NodeMCU and make strip red
	FastLED.addLeds<NEOPIXEL,4>(leds, NUM_LEDS);
	leds(0,NUM_LEDS-1)=CRGB::Red;
	FastLED.show();

	USE_SERIAL.setDebugOutput(true);

	USE_SERIAL.println();
	USE_SERIAL.println();
	USE_SERIAL.println();

	for(uint8_t t = 4; t > 0; t--) {
		USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
		USE_SERIAL.flush();
		delay(1000);
	}
    WiFiManager wifiManager;
    wifiManager.autoConnect("ThorpartyAP");

	// server address, port and URL
	webSocket.begin("wss://party-at-home.thor.edu", 443 , "/ws/chat/henk/");

	// event handler
	webSocket.onEvent(webSocketEvent);

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);

}

void loop() {
	webSocket.loop();
}
