#include "wifi.h"
#include "queue_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <string.h>
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "wifi";

static const wifi_promiscuous_filter_t promiscuous_filter =
    {.filter_mask = WIFI_PROMIS_FILTER_MASK_DATA | WIFI_PROMIS_FILTER_MASK_MGMT};

void promiscuous_cb(void *buf, wifi_promiscuous_pkt_type_t type)
{
    if (buf == NULL)
    {
        ESP_LOGE(TAG, "Buffer is NULL. Exiting...");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;

    wifi_packet_t packet;
    packet.data = malloc(pkt->rx_ctrl.sig_len);
    if (packet.data == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for packet data. Exiting...");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    memcpy(packet.data, pkt->payload, pkt->rx_ctrl.sig_len);
    packet.seconds = pkt->rx_ctrl.timestamp / 1000000U;
    packet.microseconds = pkt->rx_ctrl.timestamp % 1000000U;
    packet.length = pkt->rx_ctrl.sig_len;
    packet.length -= 4; // Remove FCS

    /* For now, the sniffer only dumps the length of the MISC type frame */
    if (type != WIFI_PKT_MISC && !pkt->rx_ctrl.rx_state)
    {
        send_wifi_packet_to_xqueue(&packet);
    }
    else
    {
        ESP_LOGW(TAG, "Didn't send packet to xQueue.");
        free(packet.data);
    }

    ESP_LOGI(TAG, "Packet sent to xQueue. Length: %u", (unsigned int)packet.length);
}

void init_nvs(void)
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

void init_wifi(void)
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
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&promiscuous_filter));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&promiscuous_cb));
    ESP_LOGI(TAG, "Promiscuous mode successfully set.");
}
