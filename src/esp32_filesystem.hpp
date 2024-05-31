#ifndef _ESP32_FILESYSTEM_H_
#define _ESP32_FILESYSTEM_H_
#include "esp32_filesystem_objects.h"
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


class esp32_file_drive: public FS {
public:
    esp32_file_drive();
    esp32_file_drive(FS& disk, const char * label = NULL, int index = 0, esp32_drive_type type = dt_SPIFFS);
    inline const char* label(){
        return partitionLabel;
    }
    inline int index(){ return _index;}

    void list(const char * directory = "/");

    esp32_drive_info info();

    virtual bool exists(const char* path){
        return _fileSystem->exists(path);
    }
    virtual bool mkdir(const char * path){
        return _fileSystem->mkdir(path);
    }

    virtual File open(const char * path, const char* mode = FILE_READ, bool create = false){
        return _fileSystem->open(path, mode, create);
    }    

    
private:
    const char * partitionLabel;
    FS* _fileSystem;
    esp32_drive_type _type;
    int _index;
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