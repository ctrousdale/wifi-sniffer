#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdio.h>
#include <stdint.h>
#include <memory>

typedef struct wifi_packet_t
{
    std::unique_ptr<uint8_t[]> data;
    uint32_t length;
    uint32_t seconds;
    uint32_t microseconds;
} wifi_packet_t;

namespace Queue_NS
{
    void init_queue_manager(void);
    void send_wifi_packet_to_xqueue(wifi_packet_t *packet);
    BaseType_t get_queue_message(wifi_packet_t *const buf);
    QueueHandle_t *get_queue_instance(void);
}
