#pragma once

#include "global.h"

namespace pcm
{

class MMIORange
{
    int32 fd;
    char * mmapAddr;
    const uint64 size;
    const bool readonly;
public:
    MMIORange(uint64 baseAddr_, uint64 size_, bool readonly_ = true);
    uint32 read32(uint64 offset);
    uint64 read64(uint64 offset);
    void write32(uint64 offset, uint32 val);
    void write64(uint64 offset, uint64 val);
    ~MMIORange();
};

}   // namespace pcm