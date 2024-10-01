#ifndef _ESP32_FILESYSTEM_H_
#define _ESP32_FILESYSTEM_H_
#include "esp32_filesystem_objects.h"
#include "esp32_file_drive.hpp"
#include "string_helper.h"
#include "FS.h"
#include "vfs_api.h"
#include <vector>
#include <SPIFFS.h>
#include "SD.h"


using namespace std;
using namespace fs;


class esp32_fs_impl : public VFSImpl
{
public:
    inline esp32_fs_impl(){};
    virtual ~esp32_fs_impl() { }
    inline bool exists(const char* path)
    {
        File f = open(path, FILE_READ,false);
        bool valid = (f == true);// && !f.isDirectory();
        f.close();
        return valid;
    }
};



class esp32_file_system
{
public:
    void addDisk(FS &disk, const char* label, esp32_drive_type type = dt_SPIFFS);
    int driveCount();
    esp32_file_drive* getDisk(int index);
    esp32_file_drive* getDisk(const char* driveName);
private:
    vector<esp32_file_drive> _disks;
    esp32_file_drive _selectedDisk;
};
#endif