#include "wifi.h"
#include "esp_wifi.h"
#include "freertos/queue.h"
#include <string.h>
#include "esp_log.h"

static const char *TAG = "wifi";

static QueueHandle_t packet_queue;

typedef struct
{
    uint8_t *data;
    uint16_t length;
} wifi_packet_t;

static const wifi_promiscuous_filter_t promiscuous_filter =
    {.filter_mask = WIFI_PROMIS_FILTER_MASK_ALL};

static void promiscuous_cb(void *buf, wifi_promiscuous_pkt_type_t type)
{
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
    wifi_packet_t packet;

    packet.data = malloc(pkt->rx_ctrl.sig_len);
    memcpy(packet.data, pkt->payload, pkt->rx_ctrl.sig_len);
    packet.length = pkt->rx_ctrl.sig_len;

    xQueueSend(packet_queue, &packet, portMAX_DELAY);
};

static void init_wifi(void)
{
    ESP_LOGI(TAG, "Initializing wifi...");

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wifi initialization succeeded. Entering Promiscuous mode...");

    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&promiscuous_filter));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&promiscuous_cb));
}

static void wifi_sniffer_cb(void *recv_buf, wifi_promiscuous_pkt_type_t type)
{
    wifi_promiscuous_pkt_t *sniffer = (wifi_promiscuous_pkt_t *)recv_buf;
}