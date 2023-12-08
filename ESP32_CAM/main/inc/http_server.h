/*
 * http_server.h
 *
 *  Created on: 28 Nov 2023
 *      Author: Tommaso
 */

#ifndef MAIN_INC_HTTP_SERVER_H_
#define MAIN_INC_HTTP_SERVER_H_

#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_system.h>

httpd_handle_t start_webserver(void);
esp_err_t stop_webserver(httpd_handle_t server);

#endif /* MAIN_INC_HTTP_SERVER_H_ */
