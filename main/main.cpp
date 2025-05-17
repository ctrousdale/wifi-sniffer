#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "wifi.hpp"
#include "queue_manager.hpp"
#include "usb.hpp"
#include "sdkconfig.h"

static const char *TAG = "main";

extern "C" void app_main(void)
{
    // Queue initialized with uart_install...
    // Queue_NS::init_queue_manager();

    USB_NS::init_usb();
    Wifi_NS::init_nvs();
    Wifi_NS::init_wifi();

    USB_NS::send_pcap_global_header();
    auto ret = xTaskCreate(USB_NS::uart_task, "uart_task", 4096, NULL, 5, NULL);
    if (ret == false)
    {
        ESP_LOGI(TAG, "Failed to create xTask.");
        ESP_ERROR_CHECK(ESP_FAIL);
    }
}
