#pragma once

#include "global.h"

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

}   // namespace pcm