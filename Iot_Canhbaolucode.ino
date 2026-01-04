/*************************************************
 * ESP32 + Laser Distance Sensor (UL53LDK)
 * Flood Monitoring System - Blynk
 *************************************************/

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <Wire.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_VL53L0X.h>

/* ---------- BLYNK INFO ---------- */
#define BLYNK_TEMPLATE_ID   "TMPLxxxxxx"
#define BLYNK_TEMPLATE_NAME "Flood Monitoring"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"

/* ---------- WIFI ---------- */
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

/* ---------- I2C PIN (TÙY CHỌN) ---------- */
#define SDA_PIN 16
#define SCL_PIN 17

/* ---------- NGƯỠNG CẢNH BÁO (cm) ---------- */
#define LEVEL_WARNING  50
#define LEVEL_DANGER   80

/* ---------- THAM SỐ ---------- */
#define SENSOR_HEIGHT  150   // cm – chiều cao từ cảm biến tới đáy sông
#define MEASURE_TIME   5000  // ms

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
BlynkTimer timer;

/* ---------- HÀM ĐỌC & GỬI DỮ LIỆU ---------- */
void readWaterLevel() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {
    float distance_cm = measure.RangeMilliMeter / 10.0;
    float water_level = SENSOR_HEIGHT - distance_cm;

    if (water_level < 0) water_level = 0;

    // Gửi mực nước lên Blynk
    Blynk.virtualWrite(V0, water_level);

    // Xác định trạng thái
    if (water_level < LEVEL_WARNING) {
      Blynk.virtualWrite(V1, "Normal");
    } 
    else if (water_level < LEVEL_DANGER) {
      Blynk.virtualWrite(V1, "Warning");
      Blynk.logEvent("warning", "Water level rising!");
    } 
    else {
      Blynk.virtualWrite(V1, "DANGER");
      Blynk.logEvent("danger", "Flood risk detected!");
    }

    Serial.print("Water level: ");
    Serial.print(water_level);
    Serial.println(" cm");
  }
  else {
    Serial.println("Sensor error!");
  }
}

void setup() { 
  Serial.begin(115200);

  // I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Kết nối Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Khởi động cảm biến
  if (!lox.begin()) {
    Serial.println("Failed to detect VL53 sensor!");
    while (1);
  }

  // Timer đọc dữ liệu
  timer.setInterval(MEASURE_TIME, readWaterLevel);

  Serial.println("System started...");
}

void loop() {
  Blynk.run();
  timer.run();
}
