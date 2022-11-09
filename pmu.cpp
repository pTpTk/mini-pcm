#include "pmu.h"

namespace pcm
{

UncorePMU::UncorePMU(const HWRegisterPtr& unitControl_,
    const HWRegisterPtr& counterControl0,
    const HWRegisterPtr& counterControl1,
    const HWRegisterPtr& counterControl2,
    const HWRegisterPtr& counterControl3,
    const HWRegisterPtr& counterValue0,
    const HWRegisterPtr& counterValue1,
    const HWRegisterPtr& counterValue2,
    const HWRegisterPtr& counterValue3,
    const HWRegisterPtr& fixedCounterControl_,
    const HWRegisterPtr& fixedCounterValue_,
    const HWRegisterPtr& filter0,
    const HWRegisterPtr& filter1
) :
    cpu_model_(0),
    eventCount(0),
    unitControl(unitControl_),
    counterControl{ counterControl0, counterControl1, counterControl2, counterControl3 },
    counterValue{ counterValue0, counterValue1, counterValue2, counterValue3 },
    fixedCounterControl(fixedCounterControl_),
    fixedCounterValue(fixedCounterValue_),
    filter{ filter0 , filter1 }
{}

void UncorePMU::cleanup()
{
    for (int i = 0; i < 4; ++i)
    {
        if (counterControl[i].get()) *counterControl[i] = 0;
    }
    if (unitControl.get()) *unitControl = 0;
    if (fixedCounterControl.get()) *fixedCounterControl = 0;
}

void UncorePMU::freeze()
{
    *unitControl = SPR_UNC_PMON_UNIT_CTL_FRZ ;
}

void UncorePMU::unfreeze()
{
    *unitControl =  0;
}

void UncorePMU::print(uint32 ctrl, uint32 counter){
	std::cout << ctrl << ": counter Control handle address is" << std::hex << counterControl[ctrl] << std::dec << std::endl;
	//std::cout << "Control Setting for ctr:" << ctrl << std::hex << *counterControl[ctrl] << std::dec << std::endl;
	//std::cout << "Counter Value for CTR:" << counter << std::hex << *counterValue[counter] << std::dec << std::endl;
}


bool UncorePMU::initFreeze()
{
        *unitControl = SPR_UNC_PMON_UNIT_CTL_FRZ; // freeze
        *unitControl = SPR_UNC_PMON_UNIT_CTL_FRZ + SPR_UNC_PMON_UNIT_CTL_RST_CONTROL; // freeze and reset control registers
        return true;
    // freeze enable bit 0 is set to 1 to freeze unit counters and bit 8 to 1 to freeze counter controls 
    // freeze

}

void UncorePMU::resetUnfreeze()
{
        *unitControl = SPR_UNC_PMON_UNIT_CTL_FRZ + SPR_UNC_PMON_UNIT_CTL_RST_COUNTERS; // counters freeze and  additionaly counter are reset with bit 9 set to 1
        *unitControl = 0; // unfreeze and take everything our of freeze and reset.
        return;
}

MMIORange::MMIORange(uint64 baseAddr_, uint64 size_, bool readonly_) :
    fd(-1),
    mmapAddr(NULL),
    size(size_),
    readonly(readonly_)
{
    const int oflag = readonly ? O_RDONLY : O_RDWR;
    int handle = ::open("/dev/mem", oflag);
    if (handle < 0)
    {
       std::cerr << "opening /dev/mem failed: errno is " << errno << " (" << strerror(errno) << ")\n";
       throw std::exception();
    }
    fd = handle;

    const int prot = readonly ? PROT_READ : (PROT_READ | PROT_WRITE);
    mmapAddr = (char *)mmap(NULL, size, prot, MAP_SHARED, fd, baseAddr_);

    if (mmapAddr == MAP_FAILED)
    {
        std::cerr << "mmap failed: errno is " << errno << " (" << strerror(errno) << ")\n";
        throw std::exception();
    }
}

uint32 MMIORange::read32(uint64 offset)
{
    return *((uint32 *)(mmapAddr + offset));
}

uint64 MMIORange::read64(uint64 offset)
{
    return *((uint64 *)(mmapAddr + offset));
}

void MMIORange::write32(uint64 offset, uint32 val)
{
    if (readonly)
    {
        std::cerr << "PCM Error: attempting to write to a read-only MMIORange\n";
        return;
    }
    *((uint32 *)(mmapAddr + offset)) = val;
}
void MMIORange::write64(uint64 offset, uint64 val)
{
    if (readonly)
    {
        std::cerr << "PCM Error: attempting to write to a read-only MMIORange\n";
        return;
    }
    *((uint64 *)(mmapAddr + offset)) = val;
}

MMIORange::~MMIORange()
{
    if (mmapAddr) munmap(mmapAddr, size);
    if (fd >= 0) ::close(fd);
}

}   // namespace pcm