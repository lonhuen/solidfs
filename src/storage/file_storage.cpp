#include "storage/file_storage.h"
#include "utils/log_utils.h"
#include "utils/fs_exception.h"
#include <cstring>
#include <iostream>

namespace solid {
    FileStorage::FileStorage(BlockID capacity, const std::string& path)
        : capacity(capacity), file(path,std::fstream::in | std::fstream::out) {
        if(!file.good()) {
            throw fs_error("Fail to open the file ",path," for storage");
        }
    }

    FileStorage::~FileStorage() {
        file.close();
    }
    /** 
     * @brief read Block id to dst
     * @return if it's out of range, throw exception
     */
    void FileStorage::read_block(BlockID id, uint8_t* dst) {
        //LOG(INFO) << "read_block: " << id;
        if(id >= capacity){
            throw fs_error("@read_block ",id," out of range ",capacity);
        }
        file.seekg(id * config::block_size);
        file.read((char*)dst, config::block_size);
        if(file.fail()) {
            throw fs_error("read_block ", id, " failed.");
        }
    }

    /** 
     * @brief write src to Block id
     * @return if it's out of range, throw exception
     */
    void FileStorage::write_block(BlockID id, const uint8_t* src) {
        //LOG(INFO) << "write_block " << id;
        if(id >= capacity){
            throw fs_error("@write_block ",id," out of range ",capacity);
        }
        file.seekp(id * config::block_size);
        file.write((char*)src, config::block_size);
        if(file.fail()) {
            throw fs_error("write_block ", id, " failed.");
        }
        file.flush();
    }
};
