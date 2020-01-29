#pragma once

#include "common.h"

class Storage{

    public:
        Storage() {};
        virtual ~Storage() {}; 

        virtual int read_block(bid_t id, uint8_t* dst) = 0;
        virtual int write_block(bid_t id, const uint8_t* src) = 0;
};