#include <Arduino.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>   
#include <ESP8266HTTPClient.h>  
#include <ArduinoJson.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

const char* ssid     = "KDU IRC 1986";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "monkeys2";
const char* url = "http://api.openweathermap.org/data/2.5/weather?lat=6.89&lon=79.99&appid=e41b37f5ed570a8d6949b45d9fdaa36f";
WiFiClient client;


U8G2_SSD1306_128X64_NONAME_F_SW_I2C disp1(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);  
U8G2_SSD1306_128X64_NONAME_F_SW_I2C disp2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);  

void setup(void) {

  disp1.setI2CAddress(0x7A);
  disp1.begin(); 
  disp2.setI2CAddress(0x78); 
  disp2.begin();
  
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
  
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }


  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());  
}


void loop(void) {

  if (WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    http.begin(client, url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();

      // Parse JSON
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      float temperatureKel = doc["main"]["temp"];
      String weather = doc["weather"][0]["description"];
      float temperature = (temperatureKel - 273.15);

      Serial.println(weather);
      updateScreen(temperature,weather);
    }
    http.end();
  }
  delay(1000);
}

void updateScreen(float temp, String weath){

  disp1.clearBuffer();
  disp1.setFont(u8g2_font_scrum_tf);
  disp1.drawStr(16,18, "Temperature:");
  disp1.setFont(u8g2_font_calibration_gothic_nbp_t_all);
  disp1.setCursor(46,44);
  disp1.print(temp);
  disp1.drawGlyph(86,44, 176);
  disp1.drawStr(92,44, "C");
  disp1.sendBuffer();

  disp2.clearBuffer();
  disp2.setFont(u8g2_font_scrum_tf);
  disp2.drawStr(20,18, "Weather:");
  disp2.setFont(u8g2_font_scrum_tf);
  int y = 35;
  for (int i = 0; i < weath.length(); i += 9) {
    String line = weath.substring(i, i + 9);
    disp2.setCursor(24, y);
    disp2.print(line);
    y += 18;
  }
  disp2.sendBuffer();
}


