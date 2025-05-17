#pragma once

#include "esp_wifi.h"
#include <esp_wifi_types_generic.h>

namespace Wifi_NS
{
    void promiscuous_cb(void *buf, wifi_promiscuous_pkt_type_t type);
    void init_nvs();
    void init_wifi();
}
