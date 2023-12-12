/*
 * connectivity.h
 *
 *  Created on: 28 Nov 2023
 *      Author: Tommaso
 */

#ifndef _MAIN_INC_CONNECTIVITY_H_
#define _MAIN_INC_CONNECTIVITY_H_

typedef enum {
    APP_CONNECTIVITY_STA,
    APP_CONNECTIVITY_AP,
    APP_CONNECTIVITY_EAP_STA
} app_connection_type_t;

void app_connect(app_connection_type_t app_connection_type);

#endif /* MAIN_INC_CONNECTIVITY_H_ */
