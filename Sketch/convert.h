/*
  Etch-a-Selfie
  Copyright (c) 2019 Patrick Knöbel
*/

#ifndef convert_h
#define convert_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t (*convert_statuscallback_t)(const char *name, uint8_t percent);
typedef void (*convert_movecallback_t)(int16_t x, int16_t y);

uint8_t convert_RGB2Grayscale(uint16_t rgb_w, uint16_t rgb_h, uint8_t *rgb, uint16_t gray_w, uint16_t gray_h, uint8_t *gray);

uint8_t convert_XDOG_init(uint16_t w, uint16_t h, convert_statuscallback_t callback, uint8_t *gray, uint16_t *buffer1, uint16_t *buffer2);

uint8_t convert_XDOG(uint16_t w, uint16_t h, float gamma, float phi, uint16_t *buffer1, uint16_t *buffer2, uint8_t *bw);

uint8_t convert_connect(uint16_t w, uint16_t h, convert_statuscallback_t callback, uint8_t *bw, uint32_t *stack);

uint8_t convert_etch(uint16_t w, uint16_t h, convert_statuscallback_t statuscb, convert_movecallback_t movecb, uint8_t *bw, uint8_t *done, uint16_t * cost, uint32_t *discoverd, uint32_t discoverd_length);

#ifdef __cplusplus
} // extern "C"
#endif

#endif