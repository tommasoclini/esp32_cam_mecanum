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

/*#include "lwip/err.h"
#include "lwip/sys.h"*/

#include <camera.h>
#include <http_server.h>
#include <rover_communication.h>

#include <cJSON.h>
#include <lwpkt/lwpkt.h>

#include <esp_bt.h>

#include "esp_blufi_api.h"

#include "esp_blufi.h"

#include <blufi_wrap.h>

// support IDF 5.x
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

static const char *TAG = "ESP32-CAM_ROVER";

/*static void disconnect_handler(void* arg, esp_event_base_t event_base,
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
*/

void app_main(void)
{
	/*gpio_set_intr_type(GPIO_NUM_4, GPIO_INTR_DISABLE);
	gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
	gpio_set_pull_mode(GPIO_NUM_5, GPIO_FLOATING);
	gpio_set_level(GPIO_NUM_4, 1);*/
	
	if(ESP_OK != init_camera()) {
        return;
    }

	esp_err_t ret;

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    initialise_wifi();

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s initialize bt controller failed: %s\n", __func__, esp_err_to_name(ret));
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable bt controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    blufi_wrap_init(NULL);

    ESP_LOGI(TAG, "BLUFI VERSION %04x\n", esp_blufi_get_version());

	start_rover_comm();

	start_webserver();
}
