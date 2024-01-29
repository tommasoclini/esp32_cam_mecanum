idf_component_register (SRCS ./main.c
						./src/camera.c
						./src/rover_communication.c
						./src/http_server.c
                        PRIV_INCLUDE_DIRS ./inc
                        PRIV_REQUIRES nvs_flash
                        esp_http_server
                        esp_timer
                        esp_wifi
                        wpa_supplicant
                        cjson
                        lwpkt
                        lwrb
                        bt
                        blufi_wrap)
