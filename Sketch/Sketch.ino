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

#define SCREEN_WIDTH  128 // OLED display width, in pixels
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

void updateDisplay()
{
  xSemaphoreTake( wireMutex, portMAX_DELAY );    
  display.display(); //~30ms
  xSemaphoreGive( wireMutex );
}

void printError(const char *error)
{
  Serial.printf("Error: %s \n",error);
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
 
  display.setCursor(30,5);
  display.print("ERROR:");

  display.setTextSize(1);
  display.setCursor(0,30);

  display.print(error);
  
  updateDisplay();
    
  delay(5000);
  
  ESP.restart();
}


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
    
  }    
  
}

void grblRunner(void *pvParameter){
  
  grbl_run();
  
}


void setup()
{
  Serial.begin(2000000);
    
  wireMutex = xSemaphoreCreateMutex();
  
  Wire.begin(4,15);

  while(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) { // Address 0x3D for 128x64
    Serial.println("SSD1306 allocation failed");
    delay(1000);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
 
  display.setCursor(30,20);
  display.print("INIT...");
  updateDisplay();
  
  //run Grbl in a sperate task
  xTaskCreatePinnedToCore
    (	  
      grblRunner,    // task
      "grblRunner", // name for task
      4096,   // size of task stack
      NULL,   // parameters
      1, // priority
      NULL,
      0
    );  
  
  delay(1000); //wait for grbl to start
  
  
  
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
    char buffer[100];
    snprintf(buffer, 100, "Camera init failed with error 0x%x", err );      
    printError(buffer);
  }
  Serial.printf("OK");
  Serial.println();
  Serial.println(heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM));
  sensor_t * s = esp_camera_sensor_get();
  
  if(s==NULL)
  {
    printError("Camera Sensor not found!");    
  }
    
  s->set_hmirror(s, 1);
  s->set_vflip(s, 1);
        
  Serial.printf("PID %d",s->id.PID);
  
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);  
  
  ads.begin();
  
  xTaskCreatePinnedToCore
    (	  
      i2cWorker,    // task
      "i2cWorker", // name for task
      2048,   // size of task stack
      NULL,   // parameters
      1, // priority
      NULL,
      1 // core
    );    
  
}



void loop() //runs on Core 1
{  
  static uint8_t marked=0;
  //    0  |  1
  //  -----|----
  //    2  |  3
  
  if(jostick_y==1 && marked==0)  marked=2;
  if(jostick_y==1 && marked==1)  marked=3;
  if(jostick_y==-1&& marked==2)  marked=0;
  if(jostick_y==-1&& marked==3)  marked=1;
  if(jostick_x==1 && marked==0)  marked=1;
  if(jostick_x==1 && marked==2)  marked=3;
  if(jostick_x==-1&& marked==1)  marked=0;
  if(jostick_x==-1&& marked==3)  marked=2;
  
  if(jostick_press==1) 
  {
    jostick_press=0;
    switch(marked)
    {
      case 0: //Calibration
        break;
      case 1: //Selfie
        takeAselfie();
        break;
      case 2: //Jostick
        break;
      case 3: //Acceloration
        break;
    }
  }
  if(jostick_press==2) jostick_press=0;
  
  
  //draw menue:
  display.clearDisplay();
  display.setTextColor(WHITE); 
  display.setTextSize(1);

  display.drawLine(0,SCREEN_HEIGHT/2,SCREEN_WIDTH,SCREEN_HEIGHT/2,WHITE);
  display.drawLine(SCREEN_WIDTH/2,0,SCREEN_WIDTH/2,SCREEN_HEIGHT,WHITE);

  
  display.setCursor(0,12);        
  display.print("   Cal.   ");
  display.setCursor(SCREEN_WIDTH/2,12);        
  display.print("  Selfie  ");
  display.setCursor(0,SCREEN_HEIGHT/2+12);        
  display.print("  Jostick ");
  display.setCursor(SCREEN_WIDTH/2,SCREEN_HEIGHT/2+12);        
  display.print("   Hand   ");
  
  
  display.fillRect((marked%2==0)?0:(SCREEN_WIDTH/2+1),(marked<2)?0:(SCREEN_HEIGHT/2+1),SCREEN_WIDTH/2,SCREEN_HEIGHT/2,INVERSE);
  
  updateDisplay();
  
  delay(100);
  
}

void doGammaPhi(char * s1, char *s2, float *gamma, float *phi)
{
  display.setTextColor(WHITE); 
  display.setTextSize(1);
  display.setCursor(93,5); 
  display.print(s1);
  display.setCursor(93,13); 
  display.print(s2);
  
  display.setCursor(90,26); // up down
  display.print((char)24);
  display.setCursor(90,26+3);
  display.print((char)25);
  display.setCursor(102,27); 
  display.print((*gamma));
  
  (*gamma)+= jostick_y*0.01;    
  if((*gamma)<0)  (*gamma)=0;
  if((*gamma)>1)  (*gamma)=1;

  display.setCursor(90,46); // left right
  display.print((char)27);
  display.setCursor(90+3,46); 
  display.print((char)26);
  display.setCursor(102,46); 
  display.print((*phi));
  
  (*phi)+= jostick_x*0.01;    
  if((*phi)<0)  (*phi)=0;
  if((*phi)>1)  (*phi)=1;
  
}

uint8_t progressCallBack(const char *name, uint8_t percent)
{
  static uint8_t lastpercent=101;
  display.clearDisplay();
  if(jostick_press==2)
  {
    //canceling
    display.setTextColor(WHITE); 
    display.setTextSize(2);
    display.setCursor(5,20);
    display.print("Canceling");
    updateDisplay();
    return 1;
  }
  if(lastpercent==percent) 
  {
    return 1;
  }
  lastpercent=percent;
  display.setTextColor(WHITE); 
  display.setTextSize(2);
  display.setCursor(5,5);
  display.print(name);
  
  display.drawRect(5,SCREEN_HEIGHT-5-20,SCREEN_WIDTH-5-5,20,WHITE);
  display.fillRect(5,SCREEN_HEIGHT-5-20,(SCREEN_WIDTH-5-5)*percent/100,20,WHITE);

  display.setTextColor(INVERSE); 
  display.setTextSize(2);
  display.setCursor(SCREEN_WIDTH/2-5 -(percent>=10?5:0)-(percent>=100?5:0),SCREEN_HEIGHT-5-18);
  display.print(percent);

  updateDisplay();
  
  return 0;
}

void takeAselfie()
{
  for(int i=0;i<=100;i++)
  {
    progressCallBack("TEST..",i);
    delay(100);
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE); 
  display.setTextSize(2);
  display.setCursor(30,20);
  display.print("SELFIE");
  updateDisplay();

  //Preview Mode => lower resolution
  sensor_t *s = esp_camera_sensor_get();
  if(s==NULL)
  {
    printError("Camera Sensor not found!");    
  }
  s->set_framesize(s, FRAMESIZE_QQVGA);    //160x120
  delay(1000);
  
  uint16_t w=120; 
  uint16_t h=88;      
  uint8_t  *rgbbuf  =(uint8_t*) heap_caps_calloc(160*120*3, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  uint8_t  *gray    =(uint8_t*) heap_caps_calloc(w*h, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  uint16_t *buffer1 =(uint16_t*)heap_caps_calloc(w*h*2, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  uint16_t *buffer2 =(uint16_t*)heap_caps_calloc(w*h*2, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  uint8_t  *bw      =(uint8_t*) heap_caps_calloc(w*h/4+1, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  float gamma= 0.8;
  float phi= 0.4;
  
  if(rgbbuf==NULL ||gray==NULL ||buffer1==NULL ||buffer2==NULL ||bw==NULL)
  {
      printError("Malloc");
  }
  while(1)
  {
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb==NULL || fb->width!=120 || fb->height!=88) {
      printError("Camera Capture Failed");
    }  
    Serial.printf("\nl=%d w=%d h=%d format=%d\n",fb->len, fb->width,fb->height,fb->format);

    if(!fmt2rgb888(fb->buf,fb->len,fb->format,rgbbuf))
    {
      printError("JPEG decoder error");      
    }

    convert_RGB2Grayscale(fb->width, fb->height, rgbbuf, w, h, gray);
    
    convert_XDOG_init(w,h, NULL, gray, buffer1, buffer2);
    
    convert_XDOG(w, h, gamma, phi, buffer1, buffer2, bw);
    
    
    //dsplay
    display.clearDisplay();
    uint32_t scale= 256*64/h;
    for(uint8_t x=0;x<w;x++) 
    {
      for(uint8_t y=0;y<h;y++) 
      {
        uint32_t pos= x+y*w;
        if((bw[pos/4] >> ((pos%4)*2)) & 0b11)
        {
          display.drawPixel(x*scale/256,y*scale/256,WHITE);                          
        }                    
      }          
    }
    
    doGammaPhi("Pre","View",&gamma,&phi);
    
    updateDisplay();
    
    esp_camera_fb_return(fb); 

    if(jostick_press){
      break;
    } 
  }    
  free(bw);
  free(buffer2);
  free(buffer1);
  free(gray);
  free(rgbbuf);

  if(jostick_press==2){
    jostick_press=0;
    return;
  }
  jostick_press=0;
  
  s->set_framesize(s, FRAMESIZE_SVGA);    //800*600

  for(int i=3;i>0;i--)
  {
    display.clearDisplay();
    display.setTextColor(WHITE); 
    display.setTextSize(2);
    display.setCursor(30,5);
    display.print("PHOTO");
    display.setCursor(60,29);
    display.print(i);
    updateDisplay();
    delay(1000);
  }

  camera_fb_t *fb = esp_camera_fb_get();
  if (fb==NULL || fb->width!=800 || fb->height!=600) {
    printError("Camera Capture Failed");
  }  
  Serial.printf("\nl=%d w=%d h=%d format=%d\n",fb->len, fb->width,fb->height,fb->format);
    
  w=600; //150mm *4
  h=440; //110mm *4
  rgbbuf          =(uint8_t*) heap_caps_calloc(fb->width*fb->height*3, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  gray            =(uint8_t*) heap_caps_calloc(w*h, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  buffer1         =(uint16_t*)heap_caps_calloc(w*h*2, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  buffer2         =(uint16_t*)heap_caps_calloc(w*h*2, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  bw              =(uint8_t*) heap_caps_calloc(w*h/4+1, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  uint8_t  *done  =(uint8_t*) heap_caps_calloc(w*h/8+1, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
      
  if(rgbbuf==NULL ||gray==NULL ||buffer1==NULL ||buffer2==NULL ||bw==NULL ||done==NULL)
  {
      printError("Malloc");
  }
  
  

  
}
/*
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
  camera_fb_t *fb;
  sensor_t *s;
  
  s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QQVGA);    //160x120
  delay(100);
  uint64_t start_time = esp_timer_get_time();
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.printf("Camera Capture Failed");
  }  
  else{
      Serial.printf("\nl=%d w=%d h=%d format=%d\n",fb->len, fb->width,fb->height,fb->format);

      uint16_t w=120;//87;  //750/550=1.36*64=87
      uint16_t h=88;//64;      
      uint8_t  *rgbbuf  =(uint8_t*) heap_caps_calloc(fb->width*fb->height*3, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
      uint8_t  *gray    =(uint8_t*) heap_caps_calloc(w*h, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
      uint16_t *buffer1 =(uint16_t*)heap_caps_calloc(w*h*2, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
      uint16_t *buffer2 =(uint16_t*)heap_caps_calloc(w*h*2, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
      uint8_t  *bw      =(uint8_t*) heap_caps_calloc(w*h/4+1, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
      
      if(rgbbuf==NULL ||gray==NULL ||buffer1==NULL ||buffer2==NULL ||bw==NULL)
      {
        Serial.println("!!!!!!!!!!!!!!!!!!MALLOC ERROR!!!!!!!!!!!!!!!!!!!");
        Serial.printf("rgbbuf %x gray %x buffer1 %x buffer1 %x bw %x /n",rgbbuf,gray,buffer1,buffer2,bw);
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
      
      convert_XDOG_init(w,h, NULL, gray, buffer1, buffer2);
      
      convert_XDOG(w, h, 0.8, 0.4, buffer1, buffer2, bw);
      
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
      
      //dsplay
      display.clearDisplay();
      uint32_t scale= 256*64/h;
      for(uint8_t x=0;x<w;x++) 
      {
        for(uint8_t y=0;y<h;y++) 
        {
          uint32_t pos= x+y*w;
          if((bw[pos/4] >> ((pos%4)*2)) & 0b11)
          {
            display.drawPixel(x*scale/256,y*scale/256,WHITE);                          
          }                    
        }          
      }
              
      display.setCursor(90,0); 
      display.print("1");
      
      xSemaphoreTake( wireMutex, portMAX_DELAY );    
      display.display(); //~30ms
      xSemaphoreGive( wireMutex );

      free(bw);
      free(buffer2);
      free(buffer1);
      free(gray);
      free(rgbbuf);

      esp_camera_fb_return(fb);    

      uint64_t fb_get_time = esp_timer_get_time();
      Serial.printf("one frame in %u ms.\n", (fb_get_time - start_time) / 1000); // this line can be commented out
      
     
      
  }  
  
    
  
  s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_SVGA);    //800*600
  delay(100);
  
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.printf("Camera Capture Failed");
  }  
  else{    

    Serial.printf("\nl=%d w=%d h=%d format=%d\n",fb->len, fb->width,fb->height,fb->format);
      
    uint16_t w=600; //150mm *4
    uint16_t h=440; //110mm *4
    uint8_t * rgbbuf = (uint8_t*) heap_caps_calloc(fb->width*fb->height*3, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    uint8_t  *gray    =(uint8_t*) heap_caps_calloc(w*h, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    uint16_t *buffer1 =(uint16_t*)heap_caps_calloc(w*h*2, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    uint16_t *buffer2 =(uint16_t*)heap_caps_calloc(w*h*2, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    uint8_t *bw      =(uint8_t*) heap_caps_calloc(w*h/4+1, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    uint8_t  *done    =(uint8_t*) heap_caps_calloc(w*h/8+1, 1, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
      
    if(rgbbuf==NULL ||gray==NULL ||buffer1==NULL ||buffer2==NULL ||bw==NULL ||done==NULL)
    {
      
      Serial.println("!!!!!!!!!!!!!!!!!!MALLOC ERROR!!!!!!!!!!!!!!!!!!!");
      Serial.printf("rgbbuf %x gray %x buffer1 %x buffer1 %x bw %x done %x /n",rgbbuf,gray,buffer1,buffer2,bw,done);
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
    
    //for(float gamma=0; gamma<1 ; gamma+=0.1){
    //  for(float phi=0; phi<1 ; phi+=0.1){
        //Serial.printf("gamma %f phi %f \n",gamma,phi);
        //convert_XDOG(w, h, gamma, phi, buffer1, buffer2, bw);
        convert_XDOG(w, h, 0.8, 0.4, buffer1, buffer2, bw);
        
        
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
                
        display.setCursor(90,0); 
        display.print("2");
        
        xSemaphoreTake( wireMutex, portMAX_DELAY );    
        display.display(); //~30ms
        xSemaphoreGive( wireMutex );

        
    //  }      
    //}
    
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
*/