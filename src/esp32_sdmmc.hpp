#ifndef ESP32_SDMMC_H_
#define ESP32_SDMMC_H_
#include <sys/unistd.h>
#include <sys/stat.h>
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "FS.h"
#include "esp32_filesystem.hpp"




#define SDMMC_BUS_WIDTH 1 //set bus width. valid values are 1 and 4

static const char *TAG = "ESP32 SD-MMC";
static const char *TMP_FILE = "tmp.tmp";
class esp32_sdmmc : public FS{

    public:
    esp32_sdmmc();

    bool init(
        gpio_num_t clk = gpio_num_t::GPIO_NUM_38, 
        gpio_num_t cmd = gpio_num_t::GPIO_NUM_39, 
        gpio_num_t d0 = gpio_num_t::GPIO_NUM_40,
        gpio_num_t d1 = gpio_num_t::GPIO_NUM_41,
        gpio_num_t d2 = gpio_num_t::GPIO_NUM_42,
        gpio_num_t d3 = gpio_num_t::GPIO_NUM_47,
        const char* mount = "/sd"
    );

    bool format();
    size_t totalBytes();
    size_t usedBytes();
    void end();

    private:
        esp32_fs_impl *_fs;
        sdmmc_card_t *_card;

};

extern esp32_sdmmc SDMMC;
#endif