/*
  Etch-a-Selfie
  Copyright (c) 2019 Patrick Knöbel
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050_tockn.h>
#include <Adafruit_ADS1015.h>
#include <esp_camera.h>

//we use GRBL as a library 
#include "src/grbl/grbl.h"
#include "convert.h"


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
Adafruit_ADS1115 ads;

volatile float angleX=0;
volatile float angleY=0;

volatile uint8_t jostick_press=0; //short=1, long=2
volatile int8_t jostick_x=0;
volatile int8_t jostick_y=0;



void i2cWorker(void *pvParameter)
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 100/portTICK_PERIOD_MS; //do this 10 times a secound 
  int16_t adc0, adc1, adc2;
  uint8_t taster_count=0;
  
  xLastWakeTime = xTaskGetTickCount();
  for(;;)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    
    
    xSemaphoreTake( wireMutex, portMAX_DELAY );    
    
    mpu6050.update();    
    adc0 = ads.readADC_SingleEnded(0); 
    adc1 = ads.readADC_SingleEnded(1);
    adc2 = ads.readADC_SingleEnded(2);
    
    xSemaphoreGive( wireMutex );

    angleX=mpu6050.getAngleX();
    angleY=mpu6050.getAngleY();  

    //adc0 Taster open ~ 17000 closed ~10
    if(adc0 < 8000)
    {
      //pressed
      taster_count++;
    }
    else
    {
      // not pressed
      if(taster_count!=0)
      {
        if(taster_count>10)
        { //>1 sec
          jostick_press=2; //long 
        }
        else{
          jostick_press=1; //short 
        }
        taster_count=0;
      }
    }
    
    //adc2 right 1700 middle 8682 left 0
    if(adc2 < 7400)
    {
      jostick_x = -1; //left
    }
    else if(adc2 > 10500)
    {
      jostick_x = 1; //right   
    }
    else{
      jostick_x = 0;            
    }
    //adc1 up ~1700 middle ~9140 down ~0
    if(adc1 < 7400)
    {
      jostick_y = 1; //down
    }
    else if(adc1 > 10500)
    {
      jostick_y = -1; //up     
    }
    else{
      jostick_y = 0;            
    }
    
    Serial.printf("x %d y %d press %d \n",jostick_x,jostick_y,jostick_press);
    
  }    
  
}

void grblRunner(void *pvParameter){
  
  grbl_run();
  
}


void setup()
{
  Serial.begin(2000000);
  
  //run Grbl in a sperate task
  xTaskCreatePinnedToCore(	  grblRunner,    // task
	                            "grblRunner", // name for task
	                            4096,   // size of task stack
	                            NULL,   // parameters
	                            1, // priority
	                            NULL,
                              0
                          );  
  
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
    
    // s->set_framesize(s, (framesize_t)val);
    // s->set_quality(s, val);
    // s->set_contrast(s, val);
    // s->set_brightness(s, val);
    // s->set_saturation(s, val);
    // s->set_gainceiling(s, (gainceiling_t)val);
    // s->set_colorbar(s, val);
    // s->set_whitebal(s, val);
    // s->set_gain_ctrl(s, val);
    // s->set_exposure_ctrl(s, val);
    // s->set_hmirror(s, val);
    // s->set_vflip(s, val);
    // s->set_awb_gain(s, val);
    // s->set_agc_gain(s, val);
    // s->set_aec_value(s, val);
    // s->set_aec2(s, val);
    // s->set_dcw(s, val);
    // s->set_bpc(s, val);
    // s->set_wpc(s, val);
    // s->set_raw_gma(s, val);
    // s->set_lenc(s, val);
    // s->set_special_effect(s, val);
    // s->set_wb_mode(s, val);
    // s->set_ae_level(s, val);  Serial.printf("OK1");
    
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
  
  ads.begin();
  
  wireMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(	  i2cWorker,    // task
	                            "i2cWorker", // name for task
	                            2048,   // size of task stack
	                            NULL,   // parameters
	                            1, // priority
	                            NULL,
	                            1 // core
	                       );    
  
}

uint8_t  *bw;
uint16_t w=750;
uint16_t h=550;


void statusCallback(const char *name, uint8_t percent){
  Serial.printf("SCB:[%3d] %s \n",percent,name);
}
void gcodeCallback(const char *gcode){
  Serial.printf("GCODE: %s \n",gcode);
  grbl_putString(gcode);
}


void loop() //runs on Core 1
{
  char * table = "$@B%8WM#*awmzcvunxr,. ";

  //*
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.printf("Camera Capture Failed");
  }  
  else{    

    Serial.printf("\nl=%d w=%d h=%d format=%d\n",fb->len, fb->width,fb->height,fb->format);
      
    uint8_t * rgbbuf = (uint8_t*) heap_caps_calloc(fb->width*fb->height*3, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    uint8_t  *gray    =(uint8_t*) heap_caps_calloc(w*h, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    uint16_t *buffer1 =(uint16_t*)heap_caps_calloc(w*h*2, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    uint16_t *buffer2 =(uint16_t*)heap_caps_calloc(w*h*2, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    bw      =(uint8_t*) heap_caps_calloc(w*h/4+1, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    uint8_t  *done    =(uint8_t*) heap_caps_calloc(w*h/8+1, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
      
    if(rgbbuf==NULL ||gray==NULL ||buffer1==NULL ||buffer2==NULL ||bw==NULL ||done==NULL)
    {
      Serial.println("!!!!!!!!!!!!!!!!!!MALLOC ERROR!!!!!!!!!!!!!!!!!!!");
      delay(1000);
    }

    Serial.println("JEPAG IMAGE DATA: ");
    Serial.print("var image=[");
    for(uint32_t i=0;i<fb->len;i++){
      Serial.print(fb->buf[i]);
      if(i!=fb->len-1)Serial.print(",");
      
    }
    Serial.println("]");
    Serial.println("var binary=\"\";");
    Serial.println("for(var i=0;i<image.length;i++){");
    Serial.println("    binary+=String.fromCharCode(image[i])");
    Serial.println("}");
    Serial.println("var base64=window.btoa(binary)");
    Serial.println("$(\"#image\")[0].src=\"data:image/jpeg;base64,\"+base64    ");

    fmt2rgb888(fb->buf,fb->len,fb->format,rgbbuf);

    
    convert_RGB2Grayscale(fb->width, fb->height, rgbbuf, w, h, gray);
    
    convert_XDOG_init(w,h, statusCallback, gray, buffer1, buffer2);
    
    for(float gamma=0; gamma<1 ; gamma+=0.1){
      for(float phi=0; phi<1 ; phi+=0.1){
        Serial.printf("gamma %f phi %f \n",gamma,phi);
        convert_XDOG(w, h, gamma, phi, buffer1, buffer2, bw);
        
        //dsplay
        display.clearDisplay();
        
        uint8_t scale = max(w/128+1,h/64+1);
        for(uint8_t x=0;x<128;x++) 
        {
          for(uint8_t y=0;y<64;y++) 
          {
            uint16_t count=0;
            for(uint8_t dx=0;dx<scale;dx++)
            {
              for(uint8_t dy=0;dy<scale;dy++)
              {
                if((x*scale+dx)<w && (y*scale+dy)<h)
                {
                  uint32_t pos= x*scale+dx + (y*scale+dy)*w;
                  if((bw[pos/4] >> ((pos%4)*2)) & 0b11)
                  {
                    count ++;
                  }                    
                }
              }              
            }
            if(count > (scale*scale)/4) 
            {
              display.drawPixel(x,y,WHITE);                          
            }
          }          
        }
                
        
        xSemaphoreTake( wireMutex, portMAX_DELAY );    
        display.display(); //~30ms
        xSemaphoreGive( wireMutex );

        
      }      
    }

    
    for(uint16_t y=0;y<h;y++){
      Serial.print("_");      
      for(uint16_t x=0;x<w;x++){
        uint32_t pos =(x+y*w);
        switch((bw[pos/4] >> ((pos%4)*2)) & 0b11){
          case 0:
            Serial.print(' ');
            break;
          case 1:
            Serial.print('.');
            break;
          case 2:
            Serial.print('*');
            break;
          case 3:
            Serial.print('#');
            break;
        }
      }
      Serial.println();      
    }
    
    convert_connect(w, h, statusCallback, bw, (uint32_t *) buffer1);

    for(uint16_t y=0;y<h;y++){
      Serial.print("_");      
      for(uint16_t x=0;x<w;x++){
        uint32_t pos =(x+y*w);
        switch((bw[pos/4] >> ((pos%4)*2)) & 0b11){
          case 0:
            Serial.print(' ');
            break;
          case 1:
            Serial.print('.');
            break;
          case 2:
            Serial.print('*');
            break;
          case 3:
            Serial.print('#');
            break;
        }
      }
      Serial.println();      
    }

    grbl_putString("G1 F10000");
    convert_etch(w, h, statusCallback, gcodeCallback, bw, done, buffer1, (uint32_t *)buffer2, w*h/2);

    for(uint16_t y=0;y<h;y++){
      Serial.print("_");      
      for(uint16_t x=0;x<w;x++){
        uint32_t pos =(x+y*w);
        switch((bw[pos/4] >> ((pos%4)*2)) & 0b11){
          case 0:
            Serial.print(' ');
            break;
          case 1:
            Serial.print('.');
            break;
          case 2:
            Serial.print('*');
            break;
          case 3:
            Serial.print('#');
            break;
        }
      }
      Serial.println();      
    }
    
    free(done);
    free(bw);
    free(buffer2);
    free(buffer1);
    free(gray);
    free(rgbbuf);
      
    esp_camera_fb_return(fb);    
  }
  //*/
  display.clearDisplay();
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print("X: ");
  display.print(angleX);
  Serial.println(angleX);
  display.setCursor(0,10);             // Start at top-left corner
  display.print("Y: ");
  display.print(angleY);
  Serial.println(angleY);
  
  
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
      grbl_putString(buf);
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
