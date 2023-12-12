#include <connectivity_private.h>

void app_connect(app_connection_type_t app_connection_type){
    switch (app_connection_type)
    {
    case APP_CONNECTIVITY_STA:
        app_connect_sta();
        break;
    case APP_CONNECTIVITY_EAP_STA:
        app_connect_eap();
        break;
    case APP_CONNECTIVITY_AP:
        app_connect_ap();
        break;
    default:
        app_connect_ap();
        break;
    }
}
