#include "usb.h"

#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_err.h"
#include "queue_manager.h"
#include "esp_task_wdt.h"

static const char *TAG = "usb";

static const uart_port_t uart_num = UART_NUM_1;

/// @brief PCAP global file header
/// @see https://wiki.wireshark.org/Development/LibpcapFileFormat
typedef struct pcap_header_t
{
    uint32_t magic_number;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t network; // https://www.tcpdump.org/linktypes.html
} pcap_header_t;

static void install_uart()
{
    ESP_LOGI(TAG, "Initializing USB...");
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS};
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    ESP_ERROR_CHECK(
        uart_set_pin(
            uart_num,
            16,
            17,
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE));

    ESP_ERROR_CHECK(
        uart_driver_install(
            uart_num,
            // 10 * sizeof(wifi_packet_t),
            1024, // Greater than UART_HW_FIFO_LEN(uart_num)
            // 1024,
            4 * 1024,
            1,
            get_queue_instance(),
            0));

    ESP_LOGI(TAG, "USB initialization succeeded.");
}

void send_pcap_global_header(void)
{
    static pcap_header_t global_header = {
        .magic_number = 0xa1b2c3d4,
        .version_major = 2,
        .version_minor = 4,
        .thiszone = 0,
        .sigfigs = 0,
        // .snaplen = 65535, //  OR: 262144
        .snaplen = 262144,
        .network = 105 // IEEE 802.11
    };

    ESP_LOGI(TAG, "Sending global packet header...");
    uart_write_bytes(uart_num, &global_header, sizeof(global_header));
    ESP_LOGI(TAG, "Sent global packet header.");
}

void init_usb(void)
{
    install_uart();
}

void uart_task(void *pvParameters)
{
    ESP_LOGI(TAG, "inside of uart_task");

    // Add the current task to the TWDT
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

    wifi_packet_t packet;
    while (true)
    {
        if (get_queue_message(&packet) != pdTRUE)
        {
            free(packet.data);
            ESP_LOGE(TAG, "Failed to get message from queue. Exiting...");
            ESP_ERROR_CHECK(ESP_FAIL);
        }

        if (packet.length > 1024U * 4U)
        {
            ESP_LOGE(TAG, "packet.length is far too long. Length: %u", (unsigned int)packet.length);
            vTaskDelay(pdMS_TO_TICKS(30 * 1000));
            continue;
        }

        wifi_packet_header header = {
            .seconds = packet.seconds,
            .microseconds = packet.microseconds,
            .capture_length = packet.length,
            .packet_length = packet.length};

        int written_bytes;
        written_bytes = uart_write_bytes(uart_num, &header, sizeof(header));
        if (written_bytes == -1)
        {
            free(packet.data);
            ESP_LOGE(TAG, "Failed to write header bytes to UART. Exiting...");
            ESP_ERROR_CHECK(ESP_FAIL);
        }

        written_bytes = uart_write_bytes(uart_num, (const char *)packet.data, packet.length);
        if (written_bytes == -1)
        {
            free(packet.data);
            ESP_LOGE(TAG, "Failed to write data bytes to UART. Exiting...");
            ESP_ERROR_CHECK(ESP_FAIL);
        }

        free(packet.data);

        // Momentarily give back control to FreeRTOS.
        // Should feed system watchdog timer.
        vTaskDelay(pdMS_TO_TICKS(1));
        // Feed task watchdog timer
        ESP_ERROR_CHECK(esp_task_wdt_reset());
        // taskYIELD();
    }

    ESP_LOGE(TAG, "Exiting uart_task()...");
}
