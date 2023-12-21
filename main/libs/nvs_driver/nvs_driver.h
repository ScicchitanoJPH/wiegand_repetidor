#ifndef NVS_DRIVER_H
#define NVS_DRIVER_H

#include "nvs_flash.h"

esp_err_t nvs_driver_init();
esp_err_t nvs_driver_write(const char *key, uint32_t value);
esp_err_t nvs_driver_read(const char *key, uint32_t *value);

#endif  // NVS_DRIVER_H