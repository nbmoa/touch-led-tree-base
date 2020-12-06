#include <CapacitiveSensor.h>

// This suppress the pragma warning of FastLED (see https://github.com/FastLED/FastLED/issues/797)
#define FASTLED_INTERNAL
#include "FastLED.h"

//#define DEBUG 1

#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.print (x)
 #define DEBUG_PRINTDEC(x)  Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x)
#endif

// WS2812B LED strips are used
#define LED_TYPE WS2812B
// WS2812B LED strips have GRB color order
#define COLOR_ORDER GRB
// TBD what is the range is 64 max?
#define DEFAULT_BRIGHTNESS 64

// TBD this should be nicer
#define MAX_NUM_LEDS    60
#define SENSE1_NUM_LEDS 60
#define SENSE2_NUM_LEDS 60
#define SENSE3_NUM_LEDS 60
#define SENSE4_NUM_LEDS 60

#define PIN_TOUCH_SEND     4
#define SENSE1_PIN_RECEIVE 5
#define SENSE1_PIN_LED     6
#define SENSE2_PIN_RECEIVE 7
#define SENSE2_PIN_LED     8
#define SENSE3_PIN_RECEIVE 9
#define SENSE3_PIN_LED     10
#define SENSE4_PIN_RECEIVE 10
#define SENSE4_PIN_LED     11

#define SENSE_MEASUREMENT_CYCLES 10

#define SENSE_STORED_PER_LED 10000
#define SENSE_STORED_PER_LED_100 100
#define DECREASE_SENSE_SPEED 400
#define INCREASE_SENSE_SPEED 200
#define CYCLE_INTERVAL 40 // 41.666 == 24 update / sec

// lastCycle stores the end of the last cycle, so we can wait
// wait that the next cycle will begin in CYCLE_INTERVAL 
long lastCycle = millis();

CRGB fade(CRGB from, CRGB to, int percent) {
    CRGB result;
    result.r = from.r + ((to.r - from.r) / 100) * percent;
    result.g = from.g + ((to.g - from.g) / 100) * percent;
    result.b = from.b + ((to.b - from.b) / 100) * percent;
    return result;
}

struct TouchLedStrip {
    bool connected;
    const uint8_t sendPin;
    const uint8_t sensePin;
    const uint8_t ledPin;
    const uint8_t numLeds;
    CRGB activeBackColor = CRGB::White;
    CRGB inactiveBackColor = CRGB::Black;
    CRGB runnerColor = CRGB::Blue;

    CapacitiveSensor sensor;
    CRGB             leds[MAX_NUM_LEDS];

    long startTime;
    long accSense;
    int curRunner;

    unsigned long binaryUpdate;
    unsigned long ledUpdate;

    TouchLedStrip(int sendPin, int sensePin, int ledPin, int numLeds) : sendPin(sendPin), sensePin(sensePin), ledPin(ledPin),numLeds(numLeds), sensor(sendPin,sensePin) { }
    void doSetup() { 
        sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
        if ( sense() >= 0 ) {
            connected = true;
        }
    }
    long sense() { return sensor.capacitiveSensor(SENSE_MEASUREMENT_CYCLES); }
    CRGB backColor(int ledIndex) {
        int toggleLed = accSense / SENSE_STORED_PER_LED;
        if (toggleLed > ledIndex) {
            return activeBackColor;
        } else if (toggleLed < ledIndex ){
            return fade(inactiveBackColor, activeBackColor, 2);
        } else {
            int senseRest = accSense % SENSE_STORED_PER_LED;
            if ( senseRest == 0 ) {
            return fade(inactiveBackColor, activeBackColor, 2);
            }
            int colorPercent = senseRest / SENSE_STORED_PER_LED_100;
            return fade(inactiveBackColor, activeBackColor, colorPercent);
        }
    }
    void runCycle() {
        if (connected == false) {
            return;
        }
        // Process the sense event
        int senseVal = sense();
        if ( senseVal < 1000 && senseVal > -1000) {
            curRunner = 0;
            // TBD let is slowly the accSense decrease
            if ( accSense < DECREASE_SENSE_SPEED ) {
                accSense = 0;
            } else {
                accSense -= DECREASE_SENSE_SPEED;
            }
            startTime = 0;
        } else {
            if (curRunner < numLeds * 2) {
              curRunner++;
            } else {
              curRunner = 0;
            }
            curRunner++;
            accSense += INCREASE_SENSE_SPEED;
            if ( startTime == 0 ) {
                startTime = millis();
            } 
        }
        for ( int i = 0; i < numLeds; i++ ) {
            CRGB bg = backColor(i);
            if ( curRunner / 2 == i  || (curRunner / 2) + 1 == i) {
              leds[i] = runnerColor;
            } else {
              leds[i] = bg;              
            }
        }
    }
};


TouchLedStrip touchLedStrips[]{
    TouchLedStrip(PIN_TOUCH_SEND, SENSE1_PIN_RECEIVE, SENSE1_PIN_LED, SENSE1_NUM_LEDS),
    TouchLedStrip(PIN_TOUCH_SEND, SENSE2_PIN_RECEIVE, SENSE2_PIN_LED, SENSE2_NUM_LEDS), 
    TouchLedStrip(PIN_TOUCH_SEND, SENSE3_PIN_RECEIVE, SENSE3_PIN_LED, SENSE3_NUM_LEDS), 
    TouchLedStrip(PIN_TOUCH_SEND, SENSE4_PIN_RECEIVE, SENSE4_PIN_LED, SENSE4_NUM_LEDS) };


void setup()
{
#if DEBUG
    Serial.begin(9600);
#endif

    for (int index = 0; index < sizeof(touchLedStrips)/sizeof(TouchLedStrip); index++) {
        touchLedStrips[index].doSetup();
        if (touchLedStrips[index].connected == true) {
            // Workaround for the template
            switch(touchLedStrips[index].ledPin) {
                case SENSE1_PIN_LED:
                    FastLED.addLeds<LED_TYPE, SENSE1_PIN_LED, COLOR_ORDER>(touchLedStrips[index].leds, touchLedStrips[index].numLeds); 
                    break;
                case SENSE2_PIN_LED:
                    FastLED.addLeds<LED_TYPE, SENSE2_PIN_LED, COLOR_ORDER>(touchLedStrips[index].leds, touchLedStrips[index].numLeds); 
                    break;
                case SENSE3_PIN_LED:
                    FastLED.addLeds<LED_TYPE, SENSE3_PIN_LED, COLOR_ORDER>(touchLedStrips[index].leds, touchLedStrips[index].numLeds); 
                    break;
                case SENSE4_PIN_LED:
                    FastLED.addLeds<LED_TYPE, SENSE4_PIN_LED, COLOR_ORDER>(touchLedStrips[index].leds, touchLedStrips[index].numLeds); 
                    break;
            }
        }
    }
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
}

void loop()
{
    for (int index = 0; index < sizeof(touchLedStrips)/sizeof(TouchLedStrip); index++) {
        touchLedStrips[index].runCycle();
    }
   
    FastLED.show();
    
    long timeRemain = (lastCycle + CYCLE_INTERVAL) - millis();
    if (timeRemain > 0) {
        delay(timeRemain);
    }
    DEBUG_PRINT("timeRemain: ");
    DEBUG_PRINTDEC(timeRemain);
    DEBUG_PRINTLN("");
    lastCycle = millis();
}

