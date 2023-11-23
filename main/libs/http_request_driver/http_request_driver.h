#include "esp_netif.h"
#include "esp_http_client.h"

#define URL_BASE "http://httpbin.org/post"

esp_err_t client_event_post_handler(esp_http_client_event_handle_t evt);

void post_rest_function(char *valor,char* port);