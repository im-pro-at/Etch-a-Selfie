/*
  Etch-a-Selfie
  Copyright (c) 2019 Patrick Knöbel
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050_tockn.h>
#include <esp_camera.h>

//we use GRBL as a library 
#include "src/grbl/grbl.h"


#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

SemaphoreHandle_t wireMutex;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
MPU6050 mpu6050(Wire);

volatile float angleX=0;
volatile float angleY=0;

void readMPUData(void *pvParameter){
  
  for(;;){
    vTaskDelay(10/portTICK_PERIOD_MS  ); //do this 100 times a secound 
    xSemaphoreTake( wireMutex, portMAX_DELAY );    
    mpu6050.update();
    xSemaphoreGive( wireMutex );
    
    angleX=mpu6050.getAngleX();
    angleY=mpu6050.getAngleY();    
  }    
  
}


void setup()
{
  grbl_init();

  Wire.begin(4,15);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) { // Address 0x3D for 128x64
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.display();

  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_1;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM; 
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.jpeg_quality = 2, //0-63 lower number means higher quality
  config.frame_size = FRAMESIZE_SVGA; //800x600 
  config.fb_count = 1;

  // camera init
  Serial.println();
  Serial.println(heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM));
  Serial.printf("camera init ...");
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
  }
  Serial.printf("OK");
  Serial.println();
  Serial.println(heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM));
  sensor_t * s = esp_camera_sensor_get();
    /*
    s->set_framesize(s, (framesize_t)val);
    s->set_quality(s, val);
    s->set_contrast(s, val);
    s->set_brightness(s, val);
    s->set_saturation(s, val);
    s->set_gainceiling(s, (gainceiling_t)val);
    s->set_colorbar(s, val);
    s->set_whitebal(s, val);
    s->set_gain_ctrl(s, val);
    s->set_exposure_ctrl(s, val);
    s->set_hmirror(s, val);
    s->set_vflip(s, val);
    s->set_awb_gain(s, val);
    s->set_agc_gain(s, val);
    s->set_aec_value(s, val);
    s->set_aec2(s, val);
    s->set_dcw(s, val);
    s->set_bpc(s, val);
    s->set_wpc(s, val);
    s->set_raw_gma(s, val);
    s->set_lenc(s, val);
    s->set_special_effect(s, val);
    s->set_wb_mode(s, val);
    s->set_ae_level(s, val);  Serial.printf("OK1");
    */
  Serial.printf("\"framesize\":%u\n", s->status.framesize);
  Serial.printf("\"quality\":%u\n", s->status.quality);
  Serial.printf("\"brightness\":%d\n", s->status.brightness);
  Serial.printf("\"contrast\":%d\n", s->status.contrast);
  Serial.printf("\"saturation\":%d\n", s->status.saturation);
  Serial.printf("\"sharpness\":%d\n", s->status.sharpness);
  Serial.printf("\"special_effect\":%u\n", s->status.special_effect);
  Serial.printf("\"wb_mode\":%u\n", s->status.wb_mode);
  Serial.printf("\"awb\":%u\n", s->status.awb);
  Serial.printf("\"awb_gain\":%u\n", s->status.awb_gain);
  Serial.printf("\"aec\":%u\n", s->status.aec);
  Serial.printf("\"aec2\":%u\n", s->status.aec2);
  Serial.printf("\"ae_level\":%d\n", s->status.ae_level);
  Serial.printf("\"aec_value\":%u\n", s->status.aec_value);
  Serial.printf("\"agc\":%u\n", s->status.agc);
  Serial.printf("\"agc_gain\":%u\n", s->status.agc_gain);
  Serial.printf("\"gainceiling\":%u\n", s->status.gainceiling);
  Serial.printf("\"bpc\":%u\n", s->status.bpc);
  Serial.printf("\"wpc\":%u\n", s->status.wpc);
  Serial.printf("\"raw_gma\":%u\n", s->status.raw_gma);
  Serial.printf("\"lenc\":%u\n", s->status.lenc);
  Serial.printf("\"vflip\":%u\n", s->status.vflip);
  Serial.printf("\"hmirror\":%u\n", s->status.hmirror);
  Serial.printf("\"dcw\":%u\n", s->status.dcw);
  Serial.printf("\"colorbar\":%u\n", s->status.colorbar);
    
  Serial.printf("PID %d",s->id.PID);
  
  
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);  
  wireMutex = xSemaphoreCreateMutex();
  xTaskCreate(
      &readMPUData,       
      "readMPUData",   
      2048,           
      NULL,             
      1,               
      NULL);
  
  
}

void loop()
{

  grbl_service();


  camera_fb_t * fb = esp_camera_fb_get();
  
  if (!fb) {
    Serial.printf("Camera Capture Failed");
  }  
  else{    
  //Serial.printf("\nl=%d w=%d h=%d format=%d\n",fb->len, fb->width,fb->height,fb->format);
    
  uint8_t * rgbbuf = (uint8_t*) heap_caps_calloc(fb->width*fb->height*3, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    
  fmt2rgb888(fb->buf,fb->len,fb->format,rgbbuf);
/*
  char * table = "$@B%8WM#*awmzcvunxr,. ";
  for(size_t y=0;y<fb->height;y+=fb->height/10){
    for(size_t x=0;x<fb->width;x+=fb->width/20){
      float v= rgbbuf[(x+y*fb->width)*3+1]*0.299+ rgbbuf[(x+y*fb->width)*3+1]*0.587 +rgbbuf[(x+y*fb->width)*3+1]*0.114;
      Serial.print(table[(uint8_t)(v/256*21)]);
    }
    Serial.println();      
  }
*/
  free(rgbbuf);
    
  esp_camera_fb_return(fb);    
  }
  
  
  display.clearDisplay();
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print("X: ");
  display.print(angleX);
  display.setCursor(0,10);             // Start at top-left corner
  display.print("Y: ");
  display.print(angleY);
  
  
  xSemaphoreTake( wireMutex, portMAX_DELAY );    
  display.display(); //~30ms
  xSemaphoreGive( wireMutex );
  
  static char buf[100];
  static uint8_t p=0;
  
  while (Serial.available() > 0) {
    char c=Serial.read();
    if(c=='\n'){
      buf[p]=0;
      p=0;
      Serial.println();
      Serial.println();
      Serial.print("RESIVERD: ");
      Serial.println(buf);
      grbl_sendCMD(buf);
      Serial.println();
      Serial.println();
    
      
    }
    else if(c=='?'){
      report_realtime_status(CLIENT_SERIAL);      
    }
    else{
      buf[p++]=c;
    }
  }
  
  delay(1000);
}
