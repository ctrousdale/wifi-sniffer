#include "wifi.hpp"

#include "queue_manager.hpp"
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <cstring>
#include <cstdlib>
#include "esp_netif.h"
#include "esp_event.h"
// #include "esp_event_cxx.hpp"
#include <memory>

namespace Wifi_NS
{
    static const char *TAG = "wifi";

    void promiscuous_cb(void *buf, wifi_promiscuous_pkt_type_t type)
    {
        if (buf == nullptr)
        {
            ESP_LOGE(TAG, "Buffer is NULL. Exiting...");
            ESP_ERROR_CHECK(ESP_FAIL);
        }

        auto *pkt = static_cast<wifi_promiscuous_pkt_t *>(buf);

        // auto *data = static_cast<uint8_t *>(std::malloc(pkt->rx_ctrl.sig_len));
        // if (data == nullptr)
        // {
        //     ESP_LOGE(TAG, "Failed to allocate memory for packet data. Exiting...");
        //     ESP_ERROR_CHECK(ESP_FAIL);
        // }
        // std::memcpy(data, pkt->payload, pkt->rx_ctrl.sig_len);
        auto dataLen = std::size_t{pkt->rx_ctrl.sig_len} - std::size_t{4};
        auto packet = wifi_packet_t{
            .data = std::make_unique<uint8_t[]>(dataLen),
            .length = dataLen,
            .seconds = pkt->rx_ctrl.timestamp / 1000000U,
            .microseconds = pkt->rx_ctrl.timestamp % 1000000U};
        std::memcpy(packet.data.get(), pkt->payload, dataLen);

        if (type != WIFI_PKT_MISC && !pkt->rx_ctrl.rx_state)
        {
            Queue_NS::send_wifi_packet_to_xqueue(&packet);
        }
        else
        {
            ESP_LOGW(TAG, "Didn't send packet to xQueue.");
            // std::free(packet.data);
        }
        ESP_LOGI(TAG, "Packet sent to xQueue. Length: %u", static_cast<unsigned int>(packet.length));
    }

    void init_nvs()
    {
        ESP_LOGI(TAG, "Initializing NVS...");
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);
        ESP_LOGI(TAG, "NVS initialization succeeded.");
    }

    void init_wifi()
    {
        ESP_LOGI(TAG, "Initializing wifi...");
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_LOGI(TAG, "Wifi initialization succeeded.");
        ESP_LOGI(TAG, "Initializing Promiscuous mode...");
        ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
        wifi_promiscuous_filter_t promiscuous_filter = {
            .filter_mask = WIFI_PROMIS_FILTER_MASK_DATA | WIFI_PROMIS_FILTER_MASK_MGMT};
        ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&promiscuous_filter));
        ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(promiscuous_cb));
        ESP_LOGI(TAG, "Promiscuous mode successfully set.");
    }
}
