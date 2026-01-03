void run10TimesPerSecond() {

}
void runOncePerSecond() {

}
void runOncePerTwoSecond() {

}
void runOncePerMinute() {

}

void processBuildInTimer() {
  if (TimeOut > 0 ) {
    if (isPinOn != 1) {
      isPinOn = 1;
      Serial.println("timer On");
      digitalWrite(PIN_RELAY, HIGH);
      digitalWrite(PIN_LED, HIGH);
    }
    TimeOut = TimeOut - interval3;
    Serial.println(TimeOut);
  } else {
    if (isPinOn != 0) {
      TimeOut = 0;
      isPinOn = 0;
      digitalWrite(PIN_RELAY, LOW);
      digitalWrite(PIN_LED, LOW);
      Serial.println("timer off");
    }
  }
}
