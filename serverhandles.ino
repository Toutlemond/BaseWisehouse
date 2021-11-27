////////////////// Тут описываем разные обработчики событий сервера////////////
void handleRoot() {


  htmlText = "";
  contentText = "";
  contentText = F("<h4>Главная страница контроллера - \n");
  contentText += contNameFromEprom;
  contentText += F("</h4>\n");
  contentText += "<hr>\n";
  contentText += "<p>Здравствуйте! Вы попали на страницу контроллера WiseHouse.</p>\n";
  contentText += "<p>Вы можете изменить настройки подключения к вашей сети WIFI, \n";
  contentText += "назначить имя этому контроллеру для показа его в мобильном приложении\n";
  contentText += "а также изменить настройки сервера умного дома(не рекомендуется)\n";
  contentText += "</p>";
  contentText += "<hr>\n";
  contentText += "<p><strong>Время до перезагрузки- ";
  contentText += resetMinute;
  contentText += " мин</strong></p>";
    contentText += "<p><strong>Uptime системы- ";
  contentText += uptime;
  contentText += " мин</strong></p>";



  // Скрипт ответстевенный за обновление страницы пока выключим
  /* contentText += "<script type='text/javascript'>\n";
    contentText += "function pageReload() { location.reload(); }\n";
    contentText += "setInterval(\"pageReload()\", 5000);\n";
    contentText += "</script>\n";
  */

  htmlText = baseText;
  htmlText += contentText;
  htmlText += endText;

  server.send(200, "text/html", htmlText);
}

void handleClick(){
  contNumber = server.arg("contNumber");
  htmlText = "";
  contentText = "";
  contentText = "<h4>Кликнуть контакт</h4>\n";
  contentText += "<hr>\n";
  contentText += "<p>Номер контакта - \n";
  contentText += contNumber;
  contentText += "</p>\n";

  htmlText = baseText;
  htmlText += contentText;
  htmlText += endText;
  
   digitalWrite(contNumber.toInt(), LOW);
    delay(1000);
    digitalWrite(contNumber.toInt(), HIGH);
    //Serial.print("Clicked");
  server.send(200, "text/html", htmlText);
  }
void handleOn() {
  contNumber = server.arg("contNumber");
  htmlText = "";
  contentText = "";
  contentText = "<h4>Включить контакт</h4>\n";
  contentText += "<hr>\n";
  contentText += "<p>Номер контакта - \n";
  contentText += contNumber;
  contentText += "</p>\n";

  htmlText = baseText;
  htmlText += contentText;
  htmlText += endText;
  digitalWrite(contNumber.toInt(), LOW);
  server.send(200, "text/html", htmlText);
}
void handleOff() {
  htmlText = "";
  contentText = "";
  contentText = "<h4>Выключить контакт</h4>\n";
  contentText += "<hr>\n";
  htmlText = baseText;
  htmlText += contentText;
  htmlText += endText;
  digitalWrite(contNumber.toInt(), HIGH);
  server.send(200, "text/html", htmlText);
}
void handleContacts() {
  htmlText = "";
  contentText = "";
  contentText = "<h4>Контакты</h4>\n";
  contentText += "<hr>\n";
  contentText += "<h5>ООО \"СИМПЛ\"</h5>\n";
  contentText += "<p>Беспалов Вадим</p>\n";
  contentText += "<p>Глущенко Илья</p>\n";
  htmlText = baseText;
  htmlText += contentText;
  htmlText += endText;

  server.send(200, "text/html", htmlText);
}
void handleConfig() {
  ip1byte = EEPROM.read(21);
  ip2byte = EEPROM.read(22);
  ip3byte = EEPROM.read(23);
  ip4byte = EEPROM.read(24);
  wifiscan();
  htmlText = "";
  contentText = "";
  contentText = F("<form method=\"GET\" action=\"/setsrvip\">\n");
  contentText += F("<h4>Настройки контроллера</h4>\n");
  contentText += F("<hr>\n");
  contentText += F("<p>ip адрес сервера: <span class=\"bold\">\n");
  contentText += ip1byte;
  contentText += F(".");
  contentText += ip2byte;
  contentText += F(".");
  contentText += ip3byte;
  contentText += F(".");
  contentText += ip4byte;
  contentText += F("</span></p>\n");
  contentText += F("<p>Новый ip адрес сервера: <span class=\"bold\"><input style=\"width: 22px;\" type=\"text\" name=\"srvip1\">.\n");
  contentText += F("<input style=\"width: 22px;\" type=\"text\" name=\"srvip2\">.\n");
  contentText += F("<input style=\"width: 22px;\" type=\"text\" name=\"srvip3\">.\n");
  contentText += F("<input style=\"width: 22px;\" type=\"text\" name=\"srvip4\"></span></p>\n");
  contentText += F("</span></p>\n");
  contentText += F("<p><span class=\"bold\">Не используйте 0 (ноль) в названии точки доступа и пароле</span></p>\n");
  int n = WiFi.scanNetworks();

  if (n == 0) {
    contentText += F("<p>Ваша сеть WIFI________ <span class=\"bold\"><input type=\"text\" name=\"ssid\" value=\"");
    contentText += ssidFromEprom;
    contentText += F("\"></span></p>\n");
  } else {
    contentText += F("<p>Ваша сеть WIFI________ <span class=\"bold\">");
    contentText += F("<select  name=\"ssid\" >");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      contentText += F("<option  value = \"");
      Serial.print(i + 1);
      Serial.print(": ");
      contentText += WiFi.SSID(i);
      contentText += F("\">");
      contentText += WiFi.SSID(i);
      contentText += ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "</option> " : "*</option>");
      delay(10);
     
    }
   contentText += F("<select></span></p>\n");
  }

  contentText += F("<p>Пароль сети___________ <span class=\"bold\"><input type=\"text\" name=\"pass\" value=\"");
  contentText += passFromEprom;
  contentText += F("\"></span></p>\n");
  contentText += F("<p>Имя контроллера_______ <span class=\"bold\"> <input type=\"text\" name=\"contName\" value=\"");
  contentText += contNameFromEprom;
  contentText += F("\">Рекомендуем  - ");

  contentText += randNumber;
  contentText += F("</span></p>\n");
  contentText += F("<p>Прямое Управление <span class=\"bold\"><select  name=\"directcont\"><option value=\"0\" selected>Нет</option><option value=\"1\" >Да</option></select>");
  contentText += F("</span></p>\n");

  // Далее завершение формы
  contentText += F("<input type=\"submit\" value=\"Сохранить\">\n");
  contentText += F("</form>\n");

  htmlText = baseText;
  htmlText += contentText;
  htmlText += endText;

  server.send(200, "text/html", htmlText);
}

void handleSetSrv() {

  String srvip1 ;
  String srvip2 ;
  String srvip3 ;
  String srvip4 ;
  String ipbyte;

  Serial.println("Setting IP address");
  // Serial.println(server.argName(0));
  srvip1 = server.arg(0);

  if (srvip1.length() > 0) {
    srvip1 = server.arg(0);
    srvip2 = server.arg(1);
    srvip3 = server.arg(2);
    srvip4 = server.arg(3);

    Serial.print("srvip1:");
    Serial.println(srvip1);
    Serial.print("srvip2:");
    Serial.println(srvip2);
    Serial.print("srvip3:");
    Serial.println(srvip3);
    Serial.print("srvip4:");
    Serial.println(srvip4);

    EEPROM.write(21, srvip1.toInt());
    EEPROM.write(22, srvip2.toInt());
    EEPROM.write(23, srvip3.toInt());
    EEPROM.write(24, srvip4.toInt());
    //запишем флаг конфига
    EEPROM.write(20, 1);
    //И сохраним все это
    EEPROM.commit();
    // МЫ все сохранили а теперь снова считаем
    ip1byte = EEPROM.read(21);
    ip2byte = EEPROM.read(22);
    ip3byte = EEPROM.read(23);
    ip4byte = EEPROM.read(24);
    String pathGet = "/objects/?object=is74sens1&op=m&m=testValueChange&testValue=1";
    String fullPath = "";
    fullPath = "http://";
    fullPath += ip1byte;
    fullPath += ".";
    fullPath += ip2byte;
    fullPath += ".";
    fullPath += ip3byte;
    fullPath += ".";
    fullPath += ip4byte;
    fullPath += pathGet;

    htmlText = "";
    contentText = "";
    contentText = "<h4>Настройки контроллера сохранены!</h4>\n";
    contentText += "<a target=\"_blank\" href=\"\n";
    contentText += fullPath;
    contentText += "\">Проверить соединение с сервером(откроется в отдельном окне)</a>\n";
  }



  String qsid = server.arg("ssid");
  String qpass = server.arg("pass");
  String contName = server.arg("contName");
  String directcont = server.arg("directcont");

  if (qsid.length() > 0 && qpass.length() > 0) {
    Serial.println("clearing eeprom");
    for (int i = 100; i < 150; ++i) {
      EEPROM.write(i, 0);
    }
    Serial.println(qsid);
    Serial.println("");
    Serial.println(qpass);
    Serial.println("");

    Serial.println("writing eeprom ssid:");
    for (int i = 0; i < qsid.length(); ++i)
    {
      EEPROM.write(100 + i, qsid[i]);
      Serial.print("Wrote: ");
      Serial.println(qsid[i]);
    }
    Serial.println("writing eeprom pass:");
    for (int i = 0; i < qpass.length(); ++i)
    {
      EEPROM.write(125 + i, qpass[i]);
      Serial.print("Wrote: ");
      Serial.println(qpass[i]);
    }
    EEPROM.commit();


  }
  if (contName.length() > 0) {
    Serial.println("clearing eeprom");
    for (int i = 150; i < 170; ++i) {
      EEPROM.write(i, 0);
    }
    Serial.println(contName);
    Serial.println("");
    Serial.println("writing eeprom Controller Name:");
    for (int i = 0; i < contName.length(); ++i)
    {
      EEPROM.write(150 + i, contName[i]);
      Serial.print("Wrote: ");
      Serial.println(contName[i]);
    }

    EEPROM.commit();
  }

  //Прощьем ключ что конфиг записан
  EEPROM.write(193, directcont.toInt());
  EEPROM.commit();
  //Прощьем ключ что конфиг записан
  EEPROM.write(10, 1);
  EEPROM.commit();

  htmlText = baseText;
  htmlText += contentText;
  htmlText += endText;

  server.send(200, "text/html", htmlText);


}

void handleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/html", message);
}
