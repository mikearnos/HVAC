/* This will send out ON/OFF, and error codes */

#define LED PD4
#define SHORT 1
#define LONG 0

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  sendCode(34);
  sendCode(34);
  sendCode(34);
  sendCode(47);
  sendCode(47);
  sendCode(47);

  sendHigh(10000);                   // high for ten seconds "ON"

  sendCode(34);
  sendCode(34);
  sendCode(34);
  sendCode(47);
  sendCode(47);
  sendCode(47);

  sendLow(10000);                    // low for ten seconds "OFF"
}

void sendHigh(int duration) {
  digitalWrite(LED, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(duration);
}

void sendLow(int duration) {
  digitalWrite(LED, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  delay(duration);
}

void sendCode(int code) {
  sendLow(2250);                      // begin code is low for 2250ms

  for (int i = code / 10; i > 0; i--) {
    pulse(SHORT);                     // send tens as short pulses
  }

  delay(750);                         // pause between pulses

  for (int i = code - ((code / 10) * 10); i > 0; i--) {
    pulse(LONG);                      // send ones as long pulses
  }
}

void pulse(bool shortPulse) {
  digitalWrite(LED, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);

  if (shortPulse) {
    delay(250);                       // short pulse is high 250ms
  } else {
    delay(1000);                      // long pulse is high 1000ms
  }

  digitalWrite(LED, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
}
