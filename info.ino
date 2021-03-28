void debuginfo() {
  Serial.println("Start To Check Sensors");
  Serial.print("Clerar Value - ");
  Serial.println(samples);
  Serial.print("Temp from - ");
  if (flagTempset == 0) {
    Serial.println("device.");
  } else {
    Serial.println("Net.");
  }
  Serial.println(tempUstavka);
  Serial.print("Temp set - ");
  Serial.println(tempUstavka);
  Serial.print("Delta Temp - ");
  Serial.println(deltaTemp.toInt());
  Serial.print("Temperature for the device 1 (index 0) is: ");
  Serial.println(digiTemp1);
  Serial.println(heatText);
  Serial.println("");
  Serial.print("Until reset  - ");
  Serial.print(resetMinute);
  Serial.println(" min");

  Serial.print(" millis  - ");
  Serial.println(currentMillis);
}
