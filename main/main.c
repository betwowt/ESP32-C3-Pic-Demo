
#include "nvs_flash.h"
#include "lvgl_demo.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "led.h"
#include "lcd.h"

#define TAG "MAIN"

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    // esp_err_t ret;
    
    // ret = nvs_flash_init();             /* 初始化NVS */

    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    // {
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }

    lvgl_demo();                        /* 运行LVGL例程 */

}
  