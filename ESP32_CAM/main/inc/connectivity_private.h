#ifndef _CONNECTIVITY_PRIVATE_H_
#define _CONNECTIVITY_PRIVATE_H_

#include <connectivity.h>

void app_connect_eap(void);
void app_connect_ap(void);
void app_connect_sta(void);

void app_connect(app_connection_type_t app_connection_type);

#endif
