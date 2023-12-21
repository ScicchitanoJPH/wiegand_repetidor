#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <string.h>
#include <esp_log.h>
#include <driver/gpio.h>

void initEncoder(gpio_num_t gpio_0, gpio_num_t gpio_1);
void encoderWiegand(uint32_t valor, gpio_num_t gpio_0, gpio_num_t gpio_1, uint8_t cantidadBits);
void encoderWiegandBits(const uint8_t *valor,size_t longitudBits, gpio_num_t gpio_0, gpio_num_t gpio_1,uint32_t delayPulso_us, uint32_t delayIntervalo_us,const char *TAG);

#endif  // ENCODER_H