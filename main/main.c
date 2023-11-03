#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "libs/wiegand/wiegand.h"
#include "libs/wiegand_sender/wiegand_sender.h"
#include <esp_log.h>
#include <string.h>

#define CONFIG_EXAMPLE_BUF_SIZE 100
#define CONFIG_EXAMPLE_D0_GPIO 26
#define CONFIG_EXAMPLE_D1_GPIO 27

#define WD1_ENCODER_D0_GPIO 33
#define WD1_ENCODER_D1_GPIO 32

#define WD2_ENCODER_D0_GPIO 18
#define WD2_ENCODER_D1_GPIO 19

// Define la etiqueta para la tarea
#define TAG_ENCODER_TASK "task_encoder"


#define MIN_CANTIDAD_BITS_VALIDO 20

static const char *TAG = "wiegand_reader";

static wiegand_reader_t reader;
static QueueHandle_t queue = NULL;
static QueueHandle_t queue_send_data = NULL;

// Single data packet
typedef struct
{
    uint8_t data[CONFIG_EXAMPLE_BUF_SIZE];
    size_t bits;
} data_packet_t;

// callback on new data in reader
static void reader_callback(wiegand_reader_t *r)
{
    // you can decode raw data from reader buffer here, but remember:
    // reader will ignore any new incoming data while executing callback

    // create simple undecoded data packet
    data_packet_t p;
    p.bits = r->bits;
    memcpy(p.data, r->buf, CONFIG_EXAMPLE_BUF_SIZE);

    // Send it to the queue
    xQueueSendToBack(queue, &p, 0);
}

static void task_decoder(void *arg)
{
    // Create queue
    queue = xQueueCreate(5, sizeof(data_packet_t));
    if (!queue)
    {
        ESP_LOGE(TAG, "Error creating queue");
        ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
    }

    // Create queue
    queue_send_data = xQueueCreate(5, sizeof(data_packet_t));
    if (!queue_send_data)
    {
        ESP_LOGE(TAG, "Error creating queue");
        ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
    }

    // Initialize reader
    ESP_ERROR_CHECK(wiegand_reader_init(&reader, CONFIG_EXAMPLE_D0_GPIO, CONFIG_EXAMPLE_D1_GPIO,
                                        true, CONFIG_EXAMPLE_BUF_SIZE, reader_callback, WIEGAND_MSB_FIRST, WIEGAND_LSB_FIRST));

    data_packet_t p;
    while (1)
    {
        ESP_LOGI(TAG, "Waiting for Wiegand data...");
        xQueueReceive(queue, &p, portMAX_DELAY);

        if (p.bits >= MIN_CANTIDAD_BITS_VALIDO && p.bits <= 40)
        {
            // dump received data
            printf("==========================================\n");
            printf("Bits received: %d\n", p.bits);
            printf("Received data:");
            int bytes = p.bits / 8;
            int tail = p.bits % 8;
            for (size_t i = 0; i < bytes + (tail ? 1 : 0); i++)
                for (int j = 7; j >= 0; j--)
                {
                    printf("%d", (p.data[i] >> j) & 1);
                }
            printf("\n==========================================\n");
            xQueueSend(queue_send_data, &p, portMAX_DELAY);
        }
    }
}




void procesarValor(uint8_t *valor, size_t cant_bits) {
    if (valor[0]) {
        ESP_LOGE(TAG_ENCODER_TASK, "ZKTECO Card");
        ESP_LOGE(TAG_ENCODER_TASK, "Enviando wiegand puerto 1");
        encoderWiegandBits(valor, cant_bits, WD1_ENCODER_D0_GPIO, WD1_ENCODER_D1_GPIO, TAG_ENCODER_TASK);
        ESP_LOGE(TAG_ENCODER_TASK, "Enviado");
    } else {
        ESP_LOGE(TAG_ENCODER_TASK, "WHITE Card");
        ESP_LOGE(TAG_ENCODER_TASK, "Enviando wiegand puerto 2");
        encoderWiegandBits(valor, cant_bits, WD2_ENCODER_D0_GPIO, WD2_ENCODER_D1_GPIO, TAG_ENCODER_TASK);
        ESP_LOGE(TAG_ENCODER_TASK, "Enviado");
    }
}



// Implementación de la función de tarea
void task_encoder(void *pvParameters)
{
    // Código de la tarea
    initEncoder(WD1_ENCODER_D0_GPIO, WD1_ENCODER_D1_GPIO);

    initEncoder(WD2_ENCODER_D0_GPIO, WD2_ENCODER_D1_GPIO);

    data_packet_t receivedData;
    while (1)
    {
        if (xQueueReceive(queue_send_data, &receivedData, portMAX_DELAY) == pdTRUE)
        {

            int bytes = receivedData.bits / 8;
            int tail = receivedData.bits % 8;
            uint8_t valor[receivedData.bits];
            uint8_t posValor = 0;
            for (size_t i = 0; i < bytes + (tail ? 1 : 0); i++)
                for (int j = 7; j >= 0; j--)
                {
                    valor[posValor] = (receivedData.data[i] >> j) & 1;
                    posValor++;
                }


            procesarValor(valor, receivedData.bits);

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void app_main()
{

    xTaskCreatePinnedToCore(task_decoder, TAG, configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(task_encoder, TAG_ENCODER_TASK, configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL, 1);
}