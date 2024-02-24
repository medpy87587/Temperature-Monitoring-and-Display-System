#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// Set the LCD address and dimensions
#define I2C_ADDR    0x27
#define LCD_COLS    16
#define LCD_ROWS    4

#define ONE_WIRE_BUS 4
#define TEMPERATURE_PRECISION 12

const char* ssid = "Wokwi-GUEST";
const char* password = "";
// Firebase project ID and database path
// Firebase project ID and database path
#define FIREBASE_HOST "----------------"
#define API_KEY "---------"





OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLS, LCD_ROWS);

FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

unsigned long lastHourCheck = 0;
float hourlyTemperatures[24]; // Array to store temperature readings for each hour
int hourlyCounts[24]; // Array to store the count of temperature readings for each hour
bool hourlyMeanCalculated[24]; // Array to store flags indicating whether the mean temperature for each hour has been calculated
String DATABASE_PATH="/temperature" ;
bool signupOK=false;
void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Initialize temperature sensor
  sensors.begin();

  // Initialize LCD
  lcd.init();
  lcd.setCursor(1, 0);
  lcd.println("Temperature is :");

  // Initialize Firebase
  config.api_key = API_KEY;
  config.database_url = FIREBASE_HOST;
    /* Sign up */
  if (Firebase.signUp(&config, &auth, "testxxxpxiix@gmail.com", "1111111")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  Firebase.begin(&config, &auth);

  // Initialize hourly temperature arrays and update Firebase with placeholder values
  for (int i = 0; i < 24; i++) {
    String path = DATABASE_PATH + "/hour_" + String(i) + "/mean_temperature";
    Firebase.setString(firebaseData, path.c_str(), 0);
    if (firebaseData.dataType() == "float") {
      Serial.println("Placeholder value sent to Firebase for hour " + String(i));
    } else {
      Serial.println("Failed to send placeholder value to Firebase for hour " + String(i));
      Serial.println("Error: " + firebaseData.errorReason());
    }
  }
}

void loop() {
  // Read temperature
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  if (temperatureC != DEVICE_DISCONNECTED_C) {
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" °C");

    // Update instant temperature value on Firebase
    Firebase.setFloat(firebaseData, "/instant_temperature", temperatureC);
    delay(1000);
    if (firebaseData.dataType() == "float") {
      Serial.println("Instant temperature data sent to Firebase: " + String(temperatureC) + " °C");
    } else {
      Serial.println("Failed to send instant temperature data to Firebase.");
      Serial.println("Error: " + firebaseData.errorReason());
    }

    // Update hourly temperature and count
    int hour = millis() / 3600000 % 24;
    hourlyTemperatures[hour] += temperatureC;
    hourlyCounts[hour]++;

    // Display temperature on LCD
    lcd.setCursor(1, 3);
    char temperatureStr[10];
    dtostrf(temperatureC, 4, 2, temperatureStr);
    lcd.println(temperatureStr);
  } else {
    Serial.println("Error: Unable to read temperature data");
  }

  // Check if an hour has passed
  if (millis() - lastHourCheck >= 3600000) { // Check every hour
    lastHourCheck = millis();

    // Calculate the current hour
    int hourIndex = (millis() / 3600000) % 24;

    // Calculate the mean temperature for the previous hour if it hasn't been calculated yet
    int previousHourIndex = (hourIndex - 1 + 24) % 24;
    if (!hourlyMeanCalculated[previousHourIndex] && hourlyCounts[previousHourIndex] > 0) {
      float meanTemperature = hourlyTemperatures[previousHourIndex] / hourlyCounts[previousHourIndex];

      // Construct JSON object with mean temperature data
      FirebaseJson json;
      json.set("hour", previousHourIndex);
      json.set("mean_temperature", meanTemperature);

      // Update data on Firebase
      String path = DATABASE_PATH + "/hour_" + String(previousHourIndex);
      if (Firebase.updateNode(firebaseData, path.c_str(), json)) {
        Serial.println("Mean temperature data sent to Firebase for hour " + String(previousHourIndex));
      } else {
        Serial.println("Failed to send mean temperature data to Firebase for hour " + String(previousHourIndex));
        Serial.println("Error: " + firebaseData.errorReason());
      }

      // Set flag indicating that the mean temperature for this hour has been calculated
      hourlyMeanCalculated[previousHourIndex] = true;
    }

    // Reset temperature and count for the new hour
    int hour = millis() / 3600000 % 24;
    hourlyTemperatures[hour] = 0;
    hourlyCounts[hour] = 0;

    // Reset flag indicating that the mean temperature for this hour has not been calculated yet
    hourlyMeanCalculated[hour] = false;
  }

  delay(1000); // Adjust delay as needed
}
