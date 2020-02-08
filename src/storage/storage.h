#pragma once

#include "common.h"
namespace solid {
    class Storage{
    public:
        Storage() {};
        virtual ~Storage() {}; 

        virtual void read_block(BlockID id, uint8_t* dst) = 0;
        virtual void write_block(BlockID id, const uint8_t* src) = 0;
    };
};