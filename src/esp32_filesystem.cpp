#include "esp32_filesystem.hpp"
#include <typeinfo>
esp32_file_drive::esp32_file_drive() :
    FS(FSImplPtr(new esp32_fs_impl())), 
    partitionLabel(NULL),
    _fileSystem(&SPIFFS)
{

}

esp32_file_drive::esp32_file_drive(FS &disk, const char* label, int index, esp32_drive_type type):
    FS(FSImplPtr(new esp32_fs_impl())), 
    partitionLabel(label),
    _fileSystem(&disk),
    _index(index),
    _type(type)
{
}

esp32_drive_info esp32_file_drive::info(){
    
    //if(typeid(this->_fileSystem) == typeid(SPIFFSFS)){
    //if (SPIFFSFS* spfs = dynamic_cast<SPIFFSFS*>((SPIFFSFS*)this->_fileSystem); spfs != nullptr)
    if(_type == dt_SPIFFS)
    {
        auto *fs = (SPIFFSFS*)this->_fileSystem;
        return esp32_drive_info(_type, fs->totalBytes(), fs->usedBytes());        
    } else if(_type == dt_SD)    {
    //} else if (SDFS* sdfs = dynamic_cast<SDFS*>((SDFS*)this->_fileSystem); sdfs != nullptr){
        auto *fs = (SDFS*)this->_fileSystem;
        return esp32_drive_info(_type, fs->totalBytes(), fs->usedBytes());
        
    }else {
        log_e("Error occured, %s has an unknown file system type", this->label());
    }
}

void esp32_file_drive::list(const char * directory)
{
    auto root = _fileSystem->open(directory);
    if(!root){
        Serial.println("Filesystem root missing");
        return;
    } 
    File pointerFile;
    pointerFile = root.openNextFile();
    do{
        if(pointerFile){
            if(pointerFile.isDirectory()){
                Serial.printf("Directory\t%s\n", pointerFile.path());
                list(pointerFile.path());
            }
            else
                Serial.printf("File\t\t%s (%d bytes). Full path /%s%s\n",                    
                    pointerFile.name(), 
                    pointerFile.size(), 
                    this->label(),
                    pointerFile.path()
                );
            pointerFile = root.openNextFile();
        }
    }while(pointerFile);
    root.close();    
}

void esp32_file_system::addDisk(FS &disk, const char* label, esp32_drive_type type)
{
    
    for(int idx = 0; idx < _disks.size();idx++){
        if(strcmp(_disks[idx].label(), label) == 0){
            Serial.printf("Disk %s already added. Skipping.\n", label);
            return;
        }
    }
    
    _disks.push_back(esp32_file_drive(disk, label,_disks.size(),type));
}

int esp32_file_system::driveCount()
{
    return _disks.size();
}

esp32_file_drive* esp32_file_system::getDisk(int index)
{
    return &_disks[index];
}

esp32_file_drive* esp32_file_system::getDisk(const char *driveName)
{
    for(int idx = 0; idx < _disks.size();idx++){
        if(strcmp(_disks[idx].label(), driveName) == 0){            
            return &_disks[idx];
        }
    }
    return NULL;
}
extern esp32_file_system filesystem;

/// @brief Parses file information from path without verifying file information on disk
/// @param path Path to file
esp32_file_info::esp32_file_info(const char *path)
{    
    string fullPath = path;
    bool drivePassed = false;
    _driveIdx = 0;
    //parse and apply query parameters
    int queryIdx = fullPath.find_first_of('?');
    if(queryIdx > 0){           
        auto params = explode(fullPath.substr(queryIdx + 1),"&",true);
        for(int idx = 0; idx < params.size(); idx++){
            auto keyValue = explode(params[idx], "=",true);
            if(keyValue.size() == 1){
                if(strcmp(keyValue[0].c_str(),"download") == 0)
                    isDownload = true;
            }
            else if(keyValue.size() == 2){
                if(strcmp(keyValue[0].c_str(),"download") == 0)
                    isDownload = strcmp(keyValue[1].c_str(),"true") == 0;
                else if(strcmp(keyValue[0].c_str(),"drive") == 0){
                    drivePassed = true;
                    _driveIdx = atoi(keyValue[1].c_str());
                }
            }
        }
        //update working path
        fullPath = fullPath.substr(0,queryIdx).c_str();
    }

    //if drive not explicitly passed in parameter, see if path includes volume
    auto parts= explode(fullPath.c_str(),"/", true);
    if(parts.size() > 1){
        auto drive = filesystem.getDisk(parts[0].c_str());
        if(drive != NULL){
            //Serial.printf("Found matching drive %s at index %d\n", drive->label(),  drive->index());
            _driveIdx = drive->index();
            fullPath = fullPath.substr(parts[0].length() + ( fullPath[0] == '/' ? 1 : 0)); //trim filesystem from path
        }
    }
    //set fully qualified path
    _fullyQualifiedPath = fullPath.c_str();
    
    //requested gz file. mark as such
    if(fullPath.length() > 3 && strcmp(fullPath.substr(fullPath.length() - 3).c_str(), ".gz") == 0){
        isGZ = true;
    }
    

    //set path and filename
    auto lastSplitIdx = fullPath.find_last_of("/");
    if(lastSplitIdx == -1){
        
        _path = "/";
        _filename = fullPath.c_str();
        
    }
    else if(lastSplitIdx == 0){            
        
        _path = "/";
        _filename = fullPath.substr(1).c_str();

    } else{        

        _path =  fullPath.substr(0,lastSplitIdx).c_str();
        _filename = fullPath.substr(lastSplitIdx + 1).c_str();

    }

    #ifdef DEBUG
    Serial.printf("Parsed Path %s file %s on disk %d. FQP: %s\n", 
        _path.c_str(), 
        _filename.c_str(),
        _driveIdx,
        _fullyQualifiedPath.c_str()
    );
    #endif
    
}

/// @brief Gets file information from path and details from file on disk
/// @param path Path to file
esp32_file_info_extended::esp32_file_info_extended(const char *path): esp32_file_info(path){
    
    //check if file requested is not gz, but stored on disk as GZ
    auto drive = filesystem.getDisk(_driveIdx);

    if(!isGZ){
    //check if file on disk is GZ file       
        string gzPath = string_format("%s.gz", _fullyQualifiedPath.c_str());
        bool foundGzVersion = drive->exists(gzPath.c_str());
        if(foundGzVersion){
            //update fully qualified path and gz flag
            isGZ = true;
            _fullyQualifiedPath = gzPath.c_str();
        }
    }
    Serial.printf("Getting extended info for file %s from drive %d: %s\n",
        _fullyQualifiedPath.c_str(),
        _driveIdx, 
        drive->label()
    );
    //verify file and get size
    auto file = drive->open(_fullyQualifiedPath.c_str(), "r",false);
    if(!file){
        _exists = false;
        return;
    }
    _size = file.size();
    _exists = true;
    file.close();
}