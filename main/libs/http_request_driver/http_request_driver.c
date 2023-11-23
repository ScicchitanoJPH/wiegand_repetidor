#include "http_request_driver.h"

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

void post_rest_function(char *valor,char* port)
{
    
    printf("DATA : %s\n", valor);

    char url[120] = URL_BASE;
    strcat(url, "?data=");
    strcat(url, valor);
    strcat(url, "&port=");
    strcat(url, port);

    
    printf("URL : %s\n", url);
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