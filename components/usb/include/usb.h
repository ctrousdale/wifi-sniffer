#pragma once

#include <stdio.h>
// #include <stdint.h>

typedef struct wifi_packet_header
{
    uint32_t seconds;
    uint32_t microseconds;
    uint32_t capture_length;
    uint32_t packet_length;
} wifi_packet_header;

void init_usb(void);

void send_pcap_global_header(void);

void uart_task(void *pvParameters);