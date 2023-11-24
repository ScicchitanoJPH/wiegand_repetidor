#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "libs/wiegand/wiegand.h"
#include "libs/wiegand_sender/wiegand_sender.h"
#include "libs/WIFI_driver/WIFI_driver.h"
#include "libs/http_request_driver/http_request_driver.h"
#include <esp_log.h>
#include <string.h>
#include "esp_event_loop.h"
#include "unity.h"

#define CONFIG_EXAMPLE_BUF_SIZE 100
#define CONFIG_EXAMPLE_D0_GPIO 26
#define CONFIG_EXAMPLE_D1_GPIO 27

#define WD1_ENCODER_D0_GPIO 33
#define WD1_ENCODER_D1_GPIO 32

#define WD2_ENCODER_D0_GPIO 18
#define WD2_ENCODER_D1_GPIO 19

// Define la etiqueta para la tarea
#define TAG_ENCODER_TASK "task_encoder"
#define TAG_DECODER_TASK "task_decoder"
#define TAG_REQUEST_TASK "task_request"


#define IS_REQUEST_ACTIVATED false
#define IS_TEST_ACTIVATED false


#define MIN_CANTIDAD_BITS_VALIDO 20

#define WD_PORT1_LONGITUDES \
    {                       \
        32, 26, 37          \
    }
#define WD_PORT2_LONGITUDES \
    {                       \
        35, 48              \
    }


static wiegand_reader_t reader;
static QueueHandle_t queue = NULL;
static QueueHandle_t queue_send_data = NULL;
static QueueHandle_t queue_request_data = NULL;

// Single data packet
typedef struct
{
    uint8_t data[CONFIG_EXAMPLE_BUF_SIZE];
    uint8_t data_decoded[CONFIG_EXAMPLE_BUF_SIZE];
    size_t bits;
    uint8_t port;
} data_packet_t;





void convertir_a_string(uint8_t *valor, char *valor_str, size_t max_size)
{
    // Inicializar la cadena para asegurar que esté vacía
    valor_str[0] = '\0';

    max_size++;

    for (size_t i = 0; i < max_size; i++)
    {
        char temp_str[5]; // Tamaño suficiente para almacenar un número de un solo dígito
        snprintf(temp_str, sizeof(temp_str), "%u", valor[i]);
        strncat(valor_str, temp_str, max_size - strlen(valor_str) - 1); // Evitar desbordamiento del búfer

        if (i == max_size)
        {
            strncat(valor_str, "", max_size - strlen(valor_str) - 1); // Agregar una coma entre los valores
        }
    }
}

void reader_callback(wiegand_reader_t *r)
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

uint8_t selectWDPort(size_t cant_bits)
{

    uint8_t WDPort1Array[] = WD_PORT1_LONGITUDES;
    uint8_t WDPort2Array[] = WD_PORT2_LONGITUDES;

    for (int i = 0; i < sizeof(WDPort1Array) / sizeof(WDPort1Array[0]); ++i)
    {
        if (cant_bits == WDPort1Array[i])
        {
            return 1;
        }
    }


    for (int i = 0; i < sizeof(WDPort2Array) / sizeof(WDPort2Array[0]); ++i)
    {
        if (cant_bits == WDPort2Array[i])
        {
            return 2;
        }
    }

    return 0;
}

uint8_t procesarValor(uint8_t *valor, size_t cant_bits)
{

    uint8_t senderWDPort = 0;
    senderWDPort = selectWDPort(cant_bits);

    

    switch (senderWDPort)
    {
    case 1:
        ESP_LOGE(TAG_ENCODER_TASK, "ZKTECO Card");
        ESP_LOGE(TAG_ENCODER_TASK, "Enviando wiegand puerto 1");
        encoderWiegandBits(valor, cant_bits, WD1_ENCODER_D0_GPIO, WD1_ENCODER_D1_GPIO, TAG_ENCODER_TASK);
        ESP_LOGE(TAG_ENCODER_TASK, "Enviado");
        break;

    case 2:
        ESP_LOGE(TAG_ENCODER_TASK, "WHITE Card");
        ESP_LOGE(TAG_ENCODER_TASK, "Enviando wiegand puerto 2");
        encoderWiegandBits(valor, cant_bits, WD2_ENCODER_D0_GPIO, WD2_ENCODER_D1_GPIO, TAG_ENCODER_TASK);
        ESP_LOGE(TAG_ENCODER_TASK, "Enviado");
        break;

    default:
        break;
    }

    return senderWDPort;
}





void task_decoder(void *arg)
{
    

    // Initialize reader
    ESP_ERROR_CHECK(wiegand_reader_init(&reader, CONFIG_EXAMPLE_D0_GPIO, CONFIG_EXAMPLE_D1_GPIO,
                                        true, CONFIG_EXAMPLE_BUF_SIZE, reader_callback, WIEGAND_MSB_FIRST, WIEGAND_LSB_FIRST));

    data_packet_t p;
    while (1)
    {
        ESP_LOGI(TAG_DECODER_TASK, "Waiting for Wiegand data...");
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
            //uint8_t valor[receivedData.bits];
            uint8_t posValor = 0;
            for (size_t i = 0; i < bytes + (tail ? 1 : 0); i++)
                for (int j = 7; j >= 0; j--)
                {
                    receivedData.data_decoded[posValor] = (receivedData.data[i] >> j) & 1;
                    posValor++;
                }
            


            receivedData.port = procesarValor(receivedData.data_decoded, receivedData.bits);

            IS_REQUEST_ACTIVATED ? xQueueSend(queue_request_data, &receivedData, portMAX_DELAY) : 0;

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void task_request(void *pvParameters)
{
    data_packet_t requestData;
    queue = xQueueCreate(5, sizeof(data_packet_t));
    while (1)
    {
        if (xQueueReceive(queue_request_data, &requestData, portMAX_DELAY) == pdTRUE)
        {
            char valor_str[100];
            convertir_a_string(requestData.data_decoded, valor_str, requestData.bits);
            valor_str[requestData.bits - 1] = '\0';

            char port_str[5]; 
            snprintf(port_str, sizeof(port_str), "%u", requestData.port);

            post_rest_function(valor_str, port_str);

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}




void queue_create(){
    // Create queue
    queue = xQueueCreate(5, sizeof(data_packet_t));
    if (!queue)
    {
        ESP_LOGE(TAG_DECODER_TASK, "Error creating queue");
        ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
    }

    // Create queue
    queue_send_data = xQueueCreate(5, sizeof(data_packet_t));
    if (!queue_send_data)
    {
        ESP_LOGE(TAG_DECODER_TASK, "Error creating queue");
        ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
    }

    if(IS_REQUEST_ACTIVATED){
        // Create queue
        queue_request_data = xQueueCreate(5, sizeof(data_packet_t));
        if (!queue_request_data)
        {
            ESP_LOGE(TAG_DECODER_TASK, "Error creating queue");
            ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
        }
    }
}



void setUp(void) {
    // Set up any necessary resources before each test
}

void tearDown(void) {
    // Clean up any resources after each test
}



void test_convertir_a_string(void) {
    uint8_t data[4] = {1, 2, 3, 4};
    char result[20];
    convertir_a_string(data, result, 4);

    // Add assertions based on expected results
    TEST_ASSERT_EQUAL_STRING("1234", result);
}

void test_selectWDPort(void) {
    // Test case where cant_bits is in WD_PORT1_LONGITUDES
    TEST_ASSERT_EQUAL_UINT8(1, selectWDPort(32));

    // Test case where cant_bits is in WD_PORT2_LONGITUDES
    TEST_ASSERT_EQUAL_UINT8(2, selectWDPort(35));

    // Test case where cant_bits is not in either array
    TEST_ASSERT_EQUAL_UINT8(0, selectWDPort(50));
}

void test_procesarValor(void) {
    uint8_t data0[] = {1, 0, 1, 1, 0, 0, 1, 0}; // Example data0
    uint8_t result0 = procesarValor(data0, 8);
    TEST_ASSERT_EQUAL_UINT8(0, result0); // Assuming it matches one of the cases in your switch statement

    uint8_t data1[] = {1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0}; // Example data0
    uint8_t result1 = procesarValor(data1, 32);
    TEST_ASSERT_EQUAL_UINT8(1, result1); // Assuming it matches one of the cases in your switch statement

    uint8_t data2[] = {1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1}; // Example data0
    uint8_t result2 = procesarValor(data2, 35);
    TEST_ASSERT_EQUAL_UINT8(2, result2); // Assuming it matches one of the cases in your switch statement
}

int unity_test_init(){
    UNITY_BEGIN();

    // Run the tests
    RUN_TEST(test_convertir_a_string);
    RUN_TEST(test_selectWDPort);
    RUN_TEST(test_procesarValor);


    return UNITY_END();
}

void app_main()
{
    
    IS_REQUEST_ACTIVATED ? wifi_init() : 0;

    IS_TEST_ACTIVATED ? unity_test_init() : 0;


    queue_create();


    if(IS_REQUEST_ACTIVATED){
        xTaskCreatePinnedToCore(task_decoder, TAG_DECODER_TASK, configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL, 1);
        xTaskCreatePinnedToCore(task_request, TAG_REQUEST_TASK, configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL, 0);
    }else{
        xTaskCreatePinnedToCore(task_decoder, TAG_DECODER_TASK, configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL, 0);
    }
    
    xTaskCreatePinnedToCore(task_encoder, TAG_ENCODER_TASK, configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL, 1);

    vTaskDelay(3000);
}