#pragma once

#include "global.h"
#include "mmio.h"
#include "msr.h"

namespace pcm
{

class HWRegister
{
public:
    virtual void operator = (uint64 val) = 0; // write operation
    virtual operator uint64 () = 0; //read operation
    virtual ~HWRegister() {}
};

class UncorePMU
{
    typedef std::shared_ptr<HWRegister> HWRegisterPtr;
    uint32 cpu_model_;
    uint32 getCPUModel();
    uint32 eventCount;
    //HWRegisterPtr unitControl;
public:
    HWRegisterPtr unitControl;
    HWRegisterPtr counterControl[4];
    HWRegisterPtr counterValue[4];
    HWRegisterPtr fixedCounterControl;
    HWRegisterPtr fixedCounterValue;
    HWRegisterPtr filter[2];

    UncorePMU(const HWRegisterPtr& unitControl_,
        const HWRegisterPtr& counterControl0,
        const HWRegisterPtr& counterControl1,
        const HWRegisterPtr& counterControl2,
        const HWRegisterPtr& counterControl3,
        const HWRegisterPtr& counterValue0,
        const HWRegisterPtr& counterValue1,
        const HWRegisterPtr& counterValue2,
        const HWRegisterPtr& counterValue3,
        const HWRegisterPtr& fixedCounterControl_ = HWRegisterPtr(),
        const HWRegisterPtr& fixedCounterValue_ = HWRegisterPtr(),
        const HWRegisterPtr& filter0 = HWRegisterPtr(),
        const HWRegisterPtr& filter1 = HWRegisterPtr()
    );
    UncorePMU() {}
    virtual ~UncorePMU() {}
    bool valid() const
    {
        return unitControl.get() != nullptr;
    }
    void cleanup();
    void freeze();
    bool initFreeze();
    void unfreeze();
    void resetUnfreeze();
    void print(uint32 ctrl, uint32 counter);

};

class MMIORegister64 : public HWRegister
{
    std::shared_ptr<MMIORange> handle;
    size_t offset;
public:
    MMIORegister64(const std::shared_ptr<MMIORange> & handle_, size_t offset_) :
        handle(handle_),
        offset(offset_)
    {
    }
    void operator = (uint64 val) override
    {
        // std::cout << std::hex << "MMIORegister64 writing " << val << " at offset " << offset << std::dec << std::endl;
        handle->write64(offset, val);
    }
    operator uint64 () override
    {
        const uint64 val = handle->read64(offset);
        // std::cout << std::hex << "MMIORegister64 read " << val << " from offset " << offset << std::dec << std::endl;
        return val;
    }
};

class MMIORegister32 : public HWRegister
{
    std::shared_ptr<MMIORange> handle;
    size_t offset;
public:
    MMIORegister32(const std::shared_ptr<MMIORange> & handle_, size_t offset_) :
        handle(handle_),
        offset(offset_)
    {
    }
    void operator = (uint64 val) override
    {
        // std::cout << std::hex << "MMIORegister32 writing " << val << " at offset " << offset << std::dec << std::endl;
        handle->write32(offset, (uint32)val);
    }
    operator uint64 () override
    {
        const uint64 val = (uint64)handle->read32(offset);
        // std::cout << std::hex << "MMIORegister32 read " << val << " from offset " << offset << std::dec << std::endl;
        return val;
    }
};

class MSRRegister : public HWRegister
{
    std::shared_ptr<SafeMsrHandle> handle;
    size_t offset;
public:
    MSRRegister(const std::shared_ptr<SafeMsrHandle> & handle_, size_t offset_) :
        handle(handle_),
        offset(offset_)
    {
    }
    void operator = (uint64 val) override
    {
        handle->write(offset, val);
    }
    operator uint64 () override
    {
        uint64 value = 0;
        handle->read(offset, &value);
        // std::cout << "reading MSR " << offset << " returning " << value << std::endl;
        return value;
    }
};

class MSRRegister48 : public HWRegister
{
    std::shared_ptr<SafeMsrHandle> handle;
    size_t offset;

public:
    MSRRegister48(const std::shared_ptr<SafeMsrHandle> & handle_, size_t offset_) :
        handle(handle_),
        offset(offset_)
    {
    }
    void operator= (uint64 val) override
    {
        std::cerr << "ERROR: writing to MSR48 is not supported\n";
        throw std::exception();
    }
    operator uint64 () override
    {
        return handle->read48(offset);
    }
};

}   // namespace pcm