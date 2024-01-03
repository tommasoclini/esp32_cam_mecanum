#include <esp_log.h>
#include <esp_system.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include <camera.h>
#include <connectivity.h>
#include <http_server.h>
#include <rover_communication.h>

#include <cJSON.h>
#include <lwpkt/lwpkt.h>

#include <driver/gpio.h>

// support IDF 5.x
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

#if CONFIG_APP_CONNECTIVITY_TYPE_STA
#define SET_APP_CONNECTIVITY APP_CONNECTIVITY_STA
#elif CONFIG_APP_CONNECTIVITY_TYPE_AP
#define SET_APP_CONNECTIVITY APP_CONNECTIVITY_AP
#elif CONFIG_APP_CONNECTIVITY_TYPE_EAP
#define SET_APP_CONNECTIVITY APP_CONNECTIVITY_EAP_STA
#else
#define SET_APP_CONNECTIVITY APP_CONNECTIVITY_AP
#endif

static const char *TAG = "ESP32-CAM_ROVER";

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void app_main(void)
{
	/*gpio_set_intr_type(GPIO_NUM_4, GPIO_INTR_DISABLE);
	gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
	gpio_set_pull_mode(GPIO_NUM_5, GPIO_FLOATING);
	gpio_set_level(GPIO_NUM_4, 1);*/
	
	if(ESP_OK != init_camera()) {
        return;
    }

    {
		//Initialize NVS
		esp_err_t ret = nvs_flash_init();
		if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		  ESP_ERROR_CHECK(nvs_flash_erase());
		  ret = nvs_flash_init();
		}
		ESP_ERROR_CHECK(ret);
    }

	app_connect(SET_APP_CONNECTIVITY);
	start_rover_comm();

	static httpd_handle_t server = NULL;

	{
		wifi_mode_t current_mode;
		esp_wifi_get_mode(&current_mode);
		if (current_mode == WIFI_MODE_STA){
		    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
			ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
		}
	}

	/* Start the server for the first time */
	/*server = */start_webserver();

	/*while (server) {
		sleep(5);
	}*/
}
