#include <esp_err.h>
#include <esp_log.h>
#include <esp_blufi.h>
#include <esp_blufi_api.h>
#include <blufi_example.h>

extern esp_blufi_callbacks_t blufi_example_callbacks;

esp_err_t blufi_wrap_init(esp_blufi_callbacks_t *callbacks){
    esp_err_t ret = esp_blufi_host_and_cb_init(callbacks != NULL ? callbacks : &blufi_example_callbacks);
    if (ret) {
        BLUFI_ERROR("%s initialise failed: %s\n", __func__, esp_err_to_name(ret));
    }
    return ret;
}
