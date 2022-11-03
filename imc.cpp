#include "imc.h"

namespace pcm
{

const std::vector<uint32> IMC::UBOX0_DEV_IDS = { 0x3451, 0x3251 };

IMC::IMC()
{
    initSocket2Ubox0Bus();
    // TODO: change the hardcoded stuff
    imcPMUs.resize(2);
    for(int socket_ = 0; socket_ < imcPMUs.size(); socket_++)
    {
        auto memBars = getServerMemBars(imcno, socket2UBOX0bus[socket_].first, socket2UBOX0bus[socket_].second);
            for (auto & memBar : memBars)
            {
    }
}


void IMC::initSocket2Ubox0Bus()
{
    if(!socket2UBOX0bus.empty()) return;

    const std::vector<MCFGRecord> & mcfg = PciHandleMM::getMCFGRecords();

    for(uint32 s = 0; s < (uint32)mcfg.size(); ++s)
    for (uint32 bus = (uint32)mcfg[s].startBusNumber; bus <= (uint32)mcfg[s].endBusNumber; ++bus)
    {
        uint32 value = 0;
        try
        {
            PciHandleType h(mcfg[s].PCISegmentGroupNumber, bus, SERVER_UBOX0_REGISTER_DEV_ADDR, SERVER_UBOX0_REGISTER_FUNC_ADDR);
            h.read32(0, &value);

        } catch(...)
        {
            // invalid bus:devicei:function
            continue;
        }
        const uint32 vendor_id = value & 0xffff;
        const uint32 device_id = (value >> 16) & 0xffff;
        if (vendor_id != PCM_INTEL_PCI_VENDOR_ID)
           continue;

        for (auto ubox_dev_id : UBOX0_DEV_IDS)
        {
           // match
           if(ubox_dev_id == device_id)
           {
                std::cout << "DEBUG: found bus " << std::hex << bus << " with device ID " << device_id << std::dec << "\n";
                socket2UBOX0bus.push_back(std::make_pair(mcfg[s].PCISegmentGroupNumber,bus));
                break;
           }
        }
    }
}

std::vector<size_t> IMC::getServerMemBars(const uint32 numIMC, const uint32 root_segment_ubox0, const uint32 root_bus_ubox0)
{
    std::vector<size_t> result;
    PciHandleType ubox0Handle(root_segment_ubox0, root_bus_ubox0, SERVER_UBOX0_REGISTER_DEV_ADDR, SERVER_UBOX0_REGISTER_FUNC_ADDR);
    uint32 mmioBase = 0;
    ubox0Handle.read32(0xd0, &mmioBase);
    // std::cout << "mmioBase is 0x" << std::hex << mmioBase << std::dec << std::endl;
    for (uint32 i = 0; i < numIMC; ++i)
    {
        uint32 memOffset = 0;
        ubox0Handle.read32(0xd8 + i * 4, &memOffset);
        // std::cout << "memOffset for imc "<<i<<" is 0x" << std::hex << memOffset << std::dec << std::endl;
        size_t memBar = ((size_t(mmioBase) & ((1ULL << 29ULL) - 1ULL)) << 23ULL) |
            ((size_t(memOffset) & ((1ULL << 11ULL) - 1ULL)) << 12ULL);
         //std::cout << "membar for imc "<<i<<" is 0x" << std::hex << memBar << std::dec << std::endl;
        if (memBar == 0)
        {
            std::cerr << "ERROR: memBar " << i << " is zero." << std::endl;
            throw std::exception();
        }
        result.push_back(memBar);
    }
    return result;
}


}   // namespace pcm