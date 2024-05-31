#ifndef _ESP32_FILESYSTEM_OBJECTS_H_
#define _ESP32_FILESYSTEM_OBJECTS_H_

#include "string_helper.h"
#include "FS.h"
using namespace std;
using namespace fs;

enum esp32_drive_type{
    dt_Invalid = -1,
    dt_SPIFFS = 0,
    dt_SD = 1
};

struct esp32_drive_info{
    public:
    esp32_drive_info(){
        this->_size = 0;
        this->_used = 0;
        this->_type = dt_Invalid;
    }
    esp32_drive_info(esp32_drive_type type, size_t size, size_t used){
        this->_size = size;
        this->_type = type;
        this->_used = used;
    }
    inline esp32_drive_type type(){ return _type;}
    /// @brief Get size of drive in bytes
    /// @return Number of bytes
    inline size_t size(){ return _size;}
    /// @brief Get used bytes
    /// @return Number of used bytes
    inline size_t used(){ return _used;}

    private:
    esp32_drive_type _type;
    size_t _size;
    size_t _used;
};

struct esp32_file_info{
    public:
    esp32_file_info(const char * path);
    ~esp32_file_info(){
    }
    /* EXAMPLE #1: GZ PATH passed in request
        REQUEST PATH: /LOG/DIR1/SNAPSHOT_2025.log.gz?download=true&drive=1
                     |---PATH--|-----filename-------|--download---|-disk:sd-|
                     |---- FULLY QUALIFIED PATH ----|
        DISK PATH: /LOG/DIR1/SNAPSHOT_2025.log.gz
        Disk: SD 1

        // EXAMPLE #2: GZ PATH not passed in request, file stored on disk is of type gz
        // REQUEST PATH: /LOG/DIR1/SNAPSHOT_2025.log.gz?download=true&drive=1
        //              |---PATH--|-----filename----|--|--download---|-disk:sd-|
        //              |---- FULLY QUALIFIED PATH ----|
        // DISK PATH: /LOG/DIR1/SNAPSHOT_2025.log.gz
        // Disk: SD 1
        // INSTEAD, USING GZ FILE DETERMINED A TIME OF REQUEST/RENDER FROM FILE
    */
    string path(){ 
        return _path;
    };
    inline string filename(){
        return _filename;
    };
    inline string fullyQualifiedPath() {
        return _fullyQualifiedPath;
    };
    inline int drive(){
        return _driveIdx;
    }
    
    bool isDownload;
   
protected:
    string _fullyQualifiedPath;
    string _path;
    string _filename;
    bool isGZ;
    int _driveIdx;
};

struct esp32_file_info_extended: public esp32_file_info{
    public:
        esp32_file_info_extended(const char * path);
        inline size_t size(){ return _size;};
        inline bool exists(){ return _exists;}
    private:
        size_t _size;
        bool _exists;
};

#endif