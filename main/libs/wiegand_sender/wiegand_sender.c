#include "wiegand_sender.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_system.h"

#define WD_W2_delayPulso_us 100  // milisegundos
#define WD_W2_delayIntervalo_us 5000  // milisegundos

static gpio_num_t gpio_0_;
static gpio_num_t gpio_1_;

void initEncoder(gpio_num_t gpio_0, gpio_num_t gpio_1) {
    gpio_0_ = gpio_0;
    gpio_1_ = gpio_1;

    // Inicializo pines GPIO como salidas Wiegand
    gpio_set_direction(gpio_0_, GPIO_MODE_OUTPUT);
    gpio_set_direction(gpio_1_, GPIO_MODE_OUTPUT);

    // Asigno un valor lógico 1 a los pines de salida Wiegand
    gpio_set_level(gpio_0_, 1);
    gpio_set_level(gpio_1_, 1);
}

void encoderWiegand(uint32_t valor, gpio_num_t gpio_0, gpio_num_t gpio_1, uint8_t cantidadBits) {
    uint8_t i = 0;

    char variable[33];
    itoa(valor, variable, 2);
    char *variablePadded = variable;
    while (strlen(variablePadded) < cantidadBits) {
        variablePadded = strcat("0", variablePadded);
    }

    // Envío código Wiegand
    while (i < cantidadBits) {
        if (variablePadded[i] == '0') {
            //printf('0');
            gpio_set_level(gpio_0, 0);
            esp_rom_delay_us(100);
            //vTaskDelay(pdMS_TO_TICKS(WD_W2_delayPulso));
            gpio_set_level(gpio_0, 1);
            esp_rom_delay_us(5000);
            //vTaskDelay(pdMS_TO_TICKS(WD_W2_delayIntervalo));
        } else {
            //printf('1');
            gpio_set_level(gpio_1, 0);
            //vTaskDelay(pdMS_TO_TICKS(WD_W2_delayPulso));
            esp_rom_delay_us(100);
            gpio_set_level(gpio_1, 1);
            esp_rom_delay_us(5000);
            //vTaskDelay(pdMS_TO_TICKS(WD_W2_delayIntervalo));
        }

        i++;
    }
}


void recorrerCharArray(char *valor) {
    for (int i = 0; valor[i] != '\0'; ++i) {
        // Accede a cada elemento con valor[i]
        char caracterActual = valor[i];
        // Haz algo con el caracter, por ejemplo, imprimirlo
        printf("%c ", caracterActual);
    }
}


void encoderWiegandBits(const uint8_t *valor,size_t longitudBits, gpio_num_t gpio_0, gpio_num_t gpio_1,const char *TAG) {

    
    ESP_LOGE(TAG, "====================================");
    ESP_LOGE(TAG, "Bits sended: %d\n", longitudBits);

    ESP_LOGE(TAG, "Data sended : ");
    for (size_t i = 0; i < longitudBits; i++) {
        // Accede a cada bit en la valor usando valor[i]
        uint8_t bitActual = valor[i];
        ESP_LOGE(TAG, "%d", bitActual);
        gpio_set_level(gpio_0_, 1);
        gpio_set_level(gpio_1_, 1);
        // Procesa el bit, por ejemplo, imprímelo
        if (bitActual == 0) {
            gpio_set_level(gpio_0, 0);
            esp_rom_delay_us(WD_W2_delayPulso_us);
            gpio_set_level(gpio_0, 1);
            esp_rom_delay_us(WD_W2_delayIntervalo_us);
        } else {
            gpio_set_level(gpio_1, 0);
            esp_rom_delay_us(WD_W2_delayPulso_us);
            gpio_set_level(gpio_1, 1);
            esp_rom_delay_us(WD_W2_delayIntervalo_us);
        }
    }
    ESP_LOGE(TAG, "====================================");
}
