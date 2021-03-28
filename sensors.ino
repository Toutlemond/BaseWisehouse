/*
   Функция термостата, Берет значения текущей темпаратуры и уставку
   Сравнивает и если не достаточно жарко включает реле
   Уставка может быть ызята как памяти так и с крутилки
   Текущая температура тоже может браться из разных мест

*/
void termostat() {

  ///Напишем работу с уставкой из сети
  setTemp = "";
  for (int i = 185; i < 190; ++i)
  {
    char ch2 = char(EEPROM.read(i));
    if (ch2 != 0 ) {
      setTemp += ch2;
    }
  }
  if (setTemp.toInt() == 0) {
    samples = analogRead(REOSTATPIN);
    tempUstavka = (samples / temtStep) + minTemp.toInt();
    flagTempset = 0;
    digitalWrite(GREENOPTRON, LOW);

  } else {
    flagTempset = 1;
    tempUstavka = setTemp.toInt();
    digitalWrite(GREENOPTRON, HIGH);
  }


  sensors.requestTemperatures(); // Send the command to get temperatures
  digiTemp1 = sensors.getTempCByIndex(0);

   // humidity = dht.readHumidity();
   // digiTemp1 = dht.readTemperature();



  if (digiTemp1 >= tempUstavka + deltaTemp.toInt()) {
    digitalWrite(OPTRON, LOW);
    heatText = "Chill";


  } else if (digiTemp1 <= tempUstavka - deltaTemp.toInt()) {
    digitalWrite(OPTRON, HIGH);
    heatText = "Fire!!!";
  }


}

void checkTempSensor() {
  //Если используется DS датчики
  
      sensors.requestTemperatures(); // Send the command to get temperatures
    digiTemp1 = sensors.getTempCByIndex(0);
       Serial.print("Temperature for the device 1 (index 0) is: ");
      Serial.println(digiTemp1);
  

  //Если используются DHT датчики
 // humidity = dht.readHumidity();
  //digiTemp1 = dht.readTemperature();

}


void DHTtoSerial() {
  // Читаем показание датчика
  webString = "Temperature: " + String((int)digiTemp1) + " C ";
  //  webString += "Humidity: " + String((int)humidity) + "%";
  Serial.println(webString);
}
/*

  void gettemperature() {

  // Reading temperature for humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  humidity = dht.readHumidity();          // Read humidity (percent)
  temp_f = dht.readTemperature();     // Read temperature as Fahrenheit
  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temp_f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  }
*/

/*void checkSinkSensor() {
  //Проверка датчика затопления
  int sinkVal;
  sinkVal = digitalRead(SinkPin);

  if (sinkVal == 1) {
    Serial.println("Sink - 1");
    sinkText = "ДА!!!";
  } else {
    //Serial.println("Sink - 0");
    sinkText = "Нет";
  }
  }
*/

/*
  void checkGasSensor() {
  //Проверка датчика затопления
  int gasVal;
  gasVal = digitalRead(gasPin);

  if (gasVal == 0) {
    Serial.println("gas - 1");
    gasText = "ДА!!!";
  } else {
    Serial.println("gas - 0");
    gasText = "Нет";
  }
  }
*/


/*
  void checkPirSensor() {

  /*
    if (oldmotion == 0) {
    Serial.println("Motion detected");
    // server.stop();
    //WiFiClient client;
    if (!client.connect(host, 80)) {
    Serial.println("Connection failed :-(");
    }
    Serial.println("Connected to host - sending request...");
    motion = 1;
    String url = "";
    //url += host;
    url += "/objects/?object=is74sens1&op=m&m=motionDetected&motion=";
    url += motion;
    Serial.println("Motion changed  - send Server request");
    Serial.print(host);
    Serial.println(url);
    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Connection: close\r\n\r\n");
    delay(50);
    oldmotion = 1;
    //  server.begin();
    }


    }
*/
