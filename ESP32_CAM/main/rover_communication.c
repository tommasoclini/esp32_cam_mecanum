/*
 * rover_communication.c
 *
 *  Created on: 28 Nov 2023
 *      Author: Tommaso
 */

#include <stdio.h>
#include <string.h>

#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <esp_log.h>
#include <sys/param.h>

#include "hal/uart_types.h"
#include "lwip/sockets.h"
#include <lwip/netdb.h>

#include <rover_communication.h>

// UDP socket related
#define KEEPALIVE_IDLE CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT CONFIG_EXAMPLE_KEEPALIVE_COUNT

static const char *TAG = "rover communication";

static void udp_server_task(void *);
static esp_err_t udp_server_init(uart_port_t);

static esp_err_t udp_server_init(const uart_port_t uart_port) {
  xTaskCreate(udp_server_task, "udp_server", 4096, (void *)uart_port, 5, NULL);
  return ESP_OK;
}

static esp_err_t uart_init(uart_port_t uart_port) {
  /* Configure parameters of an UART driver,
   * communication pins and install the driver */
  const uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };

  ESP_ERROR_CHECK(uart_driver_install(uart_port, 512 * 2, 0, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(uart_port, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(uart_port, 13, -1, -1, -1));

  ESP_LOGI(TAG, "Initialized rover uart correctly");

  return ESP_OK;
}

static void udp_server_task(void *p) {
  char rx_buffer[128];
  char addr_str[128];
  struct sockaddr_in6 dest_addr;

  const int PORT = 3333;

  const uart_port_t uart_port = (uart_port_t)p;

  while (1) {

    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(3333);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
      ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
      break;
    }
    ESP_LOGI(TAG, "Socket created");

    // Set timeout
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

    int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
      ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
    socklen_t socklen = sizeof(source_addr);

    while (1) {
      ESP_LOGI(TAG, "Waiting for data");
      int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                         (struct sockaddr *)&source_addr, &socklen);
      // Error occurred during receiving
      if (len < 0) {
        ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
        break;
      }
      // Data received
      else {
        // Get the sender's ip address as string
        if (source_addr.ss_family == PF_INET) {
          inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str,
                      sizeof(addr_str) - 1);
        } else if (source_addr.ss_family == PF_INET6) {
          inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr,
                       addr_str, sizeof(addr_str) - 1);
        }

        rx_buffer[len] =
            0; // Null-terminate whatever we received and treat like a string...
        const int sent = uart_write_bytes(uart_port, rx_buffer, len);
        ESP_LOGI(TAG,
                 "Received %d bytes from %s, sent %d bytes to serial port:",
                 len, addr_str, sent);
      }
    }

    if (sock != -1) {
      ESP_LOGE(TAG, "Shutting down socket and restarting...");
      shutdown(sock, 0);
      close(sock);
    }
  }
  vTaskDelete(NULL);
}

void start_rover_comm(void) {
  const uart_port_t uart_port = UART_NUM_1;
  ESP_ERROR_CHECK(uart_init(uart_port));
  ESP_ERROR_CHECK(udp_server_init(uart_port));
}
