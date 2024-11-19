#define ATMEGA328 1
#define USING_BUTTONS 1
#define USING_POTENTIOMETERS 1

#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
#include <ResponsiveAnalogRead.h>

const int LED_PIN = 6;  

const int N_BUTTONS = 1;
const int BUTTON_ARDUINO_PIN[N_BUTTONS] = { 5 };
int buttonCState[N_BUTTONS] = {};
int buttonPState[N_BUTTONS] = {};
unsigned long lastDebounceTime[N_BUTTONS] = { 0 };
unsigned long debounceDelay = 50;

const int N_POTS = 1;
const int POT_ARDUINO_PIN[N_POTS] = { A0 };
int potCState[N_POTS] = { 0 };
int potPState[N_POTS] = { 0 };
int midiCState[N_POTS] = { 0 };
int midiPState[N_POTS] = { 0 };
const int TIMEOUT = 300;
const int varThreshold = 20;
boolean potMoving = true;
unsigned long PTime[N_POTS] = { 0 };
unsigned long timer[N_POTS] = { 0 };
float snapMultiplier = 0.01;
ResponsiveAnalogRead responsivePot[N_POTS] = {};

byte midiCh = 1;
byte note = 36;
byte cc = 1;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  for (int i = 0; i < N_BUTTONS; i++) {
    pinMode(BUTTON_ARDUINO_PIN[i], INPUT_PULLUP);
  }

  for (int i = 0; i < N_POTS; i++) {
    responsivePot[i] = ResponsiveAnalogRead(0, true, snapMultiplier);
    responsivePot[i].setAnalogResolution(1023);
  }
}

void loop() {
  buttons();
  potentiometers();
}

void buttons() {
  for (int i = 0; i < N_BUTTONS; i++) {
    buttonCState[i] = digitalRead(BUTTON_ARDUINO_PIN[i]);
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (buttonPState[i] != buttonCState[i]) {
        lastDebounceTime[i] = millis();
        if (buttonCState[i] == LOW) {
          MIDI.sendNoteOn(note + i, 127, midiCh);
          digitalWrite(LED_PIN, HIGH); 
        } else {
          MIDI.sendNoteOn(note + i, 0, midiCh);
          digitalWrite(LED_PIN, LOW);  
        }
        buttonPState[i] = buttonCState[i];
      }
    }
  }
}

void potentiometers() {
  for (int i = 0; i < N_POTS; i++) {
    int reading = analogRead(POT_ARDUINO_PIN[i]);
    responsivePot[i].update(reading);
    potCState[i] = responsivePot[i].getValue();
    midiCState[i] = map(potCState[i], 10, 1023, 0, 127);

    int potVar = abs(potCState[i] - potPState[i]);
    if (potVar > varThreshold) PTime[i] = millis();
    timer[i] = millis() - PTime[i];

    potMoving = (timer[i] < TIMEOUT);
    if (potMoving && midiPState[i] != midiCState[i]) {
      MIDI.sendControlChange(cc + i, midiCState[i], midiCh);
      digitalWrite(LED_PIN, HIGH);  
      delay(10);  
      digitalWrite(LED_PIN, LOW);  
      potPState[i] = potCState[i];
      midiPState[i] = midiCState[i];
    }
  }
}
