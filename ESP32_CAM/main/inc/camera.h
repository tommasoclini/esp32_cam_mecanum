/*
 * camera.h
 *
 *  Created on: 28 Nov 2023
 *      Author: Tommaso
 */

#ifndef MAIN_INC_CAMERA_H_
#define MAIN_INC_CAMERA_H_

// #define BOARD_WROVER_KIT
#define BOARD_ESP32CAM_AITHINKER

#include <esp_system.h>
#include <esp_camera.h>

esp_err_t init_camera(void);

#endif /* MAIN_INC_CAMERA_H_ */
