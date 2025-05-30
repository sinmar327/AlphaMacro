#include "network_driver.h"
#include "bsp/board_api.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_check.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "esp_system.h"

#include "tinyusb.h"
#include "tinyusb_net.h"

#include "esp_netif.h"
#include "lwip/sockets.h"

#define PORT 3001

static const char *TAG = "USB_NCM";
#define CONFIG_LOG_MAXIMUM_LEVEL 0

static esp_err_t usb_recv_callback(void *buffer, uint16_t len, void *ctx)
{
    
}

static void free_buffer()
{

}


/*void app_main(void)
{
    //Initialize NVS — it is used to store PHY calibration data
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    esp_netif_init();

    ESP_LOGI(TAG, "USB NCM device initialization");
    const tinyusb_config_t tusb_cfg = {
        .external_phy = false,
    };
    ESP_GOTO_ON_ERROR(tinyusb_driver_install(&tusb_cfg), err, TAG, "Failed to install TinyUSB driver");
    tinyusb_net_config_t net_config = {
        .on_recv_callback = usb_recv_callback,
        .free_tx_buffer = free_buffer,
    };
    esp_read_mac(net_config.mac_addr, ESP_MAC_WIFI_STA);
    uint8_t *mac = net_config.mac_addr;
    ESP_LOGI(TAG, "Network interface HW address: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ESP_GOTO_ON_ERROR(tinyusb_net_init(TINYUSB_USBDEV_0, &net_config), err, TAG, "Failed to initialize TinyUSB NCM device class");

    return;

err:
    ESP_LOGE(TAG, "USB-WiFi bridge example failed!");
}*/
void tcp_server_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE("TCP_SERVER", "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI("TCP_SERVER", "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE("TCP_SERVER", "Socket unable to bind: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI("TCP_SERVER", "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE("TCP_SERVER", "Error occurred during listen: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI("TCP_SERVER", "Socket listening");

    while (1) {
        struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
        uint8_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE("TCP_SERVER", "Unable to accept connection: errno %d", errno);
            break;
        }
        inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        ESP_LOGI("TCP_SERVER", "Socket accepted ip address: %s", addr_str);

        while (1) {
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE("TCP_SERVER", "recv failed: errno %d", errno);
                break;
            }
            // Connection closed
            else if (len == 0) {
                ESP_LOGI("TCP_SERVER", "Connection closed");
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI("TCP_SERVER", "Received %d bytes: %s", len, rx_buffer);

                // Echo back the received data
                int to_write = len;
                while (to_write > 0) {
                    int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
                    if (written < 0) {
                        ESP_LOGE("TCP_SERVER", "Error occurred during sending: errno %d", errno);
                        break;
                    }
                    to_write -= written;
                }
            }
        }

        if (sock != -1) {
            ESP_LOGI("TCP_SERVER", "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}
