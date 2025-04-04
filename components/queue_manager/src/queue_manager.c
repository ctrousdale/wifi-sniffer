#include "queue_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "queue_manager";

QueueHandle_t packet_queue;

void init_queue_manager(void)
{
    packet_queue = xQueueCreate(10, sizeof(wifi_packet_t));
    if (packet_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create xQueue");
        ESP_ERROR_CHECK(ESP_FAIL);
        return;
    }
}

void send_wifi_packet_to_xqueue(wifi_packet_t *packet)
{
    if (packet == NULL)
    {
        ESP_LOGE(TAG, "Packet is NULL. Cannot send to queue.");
        ESP_ERROR_CHECK(ESP_FAIL);
        return;
    }

    int ret = xQueueSend(packet_queue, packet, portMAX_DELAY);
    if (ret != pdTRUE)
    {
        ESP_LOGE(TAG, "Work queue full!");
        free(packet->data);
    }
}

BaseType_t get_queue_message(wifi_packet_t *buf)
{
    if (buf == NULL)
    {
        ESP_LOGE(TAG, "Buffer is NULL. Cannot receive from queue.");
        return pdFALSE;
    }

    BaseType_t ret = xQueueReceive(packet_queue, buf, portMAX_DELAY);

    return ret;
}

QueueHandle_t *get_queue_instance(void)
{
    return &packet_queue;
}