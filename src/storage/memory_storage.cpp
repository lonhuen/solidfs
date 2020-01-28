#include "storage/memory_storage.h"
#include "utils/log_utils.h"
#include <cstring>
#include <iostream>

MemoryStorage::MemoryStorage(BLOCK_ID nr_blocks) : NR_BLOCKS(nr_blocks) {
    data = new uint8_t[NR_BLOCKS * BLOCK_SIZE];
}

MemoryStorage::~MemoryStorage() {
    delete []data;
}

/** 
 * @brief read Block id to dst
 * @return 1 for sucess, 0 for fail
 */
int MemoryStorage::read_block(BLOCK_ID id, uint8_t* dst) {
    if(id >= BLOCK_SIZE){
        LOG(ERROR) << "Reading Block " << id << " out of Range " << NR_BLOCKS;
        return 0;
    }
    memcpy(dst,data + id*BLOCK_SIZE,BLOCK_SIZE);
    LOG(INFO) << "Reading Block " << id;
    return 1;
}

/** 
 * @brief write src to Block id
 * @return 1 for sucess, 0 for fail
 */
int MemoryStorage::write_block(BLOCK_ID id, const uint8_t* src) {
    if(id >= BLOCK_SIZE){
        LOG(ERROR) << "Writing Block " << id << " out of Range " << NR_BLOCKS;
        return 0;
    }
    memcpy(data + id*BLOCK_SIZE, src, BLOCK_SIZE);
    LOG(INFO) << "Writing Block " << id;
    return 1;
}

#ifdef DEBUG
void MemoryStorage::dump_block(BLOCK_ID id,DumpF f) {
    if(id >= NR_BLOCKS) {
        LOG(ERROR) << "Dumping Block " << id << " out of Range.";
        return;
    }
    if(f) {
        f(data + id*BLOCK_SIZE);
    } else {
        std::cout << std::hex;
        for(int i=0;i<BLOCK_SIZE;i++){
            std::cout << (uint32_t)data[i+id*BLOCK_SIZE] << " ";
        }
        std::cout << std::endl;
    }
}

void MemoryStorage::dump_range(BLOCK_ID sid, BLOCK_ID eid,DumpF f) {
    for(auto i=0;i<eid && i<NR_BLOCKS;i++)
        dump_block(i,f);
}
#endif