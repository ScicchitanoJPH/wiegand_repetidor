// bluetooth.h
#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>


#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"


#include "sys/time.h"

#include "nvs.h"
#include "nvs_flash.h"





void send_data_over_bluetooth(const uint8_t *data, uint32_t len);

static char *bda2str(uint8_t * bda, char *str, size_t size);

static void print_speed(void);

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);

void send_data_bluetooth(char *data);

char * init_bluetooth();

#endif // BLUETOOTH_H