//тут все методы работы с музыкой
void run() {
  static int lastms = 0;
  if (mp3->isRunning()) {
    if (millis() - lastms > 1000) {
      lastms = millis();

      Serial.printf("Running for %d sec...\n", lastms / 1000);
      Serial.flush();
    }
    if (!mp3->loop()) mp3->stop();
  } else {
    Serial.printf("Pause \n");
    delay(1000);
    play();
    pauseCounter++;
    if (pauseCounter > 3 ) {
      Serial.printf("Somthing goes wrong... \n");
      nextStation();
      pauseCounter = 0 ;
    }

  }
}
void play()
{
  out = new AudioOutputI2SNoDAC();
  file = new AudioFileSourceICYStream(myStrings[currentStation]);
  file->RegisterMetadataCB(MDCallback, (void*)"ICY");
  buff = new AudioFileSourceBuffer(file, 6144); //1024, 2048,4096 ,8192, 16384
  buff->RegisterStatusCB(StatusCallback, (void*)"buffer");
  mp3 = new AudioGeneratorMP3();
  file->SetReconnect(3, 500); // это может помешать
  //mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");
  mp3->begin(buff, out);
  playingStatus = 1;
  Serial.print("Play");

}

void nextStation()
{
  Serial.print("playingStatus");
  Serial.println(playingStatus);

  playingStatus = 0;
  mp3->stop();
  if (buff) {
    buff->close();
    buff = NULL;
    delete buff;
  }

  currentStation++;
  if (currentStation > 3) {
    currentStation = 0; // переделай на длинну массива!
  }
  Serial.printf("currentStation ID is %d ...\n", currentStation);
  Serial.printf("New Station is %s ...\n", myStrings[currentStation]);
  Serial.flush();
  playingStatus = 1;
}


void prevStation()
{
  Serial.print("playingStatus - ");
  Serial.println(playingStatus);
  playingStatus = 0;
  mp3->stop();
  if (buff) {
    buff->close();
    buff = NULL;
    delete buff;
  }
  currentStation--;
  if (currentStation < 0) {
    currentStation = 3; // переделай на длинну массива!
  }
  Serial.printf("currentStation ID is %d ...\n", currentStation);
  Serial.printf("New Station is %s ...\n", myStrings[currentStation]);
  Serial.flush();
  playingStatus = 1;
}


// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  (void) isUnicode; // Punt this ball for now
  // Note that the type and string may be in PROGMEM, so copy them to RAM for printf
  char s1[32], s2[64];
  strncpy_P(s1, type, sizeof(s1));
  s1[sizeof(s1) - 1] = 0;
  strncpy_P(s2, string, sizeof(s2));
  s2[sizeof(s2) - 1] = 0;
  Serial.printf("METADATA(%s) '%s' = '%s'\n", ptr, s1, s2);
  Serial.flush();
}

// Called when there's a warning or error (like a buffer underflow or decode hiccup)
void StatusCallback(void *cbData, int code, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  // Note that the string may be in PROGMEM, so copy it to RAM for printf
  char s1[64];
  strncpy_P(s1, string, sizeof(s1));
  s1[sizeof(s1) - 1] = 0;
  Serial.printf("STATUS(%s) '%d' = '%s'\n", ptr, code, s1);
  Serial.flush();
}
