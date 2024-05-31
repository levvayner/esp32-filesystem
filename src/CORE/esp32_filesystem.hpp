#ifndef _ESP32_FILESYSTEM_H_
#define _ESP32_FILESYSTEM_H_
#include "string_helper.h"
#include "FS.h"
#include "vfs_api.h"
#include <vector>
#include <SPIFFS.h>
using namespace std;
using namespace fs;

enum esp32_drive_type{
    dt_SPIFFS = 0,
    dt_SD = 1
};

struct esp32_file_info{
    public:
    esp32_file_info(const char * path);
    ~esp32_file_info(){
        // free ((void*)_path);
        // free ((void*)_filename);
        // free ((void*)_fullyQualifiedPath);
        // delete[] _path;
        // delete[] _filename;
        // delete[] _fullyQualifiedPath;
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
//    char _fullyQualifiedPath[32];
//    char _path[32];
//    char _filename[32];
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


class esp32_fs_impl : public VFSImpl
{
public:
    inline esp32_fs_impl(){};
    virtual ~esp32_fs_impl() { }
    inline bool exists(const char* path)
    {
        File f = open(path, "r",false);
        bool valid = (f == true);// && !f.isDirectory();
        f.close();
        return valid;
    }
};


class esp32_file_drive: public FS {
public:
    esp32_file_drive();
    esp32_file_drive(FS& disk, const char * label = NULL, int index = 0);
    inline const char* label(){
        return partitionLabel;
    }
    inline int index(){ return _index;}

    void list(const char * directory = "/");

    virtual bool exists(const char* path){
        return _fileSystem->exists(path);
    }
    virtual bool mkdir(const char * path){
        return _fileSystem->mkdir(path);
    }

    virtual File open(const char * path, const char* mode = "r", bool create = false){
        return _fileSystem->open(path, mode, create);
    }    

    
private:
    const char * partitionLabel;
    FS* _fileSystem;
    int _index;
};


class esp32_file_system
{
public:
    void addDisk(FS &disk, const char* label);
    int driveCount();
    esp32_file_drive* getDrive(int index);
    esp32_file_drive* getDrive(const char* driveName);
private:
    vector<esp32_file_drive> _disks;
    esp32_file_drive _selectedDisk;
};
#endif