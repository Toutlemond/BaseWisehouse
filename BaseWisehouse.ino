//// Прошивка для розеток sonoff
///  14.10.2016
///  16.10.2017 - добавил выравнивание в HTML
//   15.10.2018 - Добавлена функция перезагрузки каждые 500 000 мс
//   22.10.2018 - Код разделен на разные файлы
//   05.09.2019 - Сделана как базовая. Убраны некоторые вещи.
//   24.09.2020 - Исправлен метод отправки на сервер
//   03.11.2020 - Добавим поддержку 433 мГц устройств

// Version 0.8.1

/// Задачи -


/// 2. WiFI
/// 2.1 Подключаться к точке доступа если, настроек нет, создавать свою точку доступа.
/// 2.2 Передавать значения и состояние датчиков GET запросом на сервер

/// Памятка для сборки Платы в микро размере 45х45
///  - 13-ю лапу подтягиваем только 4.7К. R4 не распаиваем
///  - R2 можно не паять если не планируется дип слип
///  - R15 20К - не 10!!! При условии что реостат на 10 к.

/// Адрессация епрома
//10 - ConfigFlag - Если есть то конфиг писался
//21,22,23,24 - ServerIP
//125-149 ssidFromEprom
//150-170 contNameFromEprom
//171-175 deltaTemp
//176-180 maxTemp
//182-185 minTemp
//185-190 setTemp
//191 - OnOffFlag
//193 - DirectControll - Прямое управление контактом минуя термостат. Или это розетка или управляется с сторонней системы
/*
  Примерно прикинем что нам нужно 10 символов на SSID и 10 на пароль
  возбмем 20 байт епрома как ключ!
  20 - ключ - если 0 или 255 то значит открываем точку доступа, если 1 то коннектимся к нашей
  30-40 - номер датчика
  100-150 - имя точки 10 символов не более.
  151-200 - пароль латиницей

*/

#define RCSWITCHON 1
#define ONEWIREON 1
#define DHTON 0

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>
//#include <DHT.h>

#include "button.h"


///////////////////////////////////НАЗНАЧЕНИЕ НОЖЕК ////////////////////////////////////////////////////

int PIN_RELAY = 12;
int PIN_LED = 13;
int PIN_BUTTON = 0;

Button btn1(PIN_BUTTON);

// Имя точки доступа в случае если не подключилось к сети пользователя
const char *ssid = "WiseRozetka1";
const char *password = "12345678";

const int configBite = 20;
int tryCount = 0;
int TempModul = 0; //

/// Далее 21,22,23,24 байты это будет адрес сервера.
String ip1byte = "";
String ip2byte = "";
String ip3byte = "";
String ip4byte = "";

char* functions[] = {"tempChanged", "humidityChange"};
char* values[] = {"temp", "humidity "};


///////////////////////////////////Подготовка базовых текстов HTML//////////////////////////////////////////
String additionMsg = "";
// Базовый контент ниже ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//String baseText = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
String baseText;
String contentText;
String endText;
String htmlText;
String sinkText = "Нет";
String gasText = "Нет";
String heatText = "Охлаждаем";
String webString = "";                    // Строка для отображение
unsigned long currentMillis;               // Сами секунды
unsigned long previousMillis = 0;         // когда считано последнее значение
unsigned long previousMillis2 = 0;        // когда считано последнее значение2
unsigned long previousMillisForSend = 0;  // когда считано последнее значение для отправки запроса
unsigned long previous10TimesPerSecond = 0;
unsigned long previousOncePerSecond = 0;
unsigned long previousOncePerTwoSecond = 0;
unsigned long previousOncePerMinute = 0;
unsigned long previousOncePerHour = 0;
const long interval = 2000;               // как часто читать сенсор
const long interval2 = 10000;             // Время через которое пора спать
const long interval3 = 1000;              // Как часто обрабатывать WEB-запросы
const long intervalForSend = 60000;              // Как часто обрабатывать WEB-запросы
String contNumber;
int DirectControll = 0;
int NormalMode = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int counterTemp = 0;
boolean iSleep = 0;
const int deepSlp = 0;                    // Включать ли функцию Глубокого сна
int resetDelay = 1800000;
int resetMinute = 0;
int uptime = 0;
//////////////////////////////////////////////////////////////////////////////
int ee = 0;                               // Адрес чтения из епром - тест
int eeStart = 100;
byte eevalue;

String ssidFromEprom;
String passFromEprom;
String contNameFromEprom;

int numFromEprom;

long int randNumber = random(100000, 999999);


ESP8266WebServer server(80);
WiFiClient client;

// Для перезагрузки
void(* resetFunc) (void) = 0;

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();


pinMode(PIN_LED, OUTPUT);
pinMode(PIN_RELAY, OUTPUT);
pinMode(PIN_BUTTON, INPUT);

digitalWrite(PIN_RELAY, HIGH);
digitalWrite(PIN_LED, LOW);

  /////Переменную мы определили выше а тут напишем в нее Все наполнение!////////////////////////////////////////////////////////////////////

  baseText = F("<!DOCTYPE html><style>html{\n");
  baseText += F("background: #cee2e1; /* Old browsers */\n");
  baseText += F("background: -webkit-linear-gradient(top,  #cee2e1 0%,#e3efe5 40%,#e3efe5 59%,#f9f5e9 100%); \n");
  baseText += F("background: linear-gradient(to bottom,  #cee2e1 0%,#e3efe5 40%,#e3efe5 59%,#f9f5e9 100%); \n");
  baseText += F("background-repeat:  no-repeat;\n");
  baseText += F("background-size:  cover;\n");
  baseText += F("font-family: Verdana,Helvetica,Sans;color: #666;}\n");
  baseText += F("a{text-decoration: none; color: #666; }.bold{color:  #000;}\n");
  baseText += F("a:visited{color: #663A00;}.base{ margin: 0 auto;}.header{height: 160px;}\n");
  baseText += F(".logo{float: left;  font-size: 22px;}.menu{float:right;color: #666; margin-top: 56px}\n");
  baseText += F(".content{border:  #666 solid 1px;border-radius: 6px;padding: 6px;background-color:  #f9f5e9;}\n");
  baseText += F(".inset {color: #666;text-shadow: -1px -1px 1px #000, 1px 1px 1px #fff;}\n");
  baseText += F("ul.hr {margin: 0; padding: 2px; }\n");
  baseText += F("ul.hr li {display: inline; border-right: 1px solid #000; padding-right: 6px;text-transform:  uppercase;font-weight:  400;}\n");
  baseText += F("ul.hr li:last-child { border-right: none;}\n");
  baseText += F("</style>\n");
  baseText += F("<html><head><title>WiseHouse Sensor v.0.8</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"></head>\n");
  baseText += F("<body><div class=\"base\"><div class=\"header\"><div class=\"logo\"><h1 class=\"inset\">WiseHouse - Termostat</h1></div>\n");
  baseText += F("<div class=\"menu\"><ul class=\"hr inset\">\n");
  baseText += F("<a href=\"/\"><li>Главная</li></a>\n");
  baseText += F("<a href=\"/config\"><li>Setup</li></a>\n");
  baseText += F("<a href=\"/contacts\"><li>Контакты</li></a>\n");
  baseText += F("</ul></div></div><div class=\"content\">\n");

  endText = "</div></div></body></html>\n";
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  EEPROM.begin(512);

  // Прочитаем настройку на удаленный сервер
  Serial.println();
  Serial.println("------------------------------------------------");
  Serial.println("|                     WISEHOUSE                 |");
  Serial.println("|Version - 0.8.1- 27.11.2021 - SOnoffRozetka    |");
  Serial.println("------------------------------------------------");
  Serial.println();
  Serial.println("Read data from EEPROM...");
  ip1byte = EEPROM.read(21);
  ip2byte = EEPROM.read(22);
  ip3byte = EEPROM.read(23);
  ip4byte = EEPROM.read(24);
  Serial.print("Server Ip Address:");
  Serial.print(ip1byte);
  Serial.print(".");
  Serial.print(ip2byte);
  Serial.print(".");
  Serial.print(ip3byte);
  Serial.print(".");
  Serial.print(ip4byte);
  Serial.println();



  ////////////////////////// Читаем и пишем в епром /////////////////////////////
  //Читаем ключ
  int key = EEPROM.read(10);
  Serial.print("KEY: ");
  Serial.println(key);

  if (key == 255) {
    Serial.println("Do nothing");
  } else {
    Serial.print("ssidFromEprom: ");
    ssidFromEprom = stringEpromRead(100, 124);
    Serial.print("pass------------: ");
    passFromEprom = stringEpromRead(125, 149);
    Serial.print("Controller Name-: ");
    contNameFromEprom = stringEpromRead(150, 170);
    Serial.print("DirectControll-: ");
    DirectControll = EEPROM.read(193);
    Serial.println(DirectControll);


  }
  delay(500);


  if (ssidFromEprom != 0) {
    // соединяемся с извесными сетями
    connectToAP2();
    // Проверяем есть ли мы в мажердоме
    testOrCreateObject();
  } else {
    // или создаем свою сеть
    setUpAP();
  }
  // Определяем режим работы Или нормальный или если нажата кнопка настроечный
  Serial.println("");
  for (int i = 0; i < 10; ++i) {
    digitalWrite(PIN_LED, HIGH);
    delay(100);
    digitalWrite(PIN_LED, LOW);
    delay(100);
    NormalMode = digitalRead(PIN_BUTTON);
  }



  digitalWrite(PIN_LED, HIGH);
  delay(1000);
  digitalWrite(PIN_LED, LOW);

  Serial.print(F("NormalMode"));
  Serial.print(" - ");
  Serial.println(NormalMode);

  delay(1000);

  //////////////////// Старт сервера ////////////////////////////////////////////////
  server.begin();
  Serial.print("Controller IP address - ");
  Serial.println(WiFi.localIP());
  Serial.println("CONTROLLER STARTED");
  /////////////////////Обработчики WEB событий/////////////////////////////////////
  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/setsrvip", handleSetSrv);
  server.on("/contacts", handleContacts);
  server.on("/on", handleOn);
  server.on("/click", handleClick);
  server.on("/off", handleOff);


  /////////////////Если нет такоего события
  server.onNotFound(handleNotFound);
  Serial.println("HTTP SERVER STARTED");
  Serial.println("");
  Serial.println("");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  currentMillis = millis();
  //Проверим настроецный режим или обычный
  if (NormalMode == 1) {
      //В цикле без задержек постоянно выполняем :
      if (btn1.click()) {
        Serial.println("click");
        SendToServer("buttonPressed", "button", "1");
      }
      //раз в 2 секунды
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        server.handleClient();
      }
      //раз в 60 секунд
      if (currentMillis - previousMillisForSend >= intervalForSend) {
        previousMillisForSend = currentMillis;
        if ((ip1byte.toInt() != 255) && (ip1byte.toInt() != 0) ) {
          SendToServer("keepalive", "alive", "1");
           Serial.println("keepalive");
        }
      }
      
      uptime = ((currentMillis / 1000) / 60);

      // Перезагружаем раз в 30 минут. На всякий случай
      resetMinute = ((resetDelay - currentMillis) / 1000) / 60;
      if (currentMillis  >= resetDelay) {
        resetFunc(); //вызываем reset // пока закомментируем проверим аптайм TODO: Сделать из админки
      }
  } else {

    // если выбран настроечный режим - Ничего не делаем, лишь обрабатываем сервер- ждем настройки
    if (currentMillis - previousMillis >= interval3) {
      previousMillis = currentMillis;
      // перенес вызов сервера на раз в 1 секунду.
      // см. https://esp8266.ru/forum/threads/apparatnoe-preryvanie-vyzyvaet-perezagruzku-esp8266.938/page-2#post-18584
      server.handleClient();

    }
  }

}
