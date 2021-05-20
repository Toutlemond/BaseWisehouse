//// Прошивка Базовая. На ее основе можно будет лепить другие вещи.
///  14.10.2016
///  16.10.2017 - добавил выравнивание в HTML
//   15.10.2018 - Добавлена функция перезагрузки каждые 500 000 мс
//   22.10.2018 - Код разделен на разные файлы
//   05.09.2019 - Сделана как базовая. Убраны некоторые вещи.
//   24.09.2020 - Исправлен метод отправки на сервер
//   03.11.2020 - Добавим поддержку 433 мГц устройств

//Что требуется сделать
// 1 - описать как ставить уставку из сети.
// 2 - Сделать ребут из сети
// 3 - Сделать отправку данных на сервер - сейчас она поломана - отправлять и текущую температуру и уставку и возможно чтото еще


// Version 0.8

/// Задачи -
/// 1. Опрос Датчиков
/// 1.1.Проверка датчика Газа
/// 1.2.проверять значение термодатчика
/// 1.3. Проверять значение датчика влажности(или датчика намокания)
/// 1.4. Проверять значение датчика тока

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
#include <OneWire.h>
#include <DallasTemperature.h>
#include "button.h"
#include <RCSwitch.h>


///////////////////////////////////НАЗНАЧЕНИЕ НОЖЕК ////////////////////////////////////////////////////
#define REOSTATPIN A0
//const int SinkPin = 13;/// Вход для сенсора затопления
const int ledPin = 5;    /// Выход на светодиод
const int OPTRON = 5;          /// Пин на оптрон Тот же самый
const int GREENOPTRON = 15 ;    /// Пин Для мигания зеленым
const int RELEY1 = 14;         /// Пин Для Реле 1
const int RELEY2 = 12 ;         /// Пин Для Реле 2

//#define DHTTYPE DHT11
//#define DHTPIN 13        /// Вход для датчика температуры и влажности
#define ONE_WIRE_BUS 13 // если датчик DS
const int gasPin = 4;    /// Вход для цифрового датчика газа
const int caniSleep = 0; /// нулевая нога, если при работе закоммутировано - не засыпает

Button btn1(caniSleep);






// Имя точки доступа в случае если не подключилось к сети пользователя
const char *ssid = "WiseIS";
const char *password = "12345678";

const int DHTFlag = 0; // Флаг используется ли DHT(1) или Dallas DS1820 (0)

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


////////////////////Определим перемнные для крутилки//////////////////////
int samples;
String maxTemp;
String minTemp;
String setTemp;
int flagTempset = 0;
int DirectControll = 0;
int maxTempInt;
int minTempInt;
int setTempint;
int adcMin = 0;
int adcMax = 1002;
float temtStep;
float tempUstavka;
int NormalMode = 0;
String deltaTemp; /////////Дельта для срабатывания термодатчика.

////////////////////Определим перемнные для Цифрового датчика температуры DS18B20//////////////////////

float digiTemp1;
float digiTemp2;
float humidity, temp_f;
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

/////////////////////////////////// Включение функций /////////////////////////////////////////////////
#if (DHTON == 1)
DHT dht(DHTPIN, DHTTYPE);
#endif

#if (ONEWIREON == 1)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
#endif

//RCSwitch
#if (RCSWITCHON == 1)
RCSwitch mySwitch = RCSwitch();
#endif


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

/*const char *ssidFromEprom;
  const char *passFromEprom;
  const char *contNameFromEprom;
*/
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

  ///////Запускаем DHT
#if (DHTON == 1)
  dht.begin();
#endif

#if (RCSWITCHON == 1)
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
#endif

#if (ONEWIREON == 1)
  sensors.begin();
#endif
  ///////////////////////////Определяем входы выходы //////////////////////////////////
  /*pinMode(ledPin, OUTPUT);                   // Назначим пины как выход
    pinMode(SinkPin, INPUT);                   // Назначим пины как вход
    pinMode(gasPin, INPUT);                    // Назначим пины как вход
    pinMode(caniSleep, INPUT);                 // Назначим пины как вход
  */
  pinMode(OPTRON, OUTPUT);
  pinMode(GREENOPTRON, OUTPUT);
  pinMode(RELEY1, OUTPUT);
  pinMode(RELEY2, OUTPUT);

  digitalWrite(ledPin, 0);                   // Потушим пока пин.
  digitalWrite(RELEY1, HIGH);                   // Потушим пока пин. // Реле с обратной логикой
  digitalWrite(RELEY2, HIGH);                   // Потушим пока пин.

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
  Serial.println("|Version - 0.8.1- 03.11.2020 - Add RCSwittch    |");
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
    Serial.print("deltaTemp-: ");
    deltaTemp = stringEpromRead(171, 175);
    Serial.print("maxTemp-: ");
    maxTemp = stringEpromRead(176, 180);
    Serial.print("minTemp-: ");
    minTemp = stringEpromRead(181, 185);
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
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    delay(100);
    NormalMode = digitalRead(caniSleep);
  }



  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);

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
  server.on("/gettemp", handleGettemp);
  server.on("/settemp", handleSettemp);



  /////////////////Если нет такоего события
  server.onNotFound(handleNotFound);
  Serial.println("HTTP SERVER STARTED");
  Serial.println("");
  Serial.println("");



  //////////////////////Для крутилки рассчет значений////////////////////////////////////////
  if (NormalMode == 1) {
    if (key != 255) {
      temtStep = (adcMax - adcMin) / (maxTemp.toInt() - minTemp.toInt());
      Serial.print("temtStep");
      Serial.print(" - ");
      Serial.println(temtStep);
    }

  } else {

    // Перебор всего епрома.
    for (ee = 0; ee <= 512; ee++) {
      eevalue = EEPROM.read(ee);

      Serial.print(ee);
      Serial.print("\t");
      Serial.print(eevalue, DEC);
      Serial.println();
    }
    Serial.println(F("It is a setup mode. Please fing wiseIs network and setup device"));
  }





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
