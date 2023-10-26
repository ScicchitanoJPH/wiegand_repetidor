#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <driver/gpio.h>
#include <string.h>

void initEncoder(gpio_num_t gpio_0, gpio_num_t gpio_1);
void encoderWiegand(uint32_t valor, gpio_num_t gpio_0, gpio_num_t gpio_1, uint8_t cantidadBits);
void encoderWiegandBits(const char *valor, gpio_num_t gpio_0, gpio_num_t gpio_1);

#endif  // ENCODER_H