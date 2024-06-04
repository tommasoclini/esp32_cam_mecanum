#include <esp_log.h>

#include <camera.h>
#include <http_server.h>
#include <rover_communication.h>

#include <cJSON.h>
#include <lwpkt/lwpkt.h>

#include <blufi_wrap.h>

// support IDF 5.x
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

static const char *TAG = "ESP32-CAM_ROVER";

void app_main(void)
{
	if(ESP_OK != init_camera()) {
        return;
    }

    blufi_wrap_init(true);

    ESP_LOGI(TAG, "BLUFI VERSION %04x\n", esp_blufi_get_version());

	start_rover_comm();

	start_webserver();
}
