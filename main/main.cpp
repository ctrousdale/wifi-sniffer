#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

extern "C"
{
#include "sdkconfig.h"
#include "queue_manager.h"
#include "wifi.h"
#include "usb.h"
}

static const char *TAG = "main";

extern "C" void app_main(void)
{
    // Queue initialized with uart_install...
    // init_queue_manager();

    init_usb();
    init_nvs();
    init_wifi();

    send_pcap_global_header();
    auto ret = xTaskCreate(uart_task, "uart_task", 4096, NULL, 5, NULL);
    if (ret == false)
    {
        ESP_LOGI(TAG, "Failed to create xTask.");
        ESP_ERROR_CHECK(ESP_FAIL);
    }
}
