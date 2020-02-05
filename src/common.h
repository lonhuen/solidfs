#pragma once

#include <stdint.h>

#define DEBUG

#define BLOCK_SIZE 4096
#define IDIV_BLOCK_SIZE(x) ((x)>>12)
#define MOD_BLOCK_SIZE(x) (x & 0xfff)

typedef uint64_t bid_t;
typedef uint32_t iid_t;