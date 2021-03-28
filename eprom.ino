String stringEpromRead (int EStart, int Estop){

  String stringFromEprom;
  for (int i = EStart; i < Estop; ++i)
  {
    char ch = char(EEPROM.read(i));
    if (ch != 0 ) {
      stringFromEprom += ch;
    }
  }
  Serial.println(stringFromEprom);
  return stringFromEprom;
}


