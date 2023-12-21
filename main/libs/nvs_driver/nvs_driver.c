#include "nvs_driver.h"

esp_err_t nvs_driver_init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return err;
}

esp_err_t nvs_driver_write(const char *key, uint32_t value) {
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvsHandle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_u32(nvsHandle, key, value);
    if (err != ESP_OK) {
        nvs_close(nvsHandle);
        return err;
    }

    err = nvs_commit(nvsHandle);
    nvs_close(nvsHandle);
    return err;
}

esp_err_t nvs_driver_read(const char *key, uint32_t *value) {
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvsHandle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_get_u32(nvsHandle, key, value);
    nvs_close(nvsHandle);
    return err;
}
