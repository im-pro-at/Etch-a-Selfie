/*
  Etch-a-Selfie
  Copyright (c) 2019 Patrick Knöbel
*/



//we use GRBL as a library 
#include "src/grbl/grbl.h"

#include <Wire.h>
#include "src/Adafruit_GFX_Library/Adafruit_GFX.h"
#include "src/Adafruit_SSD1306/Adafruit_SSD1306.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);



void setup()
{
  grbl_init();
  Wire.begin(16,15);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) { // Address 0x3D for 128x64
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.display();
  delay(2000); // Pause for 2 seconds
}

void loop()
{
  grbl_service();
  
  //grbl_sendCMD("G0F10X0Y0");

  
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Hello, world!"));
  display.display();

  delay(1000);
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(BLACK);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Hello, world!"));
  display.display();
  
  delay(1000);
}
