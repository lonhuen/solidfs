#pragma once

#include "common.h"

class Storage{

    public:
        Storage() {};
        ~Storage() {};

        virtual int read_block(BLOCK_ID id, uint8_t* dst) = 0;
        virtual int write_block(BLOCK_ID id, const uint8_t* src) = 0;
};