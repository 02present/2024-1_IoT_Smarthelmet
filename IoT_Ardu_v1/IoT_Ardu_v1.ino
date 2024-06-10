#include <Wire.h>
#include <MPU9250.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <BH1750.h>
#include <Adafruit_NeoPixel.h>

#define HM10_RX_PIN 2
#define HM10_TX_PIN 3
#define GPS_RX_PIN A2
#define GPS_TX_PIN A3
#define SENSOR_PIN A0
#define LED_PIN 6

#define LED_COUNT 32   // 네오픽셀 LED 개수
#define BRIGHTNESS 250  // 네오픽셀 LED 밝기(0 ~ 255) *RGBW만

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

SoftwareSerial hm10Serial(HM10_RX_PIN, HM10_TX_PIN);
SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);
TinyGPSPlus gps;
MPU9250 mpu;

int16_t threshold = 900;
const int DARK_THRESHOLD = 300;

void setup() {
  Serial.begin(9600);
  hm10Serial.begin(9600);
  gpsSerial.begin(9600);
  Wire.begin();
  strip.begin();                    // 네오픽셀 초기화(필수)
  strip.setBrightness(BRIGHTNESS);  // 네오픽셀 밝기 설정 *RGBW만
  strip.show();

  int color_r = 255;  // RED
  int color_g = 255;    // GREEN
  int color_b = 255;  // BLUE

  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, color_r, color_g, color_b, 0);
    // RGB일 경우 strip.setPixelColor(i, color_r, color_g, color_b);
  }

  // MPU9250 초기화
  if (!mpu.setup(0x68)) {  // change to your own address if necessary
    Serial.println("MPU9250 초기화 실패!");
    while (1);
  }

  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  String dataString = "";

  // GPS 데이터 수신 및 출력
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    Serial.print(c);  // 수신된 GPS 데이터를 시리얼 모니터에 출력

    if (gps.encode(c)) {
      if (gps.location.isValid()) {
        Serial.print("위도: ");
        Serial.print(gps.location.lat(), 6);
        Serial.print(", 경도: ");
        Serial.println(gps.location.lng(), 6);
      } else {
        Serial.println("위치 정보를 수신 중...");
      }
    }
  }

  // 가속도 데이터 수신 및 충격 감지
  mpu.update();
  float ax = mpu.getAccX() * 1000;
  float ay = mpu.getAccY() * 1000;
  float az = mpu.getAccZ() * 1000;
  Serial.print("ax = "); Serial.print(ax);
  Serial.print(" | ay = "); Serial.print(ay);
  Serial.print(" | az = "); Serial.println(az);
  dataString += "ax:"; dataString += String(ax); dataString += " ";
  dataString += "ay:"; dataString += String(ay); dataString += " ";
  dataString += "az:"; dataString += String(az); dataString += " ";

  if (abs(ax) > threshold || abs(ay) > threshold || abs(az) > threshold) {
    Serial.println("충격이 감지되었습니다!");
    dataString += "warning:1"; dataString += " ";
  }
  else{
    dataString += "warning:0"; dataString += " ";
  }

  // 조도 센서 데이터 수신 및 LED 제어
  int lightValue = analogRead(SENSOR_PIN);
  dataString += "bright:"; dataString += String(lightValue); dataString += " ";
  if (lightValue > DARK_THRESHOLD) {
    strip.setBrightness(BRIGHTNESS);  // 네오픽셀 밝기 설정 *RGBW만
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, 255, 255, 255, 0);  // 흰색으로 설정
    }
    strip.show();
    Serial.println("어두워서 LED를 켭니다.");
    dataString += "light:1"; dataString += " ";
  } else {
    strip.setBrightness(0);  // 네오픽셀 밝기 설정 *RGBW만
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, 0, 0, 0, 0);  // LED를 끄기 위해 모든 색상을 0으로 설정
    }
    strip.show();
    Serial.println("밝아서 LED를 끕니다.");
    dataString += "light:0"; dataString += " ";
  }
  // 데이터를 블루투스 시리얼 포트로 전송
  hm10Serial.println(dataString);

  // 데이터 문자열을 시리얼 모니터에 출력
  Serial.println(dataString);

  delay(1000);
}
