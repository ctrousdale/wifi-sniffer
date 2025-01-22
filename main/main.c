// #include <stdio.h>
// #include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "esp_chip_info.h"
#include "esp_log.h"
// #include "esp_flash.h"
// #include "esp_system.h"
// #include "components/wifi/include/wifi.h"
#include "queue_manager.h"
#include "wifi.h"
#include "usb.h"

static const char *TAG = "main";

void app_main(void)
{
    // Queue initialized with uart_install...
    // init_queue_manager();

    init_usb();
    init_nvs();
    init_wifi();

    send_pcap_global_header();
    BaseType_t ret = xTaskCreate(uart_task, "uart_task", 4096, NULL, 10, NULL);
    if (ret == false)
    {
        ESP_LOGI(TAG, "Failed to create xTask.");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    ESP_LOGI(TAG, "Outside of xTaskCreate...");

    // xQueueSend(packet_queue, sizeof(wifi_packet_t));
}
