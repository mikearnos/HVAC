/* This will send out the error code 34 */

#define LED PD4
#define SHORT 1
#define LONG 0

void setup() {
  pinMode(LED, OUTPUT);
}

void loop() {
  digitalWrite(LED, LOW);
  delay(2250);                        // wait for two seconds

  // three short blinks
  pulse(SHORT);
  pulse(SHORT);
  pulse(SHORT);

  delay(750);

  // four long blinks
  pulse(LONG);
  pulse(LONG);
  pulse(LONG);
  pulse(LONG);
}

void pulse(bool short) {
  digitalWrite(LED, HIGH);

  if (short) {
    delay(250);
  } else {
    delay(1000);
  }

  digitalWrite(LED, LOW);
  delay(250);
}
