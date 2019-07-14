/*
  Etch-a-Selfie
  Copyright (c) 2019 Patrick Knöbel
*/

#ifndef convert_h
#define convert_h

#include <stdint.h>

typedef void (*convert_statuscallback_t)(const char *name, uint8_t *percent);

void convert_RGB2Grayscale(uint16_t w, uint16_t h, uint8_t *rgb, uint8_t *gray);

void convert_XDOG_init(uint16_t w, uint16_t h, convert_statuscallback_t callback, uint8_t *gray, uint16_t *buffer1, uint16_t *buffer2);

void convert_XDOG(uint16_t w, uint16_t h, float gamma, float phi, uint16_t *buffer1, uint16_t *buffer2, uint8_t *bw);



#endif