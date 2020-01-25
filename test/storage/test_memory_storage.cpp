#include <iostream>
#include "storage/memory_storage.h"
#include "utils/log_utils.h"

int main(int argc,char* argv[]) {

    LogUtils::init(argv[0]);

    MemoryStorage* ms = new MemoryStorage();
    uint8_t buffer[BLOCK_SIZE];
    uint8_t buffer2[BLOCK_SIZE];

    for(int i=0;i<BLOCK_SIZE;i++) {
        buffer[i] = i;
    }

    ms->write_block(0,buffer);
    ms->read_block(0,buffer2);

    for(int i=0;i<BLOCK_SIZE;i++) {
        if(buffer[i]!=buffer2[i]) {
            std::cout << "Error" << std::endl;
            break;
        }
    }
    return 0;
}