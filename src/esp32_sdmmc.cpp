
#include "esp32_sdmmc.hpp"
#include "esp_vfs_fat.h"

esp32_sdmmc::esp32_sdmmc() :  FS(FSImplPtr(new esp32_fs_impl())){

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

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    const char * mount_point = "/sd";
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

    // For SoCs where the SD power can be supplied both via an internal or external (e.g. on-board LDO) power supply.
    // When using specific IO pins (which can be used for ultra high-speed SDMMC) to connect to the SD card
    // and the internal LDO power supply, we need to initialize the power supply first.
#if CONFIG_EXAMPLE_SD_PWR_CTRL_LDO_INTERNAL_IO
    sd_pwr_ctrl_ldo_config_t ldo_config = {
        .ldo_chan_id = CONFIG_EXAMPLE_SD_PWR_CTRL_LDO_IO_ID,
    };
    sd_pwr_ctrl_handle_t pwr_ctrl_handle = NULL;

    ret = sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_ctrl_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create a new on-chip LDO power control driver");
        return;
    }
    host.pwr_ctrl_handle = pwr_ctrl_handle;
#endif

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // Set bus width to use:
//#define 
//#define CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4
#define CONFIG_SOC_SDMMC_USE_GPIO_MATRIX

#ifdef CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4
    slot_config.width = 4;
#else
    slot_config.width = 1;
#endif

    // On chips where the GPIOs used for SD card can be configured, set them in
    // the slot_config structure:
#ifdef CONFIG_SOC_SDMMC_USE_GPIO_MATRIX
    slot_config.clk = gpio_num_t::GPIO_NUM_38;
    slot_config.cmd = gpio_num_t::GPIO_NUM_39;
    slot_config.d0 = gpio_num_t::GPIO_NUM_40;
#ifdef CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4
    slot_config.d1 = gpio_num_t::GPIO_NUM_38;
    slot_config.d2 =  gpio_num_t::GPIO_NUM_21;
    slot_config.d3 = gpio_num_t::GPIO_NUM_19;
#endif  // CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4
#endif  // CONFIG_SOC_SDMMC_USE_GPIO_MATRIX

    // Enable internal pullups on enabled pins. The internal pullups
    // are insufficient however, please make sure 10k external pullups are
    // connected on the bus. This is for debug / example purpose only.
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &_card);

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

    _impl->mountpoint(mount_point);
    
    // FILE * root = fopen("/sd/hello.txt","r");
    // if (root == NULL) {
    //     ESP_LOGE(TAG, "Failed to open hello.txt");
    //     return ESP_FAIL;
    // }

    // fclose(root);
    
    return true;
}

// File esp32_sdmmc::open(const char * path, const char* mode, const bool create)
// {
//     Serial.printf("Opening SDMMC File: %s\n", path);
//     return fopen(path, mode, create);
// }

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
	FATFS* fsinfo;
	DWORD fre_clust;
	if(f_getfree("0:",&fre_clust,&fsinfo)!= 0) return 0;
    uint64_t size = ((uint64_t)(fsinfo->csize))*(fsinfo->n_fatent - 2)
#if _MAX_SS != 512
        *(fsinfo->ssize);
#else
        *512;
#endif
	return size;
}

size_t esp32_sdmmc::usedBytes()
{
	FATFS* fsinfo;
	DWORD fre_clust;
	if(f_getfree("0:",&fre_clust,&fsinfo)!= 0) return 0;
	uint64_t size = ((uint64_t)(fsinfo->csize))*((fsinfo->n_fatent - 2) - (fsinfo->free_clst))
#if _MAX_SS != 512
        *(fsinfo->ssize);
#else
        *512;
#endif
	return size;
}
//esp32_sdmmc SDMMC;