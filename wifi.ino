// Создание или подключение к точкам доступа.

void setUpAP() {
  Serial.println();
  Serial.println("Configuring access point...");

  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Serial.print("ssid - ");
  Serial.println(ssid);
  Serial.print("password - ");
  Serial.println(password);
  server.on("/", handleRoot);
  server.begin();
}

void connectToAP2() {

  unsigned char* buf = new unsigned char[100];
  ssidFromEprom.getBytes(buf, 100, 0);

  const char *ssidEprom = (const char*)buf;

  unsigned char* buf2 = new unsigned char[100];
  passFromEprom.getBytes(buf2, 100, 0);

  const char *passEprom = (const char*)buf2;

  WiFi.begin(ssidEprom, passEprom);
  Serial.print(ssidEprom);
  for (int tryCount = 0; tryCount <= 30; tryCount++) {
    digitalWrite(PIN_LED, HIGH);
    delay(500);
    digitalWrite(PIN_LED, LOW);
    Serial.print(".");
    if (WiFi.status() == WL_CONNECTED) {

      tryCount = 0;
      Serial.print("Connected to ");
      Serial.println(ssidEprom);
      break;
    }
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("");
    Serial.println("Can't connect to SSID from EEPROM");
    setUpAP();
  }


}
String SendToServer(String whMethod, String whVName, String whValue) {

  Serial.println("");
  String station, getData, Link;
  String host1 = "";
  host1 = ip1byte;
  host1 += ".";
  host1 += ip2byte;
  host1 += ".";
  host1 += ip3byte;
  host1 += ".";
  host1 += ip4byte;
  char charVar[sizeof(host1)];
  host1.toCharArray(charVar, sizeof(charVar) + 1);
  const char * host = charVar; // ip or dns
  // save the last time you read the sensor

  Serial.println("Send parametrs to server");
  HTTPClient http;    //Declare object of class HTTPClient
  //GET Data


  getData = "?object=ws" + contNameFromEprom;
  getData += "&op=m&m=";
  getData += whMethod;
  getData += "&";
  getData += whVName;
  getData += "=";
  getData += whValue;

  Link = "http://";
  Link += String(host) ;
  Link += "/objects/" + getData;

  http.begin(client,Link);     //Specify request destination

  int httpCode = http.GET();            //Send the request
  String payload = http.getString();    //Get the response payload

  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload

  http.end();  //Close connection

  delay(50);
  return payload;
}
String CreateServerObject(String whVName) {

  Serial.println("");
  String station, getData, Link;
  String host1 = "";
 // String localip = WiFi.localIP();
  host1 = ip1byte;
  host1 += ".";
  host1 += ip2byte;
  host1 += ".";
  host1 += ip3byte;
  host1 += ".";
  host1 += ip4byte;
  char charVar[sizeof(host1)];
  host1.toCharArray(charVar, sizeof(charVar) + 1);
  const char * host = charVar; // ip or dns
  // save the last time you read the sensor

  Serial.println("Create Server Object");
  HTTPClient http;    //Declare object of class HTTPClient


  //http://10.170.1.121/objects/?script=AddNewDevice&id=334455
  getData = "?script=AddNewDevice";
  getData += "&id=";
  getData += whVName;
  //getData += "&ipaddress=";
 // getData += localip
 Serial.println(whVName);
 
  Link = "http://";
  Link += String(host) ;
  Link += "/objects/" + getData;
 Serial.println(Link);
  http.begin(client,Link);     //Specify request destination

  int httpCode = http.GET();            //Send the request
  String payload = http.getString();    //Get the response payload

  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload

  http.end();  //Close connection

  delay(50);
  return payload;
}
void wifiscan() {
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");

  // Wait a bit before scanning again
  delay(5000);
}

void connectedBlink() {

  for (int blCount = 0; blCount <= 20; blCount++) {
    digitalWrite(PIN_LED, HIGH);
    delay(10);
    digitalWrite(PIN_LED, LOW);
    delay(10);
  }
  digitalWrite(PIN_LED, HIGH);
  delay(1000);
  digitalWrite(PIN_LED, LOW);
}

void readSensorBlink() {

  for (int blCount = 0; blCount <= 5; blCount++) {
    digitalWrite(PIN_LED, HIGH);
    delay(100);
    digitalWrite(PIN_LED, LOW);
    delay(100);
  }
  digitalWrite(PIN_LED, HIGH);
  delay(500);
  digitalWrite(PIN_LED, LOW);
}
void testOrCreateObject() {
  
  String ansver;
  ansver = SendToServer("keepalive", "alive", "1"); // отправим сообщение что мы живы  посмотрим ответ.
  Serial.println("Send Alive signal");
  Serial.println(ansver);
  if (ansver.toInt() == 1) {
    Serial.println("Sensor's object is created. Alive updated");
  } else {
    ansver = CreateServerObject(contNameFromEprom);
    Serial.println("Send Create Server Object");
    Serial.println(ansver);
  }
  String ipaddress = WiFi.localIP().toString();
  SendToServer("setIpAddress", "ipaddress", ipaddress); // Для влажности
  
}
