#include "storage/memory_storage.h"
#include "utils/log_utils.h"
#include "utils/fs_exception.h"
#include <cstring>
#include <iostream>

namespace solid {
    MemoryStorage::MemoryStorage(BlockID capacity) : capacity(capacity) {
        data = new uint8_t[capacity * config::block_size];
    }

    MemoryStorage::~MemoryStorage() {
        delete []data;
    }
    /** 
     * @brief read Block id to dst
     * @return if it's out of range, throw exception
     */
    void MemoryStorage::read_block(BlockID id, uint8_t* dst) {
        //LOG(INFO) << "read_block: " << id;
        if(id >= capacity){
            throw fs_error("@read_block ",id," out of range ",capacity);
        }
        std::memcpy(dst,data + id * config::block_size, config::block_size);
    }

    /** 
     * @brief write src to Block id
     * @return if it's out of range, throw exception
     */
    void MemoryStorage::write_block(BlockID id, const uint8_t* src) {
        //LOG(INFO) << "write_block " << id;
        if(id >= capacity){
            throw fs_error("@write_block ",id," out of range ",capacity);
        }
        std::memcpy(data + id * config::block_size, src, config::block_size);
    }
};