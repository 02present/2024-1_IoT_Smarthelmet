#include <SoftwareSerial.h>

// GPS 모듈 연결 설정 (RX, TX)
SoftwareSerial gpsSerial(9, 10);

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
}

void loop() {
  if (gpsSerial.available()) {
    Serial.println("1");
    String gpsData = gpsSerial.readStringUntil('\n');
    if (gpsData.startsWith("$GPVTG")) {
      parseGPVTG(gpsData);
    }
  }
}

void parseGPVTG(String data) {
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

  // Extract and trim the speed in km/h
  String speedKmh = data.substring(comma6 + 1, comma7);
  speedKmh.trim();  // Remove any leading or trailing whitespace

  // Print speed in Knots
  if (speedKnots.length() > 0) {
    Serial.print("Speed (Knots): ");
    Serial.println(speedKnots);
  } else {
    Serial.println("Speed (Knots): N/A");
  }
}
