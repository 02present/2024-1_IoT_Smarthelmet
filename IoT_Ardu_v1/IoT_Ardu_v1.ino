#include <Wire.h>
#include <MPU9250.h>
#include <SoftwareSerial.h>
#include <BH1750.h>
#include <Adafruit_NeoPixel.h>

#define HM10_RX_PIN 2
#define HM10_TX_PIN 3
//#define GPS_RX_PIN 9
//#define GPS_TX_PIN 10
#define SENSOR_PIN A1
#define LED_PIN 6

#define LED_COUNT 32   // 네오픽셀 LED 개수
#define BRIGHTNESS 250  // 네오픽셀 LED 밝기(0 ~ 255) *RGBW만

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

SoftwareSerial hm10Serial(HM10_RX_PIN, HM10_TX_PIN);
SoftwareSerial gpsSerial(9, 10);
MPU9250 mpu;

int16_t threshold = 700;
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

  // GPS
  if (gpsSerial.available()) {
    String gpsData = gpsSerial.readStringUntil('\n');
    if (gpsData.startsWith("$GPVTG")) {
      dataString += "speed:";
      dataString += parseGPVTG(gpsData);
      dataString += " ";
    }
  }

  // 가속도 데이터 수신 및 충격 감지
  mpu.update();
  float ax = (mpu.getAccX() * 1000) + 200;
  float ay = (mpu.getAccY() * 1000) + 120;
  float az = (mpu.getAccZ() * 1000) - 900;
  Serial.print("ax = "); Serial.print(ax);
  Serial.print(" | ay = "); Serial.print(ay);
  Serial.print(" | az = "); Serial.println(az);
  dataString += "ax:"; dataString += ax; dataString += " ";
  dataString += "ay:"; dataString += ay; dataString += " ";
  dataString += "az:"; dataString += az; dataString += " ";

  if (abs(ax) > threshold || abs(ay) > threshold || abs(az) > threshold) {
    Serial.println("충격이 감지되었습니다!");
    dataString += "warning:1"; dataString += " ";
  }
  else{
    dataString += "warning:0"; dataString += " ";
  }

  // 조도 센서 데이터 수신 및 LED 제어
  int lightValue = analogRead(SENSOR_PIN);
  dataString += "bright:"; dataString += lightValue; dataString += " ";
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


String parseGPVTG(String data) {
  int comma1 = data.indexOf(',');
  int comma2 = data.indexOf(',', comma1 + 1);
  int comma3 = data.indexOf(',', comma2 + 1);
  int comma4 = data.indexOf(',', comma3 + 1);
  int comma5 = data.indexOf(',', comma4 + 1);
  int comma6 = data.indexOf(',', comma5 + 1);
  int comma7 = data.indexOf(',', comma6 + 1);

  // Extract and trim the speed in Knots
  String speedKnots = data.substring(comma5 + 1, comma6);
  speedKnots.trim();  // Remove any leading or trailing whitespace

  // Check if speedKnots is a valid number
  if (speedKnots.length() > 0 && isDigit(speedKnots.charAt(0))) {
    // Convert speedKnots to float and calculate speed in km/h
    float speedKnotsFloat = speedKnots.toFloat();
    float speedKmh = speedKnotsFloat * 1.852;

    // Return speed in km/h as a string
    return String(speedKmh, 3);  // Convert to string with 3 decimal places
  } else {
    return "N/A";
  }
}


