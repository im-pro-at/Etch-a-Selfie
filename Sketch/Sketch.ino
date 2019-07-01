/*
  Etch-a-Selfie
  Copyright (c) 2019 Patrick Knöbel
*/

//we use GRBL as a library 
#include "src/grbl/grbl.h"

void setup()
{
    grbl_init();
}

void loop()
{
  grbl_service();
  
  //grbl_sendCMD("G0F10X0Y0");
  
}