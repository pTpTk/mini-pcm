#include "cha.h"

namespace pcm
{

CHA::CHA()
{
    cboPMUs.resize(2);
    for (uint32 s = 0; s < cboPMUs.size(); ++s)
    {
        auto & handle = MSR[socketRefCore[s]];
        for (uint32 cbo = 0; cbo < getMaxNumOfCBoxes(); ++cbo)
        {
            cboPMUs[s].push_back(
                UncorePMU(
                    std::make_shared<MSRRegister>(handle, CX_MSR_PMON_BOX_CTL(cbo)),
                    std::make_shared<MSRRegister>(handle, CX_MSR_PMON_CTLY(cbo, 0)),
                    std::make_shared<MSRRegister>(handle, CX_MSR_PMON_CTLY(cbo, 1)),
                    std::make_shared<MSRRegister>(handle, CX_MSR_PMON_CTLY(cbo, 2)),
                    std::make_shared<MSRRegister>(handle, CX_MSR_PMON_CTLY(cbo, 3)),
                    std::make_shared<CounterWidthExtenderRegister>(
                        std::make_shared<CounterWidthExtender>(new CounterWidthExtender::MsrHandleCounter(MSR[socketRefCore[s]], CX_MSR_PMON_CTRY(cbo, 0)), 48, 5555)),
                    std::make_shared<CounterWidthExtenderRegister>(
                        std::make_shared<CounterWidthExtender>(new CounterWidthExtender::MsrHandleCounter(MSR[socketRefCore[s]], CX_MSR_PMON_CTRY(cbo, 1)), 48, 5555)),
                    std::make_shared<CounterWidthExtenderRegister>(
                        std::make_shared<CounterWidthExtender>(new CounterWidthExtender::MsrHandleCounter(MSR[socketRefCore[s]], CX_MSR_PMON_CTRY(cbo, 2)), 48, 5555)),
                    std::make_shared<CounterWidthExtenderRegister>(
                        std::make_shared<CounterWidthExtender>(new CounterWidthExtender::MsrHandleCounter(MSR[socketRefCore[s]], CX_MSR_PMON_CTRY(cbo, 3)), 48, 5555)),
                    std::shared_ptr<MSRRegister>(),
                    std::shared_ptr<MSRRegister>(),
                    std::make_shared<MSRRegister>(handle, CX_MSR_PMON_BOX_FILTER(cbo)),
                    std::shared_ptr<HWRegister>();
                )
            );
        }
    }
}

utint32 CHA::getMaxNumOfCBoxes() const
{
    static int num = -1;
    if(num >= 0) return (uint32)num;

    PciHandleType * h = getDeviceHandle(PCM_INTEL_PCI_VENDOR_ID, 0x325b);
    if(h)
    {
        uint32 value;
        h->read32(0x9c, &value);
        num = weight32(value);
        h->read32(0xa0, &value);
        num += weight32(value);
        delete h;
    }

    return num;
}

int32 CHA::getNumCores

}   // namespace pcm