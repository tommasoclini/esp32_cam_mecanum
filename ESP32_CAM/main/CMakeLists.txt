idf_component_register (SRCS ./main.c
						./camera.c
						./rover_communication.c
						./http_server.c
                        PRIV_INCLUDE_DIRS ./inc
                        PRIV_REQUIRES nvs_flash
                        esp_netif
                        esp_http_server
                        esp_timer
                        json
                        bt
                        blufi_wrap)
