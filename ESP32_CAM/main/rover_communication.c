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

#include "lwip/sockets.h"
#include <lwip/netdb.h>

#include <rover_communication.h>

#define UART_TXD (CONFIG_UART_TXD)
#define UART_RXD (CONFIG_UART_RXD)
#define UART_RTS (UART_PIN_NO_CHANGE)
#define UART_CTS (UART_PIN_NO_CHANGE)

#define UART_PORT_NUM (CONFIG_UART_PORT_NUM)
#define UART_BAUD_RATE (CONFIG_UART_BAUD_RATE)

#define PORT CONFIG_EXAMPLE_PORT
#define KEEPALIVE_IDLE CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT CONFIG_EXAMPLE_KEEPALIVE_COUNT

static const char *TAG = "rover communication";

#define BUF_SIZE 512

#define UART_RX_RB_BUFFER_SIZE 256
#define UART_TX_RB_BUFFER_SIZE 256

static int send_data_to_rover(const void *src, size_t size);

static esp_err_t udp_server_init();
static void udp_server_task(void *pvParameters);

static esp_err_t udp_server_init() {
#ifdef CONFIG_EXAMPLE_IPV4
  xTaskCreate(udp_server_task, "udp_server", 4096, (void *)AF_INET, 5, NULL);
#endif
#ifdef CONFIG_EXAMPLE_IPV6
  xTaskCreate(udp_server_task, "udp_server", 4096, (void *)AF_INET6, 5, NULL);
#endif
  return ESP_OK;
}

static esp_err_t uart_init() {
  /* Configure parameters of an UART driver,
   * communication pins and install the driver */
  uart_config_t uart_config = {
      .baud_rate = UART_BAUD_RATE,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  int intr_alloc_flags = 0;

  ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL,
                                      intr_alloc_flags));
  ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
  ESP_ERROR_CHECK(
      uart_set_pin(UART_PORT_NUM, UART_TXD, UART_RXD, UART_RTS, UART_CTS));

  ESP_LOGI(TAG, "Initialized rover uart correctly");

  return ESP_OK;
}

static void udp_server_task(void *pvParameters) {
  char rx_buffer[128];
  char addr_str[128];
  int addr_family = (int)pvParameters;
  int ip_protocol = 0;
  struct sockaddr_in6 dest_addr;

  while (1) {

    if (addr_family == AF_INET) {
      struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
      dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
      dest_addr_ip4->sin_family = AF_INET;
      dest_addr_ip4->sin_port = htons(PORT);
      ip_protocol = IPPROTO_IP;
    } else if (addr_family == AF_INET6) {
      bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
      dest_addr.sin6_family = AF_INET6;
      dest_addr.sin6_port = htons(PORT);
      ip_protocol = IPPROTO_IPV6;
    }

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
      ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
      break;
    }
    ESP_LOGI(TAG, "Socket created");

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
    int enable = 1;
    lwip_setsockopt(sock, IPPROTO_IP, IP_PKTINFO, &enable, sizeof(enable));
#endif

#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
    if (addr_family == AF_INET6) {
      // Note that by default IPV6 binds to both protocols, it is must be
      // disabled if both protocols used at the same time (used in CI)
      int opt = 1;
      setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
      setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
    }
#endif
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

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
    struct iovec iov;
    struct msghdr msg;
    struct cmsghdr *cmsgtmp;
    u8_t cmsg_buf[CMSG_SPACE(sizeof(struct in_pktinfo))];

    iov.iov_base = rx_buffer;
    iov.iov_len = sizeof(rx_buffer);
    msg.msg_control = cmsg_buf;
    msg.msg_controllen = sizeof(cmsg_buf);
    msg.msg_flags = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_name = (struct sockaddr *)&source_addr;
    msg.msg_namelen = socklen;
#endif

    while (1) {
      ESP_LOGI(TAG, "Waiting for data");
#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
      int len = recvmsg(sock, &msg, 0);
#else
      int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                         (struct sockaddr *)&source_addr, &socklen);
#endif
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
#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
          for (cmsgtmp = CMSG_FIRSTHDR(&msg); cmsgtmp != NULL;
               cmsgtmp = CMSG_NXTHDR(&msg, cmsgtmp)) {
            if (cmsgtmp->cmsg_level == IPPROTO_IP &&
                cmsgtmp->cmsg_type == IP_PKTINFO) {
              struct in_pktinfo *pktinfo;
              pktinfo = (struct in_pktinfo *)CMSG_DATA(cmsgtmp);
              ESP_LOGI(TAG, "dest ip: %s\n", inet_ntoa(pktinfo->ipi_addr));
            }
          }
#endif
        } else if (source_addr.ss_family == PF_INET6) {
          inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr,
                       addr_str, sizeof(addr_str) - 1);
        }

        rx_buffer[len] =
            0; // Null-terminate whatever we received and treat like a string...
        const int sent = send_data_to_rover(rx_buffer, len);
        ESP_LOGI(TAG,
                 "Received %d bytes from %s, sent %d bytes to serial port:",
                 len, addr_str, sent);
        // ESP_LOGI(TAG, "%s", rx_buffer);

        /*int err = sendto(sock, rx_buffer, len, 0, (struct sockaddr
        *)&source_addr, sizeof(source_addr)); if (err < 0) { ESP_LOGE(TAG,
        "Error occurred during sending: errno %d", errno); break;
        }*/
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

static int send_data_to_rover(const void *src, size_t size) {
  return uart_write_bytes(UART_PORT_NUM, src, size) == ESP_OK ? size : 0;
}

void start_rover_comm(void) {
  ESP_ERROR_CHECK(uart_init());
  ESP_ERROR_CHECK(udp_server_init());
}
