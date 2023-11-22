#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "libs/wiegand/wiegand.h"
#include "libs/wiegand_sender/wiegand_sender.h"
#include "libs/WIFI_driver/WIFI_driver.h"
#include <esp_log.h>
#include <string.h>
#include "esp_event_loop.h"
#include "esp_netif.h"
#include "esp_http_client.h"






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

#define WD_PORT1_LONGITUDES {32, 26, 37}
#define WD_PORT2_LONGITUDES {35, 48}



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




esp_err_t client_event_post_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;

    default:
        break;
    }
    return ESP_OK;
}


static void post_rest_function(char *valor,char* port)
{
    
    printf("DATA : %s\n", valor);

    char url[120] = "http://httpbin.org/post";
    strcat(url, "?data=");
    strcat(url, valor);
    strcat(url, "&port=");
    strcat(url, port);

    
    
    //test...&port=e");
    esp_http_client_config_t config_post = {
        //http://worldclockapi.com/api/json/utc/now
        .url = url,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler,
        .timeout_ms = 30000};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    char  *post_data = "test ...";
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

void convertir_a_string(uint8_t *valor, char *valor_str, size_t max_size) {
    // Inicializar la cadena para asegurar que esté vacía
    valor_str[0] = '\0';

    for (size_t i = 0; i < max_size; i++) {
        char temp_str[5];  // Tamaño suficiente para almacenar un número de un solo dígito
        snprintf(temp_str, sizeof(temp_str), "%u", valor[i]);
        strncat(valor_str, temp_str, max_size - strlen(valor_str) - 1);  // Evitar desbordamiento del búfer

        if (i == max_size) {
            strncat(valor_str, "", max_size - strlen(valor_str) - 1);  // Agregar una coma entre los valores
        }
    }
}

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

uint8_t selectWDPort(size_t cant_bits){

    uint8_t WDPort1Array[] = WD_PORT1_LONGITUDES;
    uint8_t WDPort2Array[] = WD_PORT2_LONGITUDES;

    for (int i = 0; i < sizeof(WDPort1Array) / sizeof(WDPort1Array[0]); ++i) {
        printf("%d ", WDPort1Array[i]);
        if (cant_bits == WDPort1Array[i]){
            return 1;
        }
    }

    printf("--------\n");

    for (int i = 0; i < sizeof(WDPort2Array) / sizeof(WDPort2Array[0]); ++i) {
        printf("%d ", WDPort2Array[i]);
        if (cant_bits == WDPort2Array[i]){
            return 2;
        }
    }

    return 0;


}

void procesarValor(uint8_t *valor, size_t cant_bits)
{

    uint8_t senderWDPort = 0;
    senderWDPort = selectWDPort(cant_bits);

    char valor_str[100];
    convertir_a_string(valor, valor_str, cant_bits);
    valor_str[cant_bits-1] = '\0';
    printf("Cadena de valores: %s\n", valor_str);

    char port_str[5];  // Ajusta el tamaño según tus necesidades
    snprintf(port_str, sizeof(port_str), "%u", senderWDPort);
    
    

    switch (senderWDPort)
    {
    case 1:
        ESP_LOGE(TAG_ENCODER_TASK, "ZKTECO Card");
        ESP_LOGE(TAG_ENCODER_TASK, "Enviando wiegand puerto 1");
        encoderWiegandBits(valor, cant_bits, WD1_ENCODER_D0_GPIO, WD1_ENCODER_D1_GPIO, TAG_ENCODER_TASK);
        post_rest_function(valor_str,port_str);
        ESP_LOGE(TAG_ENCODER_TASK, "Enviado");
        break;

    case 2:
        ESP_LOGE(TAG_ENCODER_TASK, "WHITE Card");
        ESP_LOGE(TAG_ENCODER_TASK, "Enviando wiegand puerto 2");
        encoderWiegandBits(valor, cant_bits, WD2_ENCODER_D0_GPIO, WD2_ENCODER_D1_GPIO, TAG_ENCODER_TASK);
        post_rest_function(valor_str,port_str);
        ESP_LOGE(TAG_ENCODER_TASK, "Enviado");
        break;
    
    default:
        break;
    }
}

// Implementación de la función de tarea
void task_encoder(void *pvParameters)
{
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
    wifi_init();

    xTaskCreatePinnedToCore(task_decoder, TAG, configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(task_encoder, TAG_ENCODER_TASK, configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL, 1);
    vTaskDelay(300);
    /*uint8_t data[5] = {4,0,5,1,3};
    char valor_str[100];
    convertir_a_string(data, valor_str, sizeof(data)/sizeof(uint8_t));
    printf("Cadena de valores: %s\n", valor_str);*/
    
    //post_rest_function(valor_str,7);
}