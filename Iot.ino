#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <WiFiManager.h> 
#include <max6675.h>
#define EN 23
#define rPWM 2
#define lPWM 15
#define pinServo 22
//max6675
#define pinCLK 5
#define pinCS 17
#define pinDO 16

MAX6675 thermo(pinCLK, pinCS, pinDO);
HTTPClient http;
WiFiManager wm;
//Setting PWM
Servo servo;
ESP32PWM Rpwm;
ESP32PWM Lpwm;
//wifi
int status = WL_IDLE_STATUS;
bool res;
char ssid[] = "KAL3A2G";
char pass[] = "@JOGJA2G";
//url
String postSuhu = "https://iotcoffeeroaster.000webhostapp.com/sensor";
String urlData = "https://iotcoffeeroaster.000webhostapp.com/getData";
String getProject = "https://iotcoffeeroaster.000webhostapp.com/getProject";
//setting waktu
unsigned long delayPost;
unsigned long delayGet;
unsigned long delayProject;
unsigned long delayServo;
int satuMenit = 60000;
//global Variable
String Sistem;
String projectKe;
int Power;
int Rpm;
int i;
float MaxSuhu;
//setting RoR
unsigned long delayRoR;
float suhuAkhir;
float RoR;
float suhu;

void setup() {
 delayPost = millis();
 delayGet = millis();
 delayProject = millis();
 delayRoR = millis();
 Serial.begin(9600);
 
 pinMode(EN, OUTPUT);
// Allow allocation of all timers
 ESP32PWM::allocateTimer(0);
 ESP32PWM::allocateTimer(1);
 ESP32PWM::allocateTimer(2);
 ESP32PWM::allocateTimer(3);
 servo.setPeriodHertz(50);// Standard 50hz servo
 servo.attach(pinServo, 500, 2400);
 Rpwm.attachPin(rPWM, 5000, 10); // 5KHz 8 bit
 Lpwm.attachPin(lPWM, 5000, 10); // 5KHz 8 bit

 koneksi();
}

void loop() {
 //default suhu
 if(Sistem == "mati"){
  suhuAkhir = thermo.readCelsius();
  Serial.println("cobaa" + String(suhuAkhir));
  digitalWrite(EN, LOW);
  Rpwm.writeScaled(0);
  Lpwm.writeScaled(0);
  servo.write(0);
 }


//getData
 if(millis() - delayGet > 3000){
  delayGet = millis();

  String data = getData();
  StaticJsonDocument<1024> doc;
  deserializeJson(doc, data);
  String sistem = doc[0]["sistem"];
  if(sistem != "null"){
    Sistem = sistem;
    int idProject = doc[0]["projectKe"];
    projectKe = idProject;
    int power = doc[0]["servo"];
    Power = power;
    int rpm = doc[0]["rpm"];
    Rpm = rpm;
    int maxSuhu = doc[0]["maxSuhu"];
    MaxSuhu = maxSuhu;
  }

//if Fail dataJson
while(sistem == "null"){
    String data = getData();
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, data);
    String sistem = doc[0]["sistem"];

    if(sistem != "null"){
      Sistem = sistem;
      int idProject = doc[0]["projectKe"];
      projectKe = idProject;
      int power = doc[0]["servo"];
      Power = power;
      int rpm = doc[0]["rpm"];
      Rpm = rpm;
      int maxSuhu = doc[0]["maxSuhu"];
      MaxSuhu = maxSuhu;
      break;
    }
  }
  
  Serial.println("Sistem: " + Sistem);
  Serial.println("Rpm: " + String(Rpm));

 }


//Logic Alat
if(Sistem == "hidup"){
//Motor
  float PWM = Rpm * 0.01;
  digitalWrite(EN, HIGH);
  Rpwm.writeScaled(PWM);
  Lpwm.writeScaled(0);

//Post Data
  if(millis() - delayPost > 5000){
  suhu = thermo.readCelsius();
  delayPost = millis();

  postData(suhu,projectKe,RoR);
 } 

//Servo
  if(millis() - delayServo > 300){
    delayServo = millis();
    if(suhu < MaxSuhu){
      if(i < Power){
        i += 2;
        perintahServo(i);
      }else{
        i = Power;
        perintahServo(Power);
      }
    }else {
      if(i >= 20){
        i -= 2;
        perintahServo(i);
      }
    }
  }

//RoR
  if(millis() - delayRoR > 30000){
  delayRoR = millis();
  RoR = suhu - suhuAkhir;
  suhuAkhir = suhu;
  Serial.println("cobaa" + String(RoR));
  }

}
}
//endVoidLoop

//Fungsi Servo
void perintahServo(int val){
  int deg = map(val, 0, 100, 0, 180);
  servo.write(deg);
}

//Fungsi Post Data
void postData(float suhu, String projectKe, float RoR){
 http.begin(postSuhu);
 http.addHeader("Content-Type", "application/x-www-form-urlencoded");
 String data = "suhu=" + String(suhu) + "&projectKe=" + String(projectKe) + "&RoR=" + String(RoR) + "&power=" + String(Power) + "&rpm=" + String(Rpm);
 int status = http.POST(data);
 Serial.println("Status: " + String(status));

 http.end();
}

//Fungsi Get Data
String getData(){
 http.begin(urlData);
 int statusGetData = http.GET();
 String getData = http.getString();

 Serial.println(String(statusGetData));
 http.end();
 return getData;
}

//Fungsi Koneksi
void koneksi(){
while (WiFi.status() != WL_CONNECTED) {
  Serial.println("Mengubungkan");
  res = wm.autoConnect("CoffeRoaster","12345678");
  delay(5000);
 };
 Serial.println("Terhubung");
}