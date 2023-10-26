#include "wiegand_sender.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define WD_W2_delayPulso 50  // milisegundos
#define WD_W2_delayIntervalo 2  // milisegundos

static gpio_num_t gpio_0_;
static gpio_num_t gpio_1_;

void initEncoder(gpio_num_t gpio_0, gpio_num_t gpio_1) {
    gpio_0_ = gpio_0;
    gpio_1_ = gpio_1;

    // Inicializo pines GPIO como salidas Wiegand
    gpio_set_direction(gpio_0_, GPIO_MODE_OUTPUT);
    gpio_set_direction(gpio_1_, GPIO_MODE_OUTPUT);

    // Asigno un valor lógico 1 a los pines de salida Wiegand
    gpio_set_level(gpio_0_, true);
    gpio_set_level(gpio_1_, true);
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
            printf('0');
            gpio_set_level(gpio_0, 0);
            vTaskDelay(pdMS_TO_TICKS(WD_W2_delayPulso));
            gpio_set_level(gpio_0, 1);
            vTaskDelay(pdMS_TO_TICKS(WD_W2_delayIntervalo));
        } else {
            printf('1');
            gpio_set_level(gpio_1, 0);
            vTaskDelay(pdMS_TO_TICKS(WD_W2_delayPulso));
            gpio_set_level(gpio_1, 1);
            vTaskDelay(pdMS_TO_TICKS(WD_W2_delayIntervalo));
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


void encoderWiegandBits(const char *valor, gpio_num_t gpio_0, gpio_num_t gpio_1) {
    // Escribe separador + fecha + hora
    // (Asumo que log.escribeSeparador y log.escribeLineaLog son funciones que deben ser adaptadas)0

    for (size_t i = 0; valor[i] != '\0'; ++i) {
        // Accede a cada bit en la valor usando valor[i]
        char bitActual = valor[i];

        // Procesa el bit, por ejemplo, imprímelo
        //printf("%c ", bitActual);
        if (bitActual == '0') {
            printf("0");
            gpio_set_level(gpio_0, 0);
            vTaskDelay(pdMS_TO_TICKS(WD_W2_delayPulso));
            gpio_set_level(gpio_0, 1);
            vTaskDelay(pdMS_TO_TICKS(WD_W2_delayIntervalo));
        } else {
            printf("1");
            gpio_set_level(gpio_1, 0);
            vTaskDelay(pdMS_TO_TICKS(WD_W2_delayPulso));
            gpio_set_level(gpio_1, 1);
            vTaskDelay(pdMS_TO_TICKS(WD_W2_delayIntervalo));
        }
    }
    printf("\n");

    //printf("Datos a enviar: ");
    // Envío código Wiegand byte a byte
    /*for (size_t i = 0; valor[i] != '\0'; ++i) {
        if (valor[i] == '0') {
            printf('0');
            gpio_set_level(gpio_0, 0);
            vTaskDelay(pdMS_TO_TICKS(WD_W2_delayPulso));
            gpio_set_level(gpio_0, 1);
            vTaskDelay(pdMS_TO_TICKS(WD_W2_delayIntervalo));
        } else {
            printf('1');
            gpio_set_level(gpio_1, 0);
            vTaskDelay(pdMS_TO_TICKS(WD_W2_delayPulso));
            gpio_set_level(gpio_1, 1);
            vTaskDelay(pdMS_TO_TICKS(WD_W2_delayIntervalo));
        }
    }*/
}
