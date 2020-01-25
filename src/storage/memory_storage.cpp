#include "storage/memory_storage.h"
#include "utils/log_utils.h"
#include <cstring>
/** 
 * @brief read Block id to dst
 * @return 1 for sucess, 0 for fail
 */
int MemoryStorage::read_block(BLOCK_ID id, uint8_t* dst) {
    if(id >= BLOCK_SIZE){
        LOG(ERROR) << "Reading Block " << id << " out of Range " << NR_BLOCKS;
        return 0;
    }
    memcpy(dst,buffer+id*BLOCK_SIZE,BLOCK_SIZE);
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
    memcpy(buffer+id*BLOCK_SIZE, src, BLOCK_SIZE);
    LOG(INFO) << "Writing Block " << id;
    return 1;
}
