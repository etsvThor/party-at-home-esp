/*
 *
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

FASTLED_USING_NAMESPACE

#define NUM_LEDS 60 //change to number of pixels
#define FRAMES_PER_SECOND 120
WebSocketsClient webSocket;
CRGBArray<NUM_LEDS> leds;

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0;				   // rotating "base color" used by many of the patterns

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
void red()
{
	leds(0, NUM_LEDS - 1) = CRGB::Red;
}

void blue() {
	leds(0, NUM_LEDS - 1) = CRGB::Blue;
}

void green()
{
	leds(0, NUM_LEDS - 1) = CRGB::Green;
}
void Rainbow()
{
	// FastLED's built-in rainbow generator
	fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void addGlitter(fract8 chanceOfGlitter)
{
	if (random8() < chanceOfGlitter)
	{
		leds[random16(NUM_LEDS)] += CRGB::White;
	}
}

void rainbowWithGlitter()
{
	// built-in FastLED rainbow, plus some random sparkly glitter
	Rainbow();
	addGlitter(80);
}

void confetti()
{
	// random colored speckles that blink in and fade smoothly
	fadeToBlackBy(leds, NUM_LEDS, 10);
	int pos = random16(NUM_LEDS);
	leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon()
{
	// a colored dot sweeping back and forth, with fading trails
	fadeToBlackBy(leds, NUM_LEDS, 20);
	int pos = beatsin16(13, 0, NUM_LEDS - 1);
	leds[pos] += CHSV(gHue, 255, 192);
}

void bpm()
{
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)
	uint8_t BeatsPerMinute = 62;
	CRGBPalette16 palette = PartyColors_p;
	uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++)
	{ //9948
		leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
	}
}

void juggle()
{
	// eight colored dots, weaving in and out of sync with each other
	fadeToBlackBy(leds, NUM_LEDS, 20);
	byte dothue = 0;
	for (int i = 0; i < 8; i++)
	{
		leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
		dothue += 32;
	}
}
SimplePatternList gPatterns = {red, blue, green, Rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm};

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
	// add one to the current pattern number, and wrap around at the end
	gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns); 
	if(gCurrentPatternNumber < 3)
		gCurrentPatternNumber = 3;
}

#define USE_SERIAL Serial
bool pattern = 0;
bool colorchord = 0;
bool autoRotate = 0;
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{

	switch (type)
	{
	case WStype_DISCONNECTED:
		USE_SERIAL.printf("[WSc] Disconnected!\n");
		break;
	case WStype_CONNECTED:
	{
		USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);

		// send message to server when Connected
		//webSocket.sendTXT("Connected");
	}
	break;
	case WStype_TEXT:
		if (colorchord == 1)
		{
			if (strcmp("colorchord_off", (const char *)payload) == 0)
			{
				colorchord = 0;
				USE_SERIAL.printf("Colorchord mode deactivated!");
			}
			else
			{
				char *rgbString;
				int r, g, b;
				rgbString = strtok(((char *)payload), " ,.-");
				int start = millis();
				for (int l = 0; l < NUM_LEDS; l++)
				{
					for (int i = 0; i < 4; i++)
					{
						//USE_SERIAL.printf("%s\n", rgbString);
						switch (i)
						{
						case 0:
							r = atoi(rgbString);
							break;
						case 1:
							g = atoi(rgbString);
							break;
						case 2:
							b = atoi(rgbString);
							break;
						}
						rgbString = strtok(NULL, " ,.-");
					}
					leds[l].setRGB(r, g, b);
				}
				//USE_SERIAL.printf("Time:%ld\n", millis() - start);//debug time needed to calculate new led array
				FastLED.show();
			}
		}
		else
		{
			USE_SERIAL.printf("[WSc] get text: %s\n", payload);
			if (strcmp("colorchord", (const char *)payload) == 0)
			{
                pattern = 0;
				colorchord = 1;
				USE_SERIAL.printf("Colorchord mode activated!");
			}
			if (strcmp("red", (const char *)payload) == 0)
			{
                colorchord = 0;
                pattern=1;
				gCurrentPatternNumber = 0;
			}
			if (strcmp("blue", (const char *)payload) == 0)
			{
                colorchord = 0;
                pattern=1;
				gCurrentPatternNumber = 1;
			}
			if (strcmp("green", (const char *)payload) == 0)
			{
                colorchord = 0;
                pattern=1;
				gCurrentPatternNumber = 2;
			}
			if (strcmp("rainbow", (const char *)payload) == 0)
			{
                colorchord = 0;
                pattern=1;
				gCurrentPatternNumber = 3;
			}
			if (strcmp("pattern_off", (const char *)payload) == 0)
			{
                colorchord = 0;
				pattern = 0;
			}
			if (strcmp("next", (const char *)payload) == 0)
			{
                colorchord = 0;
                pattern=1;
				nextPattern();
			}
			if (strcmp("auto", (const char *)payload) == 0)
			{
                colorchord = 0;
                pattern = 1;
				autoRotate = true;
			}
			if (strcmp("man", (const char *)payload) == 0)
			{
                colorchord = 0;
                pattern = 1;
				autoRotate = false;
			}
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

void setup()
{
	USE_SERIAL.begin(115200);
	//initialize ledstrip on D2 of NodeMCU and make strip red
	FastLED.addLeds<NEOPIXEL, 4>(leds, NUM_LEDS);
	leds(0, NUM_LEDS - 1) = CRGB::Red;
	FastLED.show();

	USE_SERIAL.setDebugOutput(true);

	USE_SERIAL.println();
	USE_SERIAL.println();
	USE_SERIAL.println();

	for (uint8_t t = 4; t > 0; t--)
	{
		USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
		USE_SERIAL.flush();
		delay(1000);
	}
	WiFiManager wifiManager;
	wifiManager.autoConnect("ThorpartyAP");

	// server address, port and URL
	webSocket.beginSSL("party-at-home.thor.edu", 443, "/ws/chat/thorparty/");

	// event handler
	webSocket.onEvent(webSocketEvent);

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);
}

void loop()
{
	webSocket.loop();
    if(pattern){
        gPatterns[gCurrentPatternNumber]();
        if(gCurrentPatternNumber >2) {
            EVERY_N_MILLISECONDS(20) { gHue++; }   // slowly cycle the "base color" t
        }
        if (autoRotate)
        {
            EVERY_N_SECONDS(10) { nextPattern(); } // change patterns periodically
        }
        // send the 'leds' array out to the actual LED strip
        FastLED.show();
        // insert a delay to keep the framerate modest
        FastLED.delay(1000 / FRAMES_PER_SECOND);
    }
}
