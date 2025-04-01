
#include "nvs_flash.h"
#include "lvgl_demo.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "lcd.h"
#include "esp_log.h"

#define TAG "MAIN"

void spiffs_init(){
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/storage",      // 挂载点
        .partition_label = "storage",  // 分区名称
        .max_files = 5,              // 最大文件数
        .format_if_mount_failed = true  // 如果挂载失败，则格式化分区
    };        
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    ESP_ERROR_CHECK(ret);	// 检查是否挂载成功
}

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    esp_err_t ret;

    // ret = nvs_flash_init();             /* 初始化NVS */
    // ESP_ERROR_CHECK(ret);	
    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    // {
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }

    // spiffs_init();

    lvgl_demo();                        /* 运行LVGL例程 */

}
  