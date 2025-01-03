  #include <M5Core2.h>
  #include <DHT20.h>
  #include <Wire.h>
  #include <Adafruit_NeoPixel.h>
  #include <Adafruit_MQTT.h>
  #include <Adafruit_MQTT_Client.h>
  #include <WiFi.h>
  #include <ESPAsyncWebServer.h>
  #include <SPIFFS.h>
  #include <ArduinoJson.h>

  #define WLAN_SSID "HUY"
  #define WLAN_PASS "123456789"
  #define AIO_USERNAME  "khanhhuy2003"
  #define AIO_KEY       "aio_lvVc393MViJyeUidjiXK7ptohMmg"
  #define AIO_SERVER      "io.adafruit.com"
  #define AIO_SERVERPORT  1883

  WiFiClient client;
  Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

  /****************************** Feeds ***************************************/
  //Cambiennhietdo
  Adafruit_MQTT_Publish cambiennhietdo = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");

  //Cambiendoam
  Adafruit_MQTT_Publish cambiendoam = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");

  //Cambienanhsang
  Adafruit_MQTT_Publish cambienanhsang = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/light");

  //nutnhan
  Adafruit_MQTT_Publish doamdat = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/soil");



  //void TaskBlink(void *pvParameters);
  void TaskTemperatureHumidity(void *pvParameters);
  void TaskSoilMoistureAndRelay(void *pvParameters);
  void TaskLightAndLED(void *pvParameters);
  void TaskPrintLCD(void *pvParameters);
  void TaskPublishToAdafruit(void *pvParameters);
  bool publishToAdafruit(Adafruit_MQTT_Publish &feed, float value);


DHT20 dht20;
Adafruit_NeoPixel pixels3(4, 13, NEO_GRB + NEO_KHZ800);

AsyncWebServer server(80);

float Temperature = 0;
float Humidity = 0;
uint32_t light = 0;
uint32_t moisture = 0;

  void setup() {

      M5.begin(true, false, true, true, kMBusModeOutput, false); 
      dht20.begin();
      pixels3.begin();
      
      M5.Lcd.setTextSize(2);  
      M5.Lcd.fillScreen(BLACK);  
      WiFi.begin(WLAN_SSID, WLAN_PASS);

    WiFi.softAP(ssid, password);

    // Print AP details
    Serial.println("Access Point Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Set up web server route



    xTaskCreate(TaskTemperatureHumidity, "Task Temperature" ,2048  ,NULL  ,2 , NULL);
    xTaskCreate(TaskLightAndLED, "Task Light LED" ,2048  ,NULL  ,2 , NULL);
    xTaskCreate(TaskSoilMoistureAndRelay, "Task Soild Relay" ,2048  ,NULL  ,2 , NULL);
    xTaskCreate(TaskPrintLCD, "LCD" ,2048  ,NULL  ,2 , NULL);
    xTaskCreate(TaskPublishToAdafruit, "LCD" ,2048  ,NULL  ,2 , NULL);


  
    }
  }
  void loop() {

  }
  void TaskTemperatureHumidity(void *pvParameters) {  
    while(1) {                          
      dht20.read();
      Temperature = dht20.getTemperature();
      Humidity = dht20.getHumidity();

      // Display Temperature

      Serial.print("Temperature: ");
      Serial.print(Temperature);
      Serial.println(" C");
      Serial.print("Humidity: ");
      Serial.print(Humidity);
      Serial.println(" %");
      vTaskDelay(3000);
      
    }
  }
  void TaskLightAndLED(void *pvParameters) { 

    while(1) { 
      light = analogRead(34);
      Serial.print(F("Light: "));
      Serial.println(light);                      
      if(light < 700){
        pixels3.setPixelColor(0, pixels3.Color(0,0,255));
        pixels3.setPixelColor(1, pixels3.Color(0,255,0));
        pixels3.setPixelColor(2, pixels3.Color(255,0,0));
        pixels3.setPixelColor(3, pixels3.Color(128, 0, 128));
        pixels3.show();
      }
      if(light > 700){
        pixels3.setPixelColor(0, pixels3.Color(0,0,0));
        pixels3.setPixelColor(1, pixels3.Color(0,0,0));
        pixels3.setPixelColor(2, pixels3.Color(0,0,0));
        pixels3.setPixelColor(3, pixels3.Color(0,0,0));
        pixels3.show();
      }
      vTaskDelay(3000);
    }
  }
  void TaskSoilMoistureAndRelay(void *pvParameters) {  
    pinMode(2, OUTPUT);

    while(1) { 
      moisture = analogRead(36);   
      //Serial.print("IP: ");
      //Serial.println(WiFi.softAPIP());                     
      // if(moisture > 300){
      //   digitalWrite(2, LOW);
      // }
      // if(moisture < 50){
      //   digitalWrite(2, HIGH);
      // }
      Serial.println(F("Moisture:"));
      Serial.println(moisture);

      Serial.println("");

      vTaskDelay(3000);
    }
  }
  void TaskPrintLCD(void *pvParameters) {
    while (1) {
      M5.Lcd.clear();
      M5.Lcd.setCursor(0, 50);
      M5.Lcd.setTextColor(YELLOW);
      M5.Lcd.println("Light: ");
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(90, 50);
      M5.Lcd.printf("%d", light);

      M5.Lcd.setCursor(0, 100);
      M5.Lcd.setTextColor(YELLOW);
      M5.Lcd.println("Temperature: ");
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(200, 100);
      M5.Lcd.printf("%.2f *C", Temperature);

      M5.Lcd.setCursor(0, 150);
      M5.Lcd.setTextColor(YELLOW);
      M5.Lcd.println("Humidity: ");
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(150, 150);
      M5.Lcd.printf("%.2f ", Humidity);

      M5.Lcd.setCursor(0, 200);
      M5.Lcd.setTextColor(YELLOW);
      M5.Lcd.println("Moisture: ");
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(200, 200);
      M5.Lcd.printf("%d", moisture);
      vTaskDelay(3000);

    }
}
void MQTT_connect() {
    int8_t ret;

    if (mqtt.connected()) {
        return;
    }

    Serial.print("Connecting to MQTT... ");
    uint8_t retries = 3;
    while ((ret = mqtt.connect()) != 0) {
        Serial.println(mqtt.connectErrorString(ret));
        Serial.println("Retrying MQTT connection in 10 seconds...");
        mqtt.disconnect();
        delay(10000); // Wait 10 seconds
        retries--;
        if (retries == 0) {
            ESP.restart(); // Restart the device
        }
    }
    Serial.println("MQTT Connected!");
}
void TaskPublishToAdafruit(void *pvParameters) {
    while (1) {
        // Ensure MQTT connection is alive
        MQTT_connect();

        // Publish Temperature
        publishToAdafruit(cambiennhietdo, Temperature);

        // Publish Humidity
        publishToAdafruit(cambiendoam, Humidity);

        // Publish Light Intensity
        publishToAdafruit(cambienanhsang, light);

        // Publish Soil Moisture
        publishToAdafruit(doamdat, moisture);

        // Delay to allow other tasks to run
        vTaskDelay(6000 / portTICK_PERIOD_MS); // Publish every 2 seconds
    }
}
bool publishToAdafruit(Adafruit_MQTT_Publish &feed, float value) {
    if (feed.publish(value)) {
        Serial.print("Published successfully: ");
        Serial.println(value);
        return true;
    } else {
        Serial.print("Failed to publish: ");
        Serial.println(value);
        return false;
    }
}

void handleRoot() {
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
        <title>ESP32 Real-Time Dashboard</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                margin: 0;
                padding: 0;
                background-color: #f4f4f9;
                color: #333;
                text-align: center;
            }
            h1 {
                background-color: #4CAF50;
                color: white;
                margin: 0;
                padding: 20px;
            }
            .grid-container {
                display: grid;
                grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
                gap: 20px;
                padding: 20px;
                margin: 20px;
            }
            .grid-item {
                background-color: #ffffff;
                border: 2px solid #4CAF50;
                border-radius: 10px;
                box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.1);
                padding: 20px;
                text-align: center;
                font-size: 18px;
                font-weight: bold;
            }
            .value {
                font-size: 24px;
                color: #4CAF50;
            }
        </style>
        <script>
            function fetchData() {
                fetch('/data')
                    .then(response => response.json())
                    .then(data => {
                        document.getElementById('temp-value').innerText = data.temperature + "°C";
                        document.getElementById('humidity-value').innerText = data.humidity + "%";
                        document.getElementById('soil-value').innerText = data.soilMoisture;
                        document.getElementById('light-value').innerText = data.light;
                    })
                    .catch(err => console.log("Error: ", err));
            }
            setInterval(fetchData, 1000); // Update every second
        </script>
    </head>
    <body>
        <h1>ESP32 Sensor Dashboard</h1>
        <div class="grid-container">
            <div class="grid-item">
                Temperature
                <div id="temp-value" class="value">Loading...</div>
            </div>
            <div class="grid-item">
                Humidity
                <div id="humidity-value" class="value">Loading...</div>
            </div>
            <div class="grid-item">
                Soil Moisture
                <div id="soil-value" class="value">Loading...</div>
            </div>
            <div class="grid-item">
                Light Level
                <div id="light-value" class="value">Loading...</div>
            </div>
        </div>
    </body>
    </html>
    )rawliteral";

    server.send(200, "text/html", html);
}
void handleData() {
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["temperature"] = Temperature;
    jsonDoc["humidity"] = Humidity;
    jsonDoc["soilMoisture"] = moisture;
    jsonDoc["light"] = light;

    String jsonString;
    serializeJson(jsonDoc, jsonString);

    server.send(200, "application/json", jsonString);
}


#include <M5Core2.h>
#include <DHT20.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

// Wi-Fi credentials
const char* ssid = "ESP32_AP";
const char* password = "12345678";

// Declare AsyncWebServer instance
AsyncWebServer server(80);

DHT20 dht20;
Adafruit_NeoPixel pixels3(4, 13, NEO_GRB + NEO_KHZ800);

float Temperature = 0;
float Humidity = 0;
uint32_t light = 0;
uint32_t moisture = 0;
  //void TaskBlink(void *pvParameters);
  void TaskTemperatureHumidity(void *pvParameters);
  void TaskSoilMoistureAndRelay(void *pvParameters);
  void TaskLightAndLED(void *pvParameters);
  void TaskPrintLCD(void *pvParameters);
  void TaskPublishToAdafruit(void *pvParameters);
  bool publishToAdafruit(Adafruit_MQTT_Publish &feed, float value);
 
void setup() {
    M5.begin(true, false, true, true, kMBusModeOutput, false);
    dht20.begin();
    pixels3.begin();

    M5.Lcd.setTextSize(2);
    M5.Lcd.fillScreen(BLACK);

    // Initialize Wi-Fi Access Point
    WiFi.softAP(ssid, password);
    Serial.println("Access Point Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    // Serve HTML file from SPIFFS
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/m5core2.html", "text/html");
    });

    // API endpoint to fetch temperature
    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(Temperature));
    });

    // API endpoint to fetch humidity
    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(Humidity));
    });

    // API endpoint to fetch soil moisture
    server.on("/soil", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(moisture));
    });

    // API endpoint to fetch light level
    server.on("/light", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(light));
    });

    // Unified data endpoint
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
        StaticJsonDocument<200> jsonDoc;
        jsonDoc["temperature"] = String(Temperature,2);
        jsonDoc["humidity"] = String(Humidity,2);
        jsonDoc["soilMoisture"] = moisture;
        jsonDoc["light"] = light;

        String jsonString;
        serializeJson(jsonDoc, jsonString);
        request->send(200, "application/json", jsonString);
    });

    // Start the server
    server.begin();
    Serial.println("HTTP server started");

    // Start tasks
    xTaskCreate(TaskTemperatureHumidity, "Task Temperature", 2048, NULL, 2, NULL);
    xTaskCreate(TaskLightAndLED, "Task Light LED", 2048, NULL, 2, NULL);
    xTaskCreate(TaskSoilMoistureAndRelay, "Task Soil & Relay", 2048, NULL, 2, NULL);
    xTaskCreate(TaskPrintLCD, "LCD", 2048, NULL, 2, NULL);
}


void loop() {
    // No code needed in the loop when using AsyncWebServer
}

void TaskTemperatureHumidity(void *pvParameters) {
    while (1) {
        dht20.read();
        Temperature = dht20.getTemperature();
        Humidity = dht20.getHumidity();
        Serial.printf("Temperature: %.2f°C, Humidity: %.2f%%\n", Temperature, Humidity);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void TaskLightAndLED(void *pvParameters) {
    while (1) {
        light = analogRead(34);
        Serial.printf("Light Level: %d\n", light);

        if (light < 700) {
            pixels3.setPixelColor(0, pixels3.Color(0, 0, 255));
            pixels3.setPixelColor(1, pixels3.Color(0, 255, 0));
            pixels3.setPixelColor(2, pixels3.Color(255, 0, 0));
            pixels3.setPixelColor(3, pixels3.Color(128, 0, 128));
            pixels3.show();
        } else {
            pixels3.clear();
            pixels3.show();
        }
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void TaskSoilMoistureAndRelay(void *pvParameters) {
    pinMode(2, OUTPUT);

    while (1) {
        moisture = analogRead(36);
        Serial.printf("Soil Moisture: %d\n", moisture);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void TaskPrintLCD(void *pvParameters) {
    while (1) {
        M5.Lcd.clear();
        M5.Lcd.setCursor(0, 50);
        M5.Lcd.setTextColor(YELLOW);
        M5.Lcd.printf("Light: %d\n", light);

        M5.Lcd.setCursor(0, 100);
        M5.Lcd.setTextColor(YELLOW);
        M5.Lcd.printf("Temperature: %.2f*C\n", Temperature);

        M5.Lcd.setCursor(0, 150);
        M5.Lcd.setTextColor(YELLOW);
        M5.Lcd.printf("Humidity: %.2f%%\n", Humidity);

        M5.Lcd.setCursor(0, 200);
        M5.Lcd.setTextColor(YELLOW);
        M5.Lcd.printf("Moisture: %d\n", moisture);

        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}


