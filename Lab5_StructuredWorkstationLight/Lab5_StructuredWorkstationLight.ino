#include <Arduino.h>

const uint8_t BUTTON_PIN = 23;
const uint8_t POT_PIN = 34;
const uint8_t STATUS_LED_PIN = 18;
const uint8_t PWM_LED_PIN = 19;

bool buttonPressed = false;
bool pwmReady = false;
int rawInput = 0;
int requestedDuty = 0;
int appliedDuty = 0;

void readInputs();
void processInputs();
void updateOutputs();
int scaleToDuty(int raw);          // NEW prototype

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);
  pinMode(PWM_LED_PIN, OUTPUT);
  digitalWrite(PWM_LED_PIN, LOW);
  analogReadResolution(12);
  analogSetPinAttenuation(POT_PIN, ADC_11db);
  pwmReady = ledcAttach(PWM_LED_PIN, 5000, 8);
  if (pwmReady) {
    ledcWrite(PWM_LED_PIN, 0);
  } else {
    Serial.println("PWM setup failed.");
  }
}

void loop() {
  readInputs();
  processInputs();
  updateOutputs();
  delay(20);                       // Pause for 20 ms between loop iterations.
}

// ---------- INPUT ----------
void readInputs() {
  buttonPressed = (digitalRead(BUTTON_PIN) == LOW);
  rawInput = analogRead(POT_PIN);
}

// ---------- PROCESSING ----------
int scaleToDuty(int raw) {         // NEW: scaling calculation lives here
  return (int)constrain(map(raw, 0, 4095, 0, 255), 0L, 255L);
}

void processInputs() {
  requestedDuty = scaleToDuty(rawInput);
  if (buttonPressed && pwmReady) {
    appliedDuty = requestedDuty;
  } else {
    appliedDuty = 0;
  }
}

// ---------- OUTPUT ----------
void updateOutputs() {
  digitalWrite(STATUS_LED_PIN, (buttonPressed && pwmReady) ? HIGH : LOW);
  if (pwmReady) {
    ledcWrite(PWM_LED_PIN, appliedDuty);
  }

  // Serial report every 250 ms for recording results
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 250) {
    lastPrint = millis();
    Serial.printf("button=%d raw=%d requested=%d applied=%d\n",
                  buttonPressed, rawInput, requestedDuty, appliedDuty);
  }
}