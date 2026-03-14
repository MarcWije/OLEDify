#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFi.h>   

#define DISABLE_AUDIOBOOKS
#define DISABLE_CATEGORIES
#define DISABLE_CHAPTERS
#define DISABLE_EPISODES
#define DISABLE_GENRES
#define DISABLE_MARKETS
#define DISABLE_PLAYLISTS
#define DISABLE_SEARCH
#define DISABLE_SHOWS

#include "SpotifyEsp32.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

const char* ssid     = "SLT_FIBRE";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "wijesuriyawifi";
const char* clientID = "1abcfdb77ff9400eb41b624e590d12d4";
const char* clientSecret = "ae99f25f0ee64f5fb94677a38ed1d63b";
const char* refreshToken = "AQADzf6hsO7mILLuubcgxkhTL84SPA4b3PgOGK6jFX0k16Qs_dG9G6eN6xQTRNURLV4rNhYCAts81qxpr4u-C0gIQ_ypHmZnXHg8aJiTOx-i9MV0cioeHmJSMWsv3w3Ojl4";

int xPin = 34;        
int yPin = 35;
int xValue = 0; 
int yValue = 0; 
bool isXTilted = false;
bool isYTilted = false;
bool isRight = false;
bool isUp = false;
int tiltStart = 0;
int pressDuration = 0;

Spotify sp(clientID,clientSecret, refreshToken);

// U8G2_SSD1306_128X64_NONAME_F_HW_I2C disp1(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8X8_SSD1306_128X64_NONAME_HW_I2C disp1(/* reset=*/ U8X8_PIN_NONE); 
//U8G2_SH1106_128X64_NONAME_F_HW_I2C disp2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8X8_SH1106_128X64_NONAME_HW_I2C disp2(/* reset=*/ U8X8_PIN_NONE);


void setup(void) {

  disp1.setI2CAddress(0x7A);
  disp1.begin(); 
  disp2.setI2CAddress(0x78); 
  disp2.begin();

  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  WiFi.mode(WIFI_STA);

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
  sp.begin();

  while (!sp.is_auth()) {
    sp.handle_client(); // Required for receiving the authorization code
  }

}

unsigned long lastSpotifyCheck = 0;

void loop() {
  // put your main code here, to run repeatedly:
  static String lastArtist;
  static String lastTrackname;

  xValue = analogRead(xPin);
  yValue = analogRead(yPin);
  buttonPoll(xValue, isXTilted, true);
  buttonPoll(yValue, isYTilted, false);
  
  if (millis() - lastSpotifyCheck > 2000) {
    lastSpotifyCheck = millis();
    String currentArtist = sp.current_artist_names();
    String currentTrackname = sp.current_track_name();

    if (lastArtist != currentArtist && currentArtist != "Something went wrong") {
        disp1.clearDisplay();
        lastArtist = currentArtist;
        Serial.print("Artist: ");
        Serial.print(currentArtist);
        Serial.println("");
        disp1.setFont(u8x8_font_chroma48medium8_r);
        disp1.drawString(0, 2,lastArtist.c_str());
    }
    
    if (lastTrackname != currentTrackname && currentTrackname != "Something went wrong") {
        disp2.clearDisplay();
        lastTrackname = currentTrackname;
        Serial.print("Track: ");
        Serial.print(currentTrackname);
        Serial.println("");
        disp2.setFont(u8x8_font_chroma48medium8_r);
        disp2.drawString(0, 2,lastTrackname.c_str());
    }
  }
}

void buttonPoll(int value, bool isTilted, bool x){
  if ((value > 2500 || value < 1200) && isTilted == false){
    tiltStart = millis();
    if (x){ // Checks if input is from X Axis or Y Axis
      isXTilted = true;
      isRight = value > 2000;
    } else {
      isYTilted = true;
      isUp = value > 2000;
    }
  }

  if ((value > 1400 && value < 2100) && isTilted == true){
    if (x){
      printInput(checkInput(isRight, millis() - tiltStart, x)); 
      isXTilted = false;
      isRight = false; 
    } else {
      printInput(checkInput(isUp, millis() - tiltStart, x)); 
      isYTilted = false;
      isUp = false;
    }
    tiltStart = 0;

  }
}

int checkInput(bool isRight, int tiltDuration, bool x){
  int input = 0;
  if (x){
    if (tiltDuration > 3000){
      if (isRight){
        input = 1; // long right
      } else {
        input = 2; // long left
      }
    } else {
      if (isRight){
        input = 3; // short right
      } else {
        input = 4; // short left
      }
    }
  } else {
    if (tiltDuration > 3000){
      if (isRight){
        input = 5; // long up
      } else {
        input = 6; // long down
      }
    } else {
      if (isRight){
        input = 7; // short up
      } else {
        input = 8; // short down
      }
    }
  }
  return input;
}

void printInput(int input){
  switch (input) {
    case 1:{
      Serial.println("Long Right");
      JsonDocument filter;
      filter["progress_ms"] = true;
      response response = sp.current_playback_state(filter);
      int timestamp = response.reply["progress_ms"];
      sp.seek_to_position(timestamp + 10000);
      break;
    }
    case 2: {
      Serial.println("Long Left");
      JsonDocument filter;
      filter["progress_ms"] = true;
      response response = sp.current_playback_state(filter);
      int timestamp = response.reply["progress_ms"];
      sp.seek_to_position(timestamp - 10000);
      break;
    }
    case 3: 
      Serial.println("Short Right");
      sp.skip();
      break;
    case 4:
      Serial.println("Short Left");
      sp.previous();
      break;
    case 5:
      Serial.println("Long Up");
      sp.set_volume(100);
      break;
    case 6:
      Serial.println("Long Down");
      sp.set_volume(20);
      break;
    case 7:
      Serial.println("Short Up");
      break;
    case 8:
      Serial.println("Short Down");
      break;
    default: 
      Serial.println("No input");
  }
}
