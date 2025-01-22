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
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
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
            10,
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
    // send_pcap_global_header();
}

void uart_task(void *pvParameters)
{
    ESP_LOGI(TAG, "inside of uart_task");

    // Initialize the Task Watchdog Timer (TWDT)
    // static esp_task_wdt_config_t wdt_config = {
    //     .timeout_ms = 10 * 1000,
    //     .trigger_panic = true};
    // ESP_ERROR_CHECK(esp_task_wdt_init(&wdt_config));

    // Add the current task to the TWDT
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

    wifi_packet_t packet;
    while (1)
    {
        ESP_LOGI(TAG, "Before get_queue_message. Should be garbage. packet.length: %u", (unsigned int)packet.length);
        if (get_queue_message(&packet) == pdTRUE)
        {
            ESP_LOGI(TAG, "After get_queue_message. Should be good. packet.length: %u", (unsigned int)packet.length);
            ESP_LOGI(TAG, "got queued message");
            wifi_packet_header header = {
                .seconds = packet.seconds,
                .microseconds = packet.microseconds,
                .capture_length = packet.length,
                .packet_length = packet.length};

            ESP_LOGI(TAG, "Attempting to write header to UART...");
            int written_bytes;
            written_bytes = uart_write_bytes(uart_num, &header, sizeof(header));
            if (written_bytes == -1)
            {
                ESP_LOGE(TAG, "Failed to write header bytes to UART. Exiting...");
                ESP_ERROR_CHECK(ESP_FAIL);
            }

            /*
            ESP_LOGI(TAG, "Attempting to write data to UART...");
            written_bytes = uart_write_bytes(uart_num, (const char *)packet.data, packet.length);
            if (written_bytes == -1)
            {
                ESP_LOGI(TAG, "Failed to write data bytes to UART. Exiting...");
                ESP_LOGE(TAG, "Failed to write data bytes to UART. Exiting...");
                ESP_ERROR_CHECK(ESP_FAIL);
            }
            */

            int remaining_bytes = packet.length;
            uint8_t *data_ptr = packet.data;
            const int chunk_size = 1024;
            while (remaining_bytes > 0)
            {
                int bytes_to_write = (remaining_bytes > chunk_size) ? chunk_size : remaining_bytes;
                written_bytes = uart_write_bytes(uart_num, (const char *)data_ptr, bytes_to_write);
                if (written_bytes == -1)
                {
                    ESP_LOGE(TAG, "Failed to write data bytes to UART. Exiting...");
                    ESP_ERROR_CHECK(ESP_FAIL);
                }
                data_ptr += written_bytes;
                remaining_bytes -= written_bytes; // Feed the watchdog timer

                ESP_LOGI(TAG, "Wrote %i bytes, %i remaining.", written_bytes, remaining_bytes);

                esp_task_wdt_reset();
            }

            ESP_LOGI(TAG, "Data written to UART, total bytes: %u", (unsigned int)packet.length);
            ESP_LOGI(TAG, "Wrote all data to UART.");

            free(packet.data);

            ESP_LOGI(TAG, "Waiting...");
            vTaskDelay(pdMS_TO_TICKS(10 * 1000));
        }

        ESP_LOGE(TAG, "Exiting uart_task()...");
    }
}
