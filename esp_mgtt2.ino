#include <ESP8266WiFi.h>
#include <PubSubClient.h> // клиент MQTT
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h> // Библиотека для OTA-прошивки


#define LED1 4 // определяем пин светодиода
#define LED2 1 // определяем пин светодиода
#define IN1  3 // вход
#define MQTT_client "maxico_esp-01-1" // произвольное название MQTT клиента, иногда требуется уникальное.

// настройки домашней сети
const char *ssid =  "horeca-service";  // название точки достпа 
const char *password =  "89787562306"; // Пароль от точки доступа

// настройки для MQTT брокера
const char *mqtt_server = "192.168.50.187"; // адрес сервера MQTT
const int mqtt_port = 1883; // Порт для подключения к серверу MQTT
const char *mqtt_user = ""; // Логин от сервера MQTT
const char *mqtt_pass = ""; // Пароль от сервера MQTT
const char *led_topic1="test/led1"; // топик для светодиода1
const char *led_topic2="test/led2"; // топик для светодиода2
//const char *data_topic1="test/data1"; // топик для данных
const char *data_topic2="test/data2"; // топик для данных


int pause = 300; // переменная для паузы между отправками данных
long int times=0; // для времени

int oldValue = 0;



WiFiClient wclient;      
PubSubClient client(wclient, mqtt_server, mqtt_port);


// получение данных с сервера и отработка
void callback(const MQTT::Publish& pub)    
{
    String topic = pub.topic();
    String payload = pub.payload_string(); // чтение данных из топика
    // действия над светодиодом в зависимости от данных из топика
    if (topic==led_topic1) 
    {
    if (payload[0] == '0') digitalWrite(LED1, LOW); 
    else if (payload[0] == '1') digitalWrite(LED1, HIGH); 
    else if (payload[0] == '2') digitalWrite(LED1, !digitalRead(LED1)); 
    }
    if (topic==led_topic2) 
    {
    if (payload[0] == '1') digitalWrite(LED2, LOW); 
    else if (payload[0] == '0') digitalWrite(LED2, HIGH); 
    else if (payload[0] == '2') digitalWrite(LED2, !digitalRead(LED2)); 
    }


    
}
//-------------------------------



// Функция отправки показаний
void refreshData() {
  /*if (pause == 0) {
    times = millis(); // формируем данные для отправки
    client.publish(data_topic1, String(times));
    pause = 3000; // пауза меду отправками 3 секунды
  }
  */

  //client.publish(data_topic2,"debug");
  
  int value = digitalRead(IN1);
     if (value != oldValue) {  
        oldValue = value;
        if (value != 1) client.publish(data_topic2, "1");
        else client.publish(data_topic2, "0");
        
      
    }    
    
 
  delay(1); 
}
//------------------------------------

void setup() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.setHostname("ESP-01-00001"); // Задаем имя сетевого порта
  //ArduinoOTA.setPassword((const char *)"0000"); // Задаем пароль доступа для удаленной прошивки
  ArduinoOTA.begin(); // Инициализируем OTA
 
    
    //pinMode(2, INPUT_PULLUP);
    
    pinMode(LED1, OUTPUT); // пин светика1 на выход
    pinMode(LED2, OUTPUT); // пин светика2 на выход
    pinMode(IN1, INPUT);
}

void loop() {
    ArduinoOTA.handle(); // Всегда готовы к прошивке
     
    // подключаемся к MQTT серверу
    if (WiFi.status() == WL_CONNECTED) { // если есть подключение к wi-fi
        if (!client.connected()) { // если нет подключения к серверу MQTT
            
            if (client.connect(MQTT::Connect(MQTT_client) // если соединились то делаем всякое
                                 .set_auth(mqtt_user, mqtt_pass))) {
               
                client.set_callback(callback);
                client.subscribe(led_topic1);                  // подписка на топик led1
                client.subscribe(led_topic2);                  // подписка на топик led2
                //client.subscribe(data_topic1);                 // подписка на топик data
                client.subscribe(data_topic2);                 // подписка на топик data
             }
        }
        
        if (client.connected()){ // если есть соединение с MQTT 
            client.loop();
            refreshData();
        }
    }

    

    
} // конец основного цикла



