
#include "esp32_sdmmc.hpp"
#include "esp_vfs_fat.h"
esp32_sdmmc::esp32_sdmmc() : FS(FSImplPtr(_fs)){

}
    

bool esp32_sdmmc::init(
    gpio_num_t clk, 
    gpio_num_t cmd, 
    gpio_num_t d0,
    gpio_num_t d1,
    gpio_num_t d2,
    gpio_num_t d3 ,
    const char* mount
){
    //initialize peripheral
    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_err_t ret;
    
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    #ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
    #else
        .format_if_mount_failed = false,
    #endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    
    //const char mount_point[] = mount;
    ESP_LOGI(TAG, "Initializing SD card");
    
    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.

    ESP_LOGI(TAG, "Using SDMMC peripheral");

    // By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
    // For setting a specific frequency, use host.max_freq_khz (range 400kHz - 40MHz for SDMMC)
    // Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    slot_config.width = 1;
    #ifdef SDMMC_BUS_WIDTH
        #if SDMMC_BUS_WIDTH == 4
            slot_config.width = 4;
        #endif
    #endif

    slot_config.clk = clk;
    slot_config.cmd = cmd;
    slot_config.d0 = d0;
    #ifdef SDMMC_BUS_WIDTH
        #if SDMMC_BUS_WIDTH == 4
            slot_config.d1 = d1;
            slot_config.d2 =  d2;
            slot_config.d3 = d3;
        #endif
    #endif


    // Enable internal pullups on enabled pins. The internal pullups
    // are insufficient however, please make sure 10k external pullups are
    // connected on the bus. This is for debug / example purpose only.
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdmmc_mount(mount, &host, &slot_config, &mount_config, &_card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                    "If you want the card to be formatted, set the FORMAT_IF_MOUNT_FAILED define.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                    "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
    #ifdef CONFIG_EXAMPLE_DEBUG_PIN_CONNECTIONS
        check_sd_card_pins(&config, pin_count);
    #endif
        }
        return true;
    }
    ESP_LOGI(TAG, "Filesystem mounted");
    
    return true;
}

//idf v.4.4  does not have format implemented
bool esp32_sdmmc::format()
{
    // auto ret = esp_vfs_fat_sdcard_format(_fs->mountpoint(), _card);
    // File f = _fs->open(TMP_FILE,"w",true);
    // f.close();
    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to format FATFS (%s)", esp_err_to_name(ret));
    //     return false;
    // }

    // if (stat(TMP_FILE, &st) == 0) {
    //     ESP_LOGI(TAG, "file still exists");
    //     return false;
    // } else {
    //     ESP_LOGI(TAG, "file doesn't exist, formatting done");
    // }
    // return true;
    return false;
}

size_t esp32_sdmmc::totalBytes()
{
    // Card has been initialized, print its properties
    //sdmmc_card_print_info(stdout, _card);
    return 0;
}
esp32_sdmmc SDMMC = esp32_sdmmc();