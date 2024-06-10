#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

#define HM10_RX_PIN 2
#define HM10_TX_PIN 3
#define SENSOR_PIN A0

SoftwareSerial hm10Serial(HM10_RX_PIN, HM10_TX_PIN);

const int DARK_THRESHOLD = 300;

void setup() {
  Serial.begin(9600);
  hm10Serial.begin(9600);
  Wire.begin();
}

void loop() {
  String dataString = "speed:50 ";

  // 조도 센서 데이터 수신 및 LED 제어
  int lightValue = analogRead(SENSOR_PIN);
  dataString += "bright:"; dataString += String(lightValue); dataString += " ";
  if (lightValue > DARK_THRESHOLD) {
    Serial.println("어두워서 LED를 켭니다.");
    dataString += "light:1"; dataString += " ";
  } else {
    Serial.println("밝아서 LED를 끕니다.");
    dataString += "light:0"; dataString += " ";
  }

  // 데이터를 블루투스 시리얼 포트로 전송
  hm10Serial.println(dataString);

  // 데이터 문자열을 시리얼 모니터에 출력
  Serial.println(dataString);

  delay(1000);
}
