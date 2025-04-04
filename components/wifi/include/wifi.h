#pragma once

#include "esp_wifi.h"

void promiscuous_cb(void *buf, wifi_promiscuous_pkt_type_t type);

void init_nvs(void);

void init_wifi(void);
