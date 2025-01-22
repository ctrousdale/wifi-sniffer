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
    int ret = xQueueSend(packet_queue, &packet, portMAX_DELAY);
    if (ret != pdTRUE)
    {
        ESP_LOGE(TAG, "Work queue full!");
    }

    free(packet->data);
}

BaseType_t get_queue_message(wifi_packet_t *buf)
{
    ESP_LOGI(TAG, "Before xQueue: buf->length: %u", (unsigned int)buf->length);

    wifi_packet_t t;
    BaseType_t ret = xQueueReceive(packet_queue, &t, portMAX_DELAY);
    ESP_LOGI(TAG, "After xQueue: t->length: %u", (unsigned int)t.length);

    buf = &t;
    ESP_LOGI(TAG, "After assignment: buf->length: %u", (unsigned int)buf->length);

    return ret;
}

QueueHandle_t *get_queue_instance(void)
{
    return &packet_queue;
}