#include <WiFi.h>
#include <PubSubClient.h>
#define BLYNK_TEMPLATE_ID "xxxxx"
#define BLYNK_DEVICE_NAME "xxxxx"
#define BLYNK_AUTH_TOKEN "xxxxxx"

#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp32.h>

#define LINE_TOKEN  "xxxxx"  //line token
#include <TridentTD_LineNotify.h>

#include "DHT.h"
#define DHTPIN 27     //Pin ขาสำหรับ เสียบกับ Nodemcu
#define analog  34
const int Relay = 14;
#define DHTTYPE DHT11
WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_client = "xxxx";
const char* mqtt_username = "xxxx";
const char* mqtt_password = "xxxx";
char ntp_server1[20] = "pool.ntp.org";
char ntp_server2[20] = "time.nist.gov";
char ntp_server3[20] = "time.uni.net.th"; 

int check =0;
char auth[] = "_FN_YfE9gkOB7s40_9WT3Vit6qCjgO-k"; 

char ssid[] = "xxxx";  //ชื่อไวไฟ
char pass[] = "xxxx";  //รหัส


int SR = 0; // ประกาศตัวแปร SR มีค่าเท่ากับ 0
int soil = 0; // ประกาศตัวแปร temp มีค่าเท่ากับ 0
int c;

DHT dht(DHTPIN, DHTTYPE);

void reconnect() {
  while (!client.connected()) {
    if (client.connect(mqtt_client, mqtt_username, mqtt_password)) {
      client.subscribe("@msg/smart");  //subscribe to TOPIC 
    }
    else {
      delay(5000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }

  //Turn on LED if an incoming message is "open", delay for 2s and then turn it off.
  if (message == "open") {
     myservo.write(35); // สั่งให้ Servo หมุนไปองศาที่ 35
  }
  if(message == "close") {
     myservo.write(160); // สั่งให้ Servo หมุนไปองศาที่ 160
     delay(1000);
     myservo.write(100); // สั่งให้ Servo หมุนไปองศาที่ 100
  }
}
void setup()
{
  Serial.begin(115200);
  pinMode(Relay, OUTPUT); // กำหนด รีเลย์เป็น Output
  dht.begin(); 
  WiFi.mode(WIFI_STA);
  Serial.println(LINE.getVersion());
  Serial.println("WiFi Connecting");
  Serial.println(ssid);
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED)       //ตรวจเช็คสถานะการเขื่อมต่อ wifi
  {
    delay(500);
    Serial.print(".");                      //ถ้าไม่สำเร็จให้แสดง "." จนกว่าจะเขื่อมต่อได้
  }
  Serial.println("");
  Serial.println("WiFi connected"); 
  Blynk.begin(auth, ssid, pass); // เชื่อมต่อ auth , ssis , pass      
  LINE.setToken(LINE_TOKEN);
}
BLYNK_WRITE(V3)                              
  { 
   float h = dht.readHumidity();                                         
   int val = param.asInt();
    if (soil < 69) {  //ตั้งค่า % ที่ต้องการจะรดน้ำต้นไม้
      if(h > 95) {
         LINE.notify("ความชื้นในอากาศสูงมีแนวโน้มฝนจะตกไม่ควรรดน้ำต้นไม้ --> Humidity : "+String(h)+" % ");
      }
      else{
        digitalWrite(Relay ,val); 
        delay(3000);  
        LINE.notify("รดน้ำต้นไม้เเล้ว: soil moisture "+String(soil)+" %");
      }
    }
    else { 
      LINE.notify("ความชื้นดินเพียงพอเเล้วไม่ควรรดน้ำต้นไม้ --> soil moisture : "+String(soil)+" %");   
     }                                
  }  
  
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  Blynk.run();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  SR = analogRead(34); // ให้ตัวแปล SR อ่านค่าจาก Pin A0
  soil = map(SR, 4095, 0, 0, 100); // ตัวแปล Temp คือ แปลงค่าของตัวแปร SR จาก 1023 ถึง 0 เป็น 0 ถึง 100  
  Blynk.virtualWrite(V0, soil);
  Serial.print("Soil Moisture = ");
  Serial.print(soil);
  Serial.println(" %");
  if (soil > 49) {  //ตั้งค่า % ที่ต้องการจะรดน้ำต้นไม้
    if (c == 1){
       digitalWrite(Relay, LOW);
       LINE.notify("รดน้ำต้นไม้เเล้ว: soil moisture "+String(soil)+" %");
       c = 0;
    }
    else {
      digitalWrite(Relay, LOW);
     }
  }
  else {
     if(h < 95) {
        digitalWrite(Relay, HIGH);
        LINE.notify("soil moisture "+String(soil)+" %");
        c = 1; 
      }
    else{
      if (soil < 40) {
        digitalWrite(Relay, HIGH);
        LINE.notify("soil moisture "+String(soil)+" %");
        c = 1; 
      }
      else{
       digitalWrite(Relay, LOW);
    }
  }
  }   
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F(" C "));
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
  delay(10000); 
}
