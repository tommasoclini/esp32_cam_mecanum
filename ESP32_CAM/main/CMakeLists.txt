idf_component_register (SRCS ./main.c
						./src/camera.c
                        ./src/connectivity_private.c
						./src/ap_connectivity.c
						./src/eap_connectivity.c
						./src/sta_connectivity.c
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
                        lwrb)
