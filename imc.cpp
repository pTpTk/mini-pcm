#include "imc.h"

namespace pcm
{
inline UncorePMU makeIMCPMU(std::shared_ptr<MMIORange> handle)
{
    return UncorePMU(
        std::make_shared<MMIORegister32>(handle, SERVER_MC_CH_PMON_BOX_CTL_OFFSET),
        std::make_shared<MMIORegister32>(handle, SERVER_MC_CH_PMON_CTL0_OFFSET),
        std::make_shared<MMIORegister32>(handle, SERVER_MC_CH_PMON_CTL1_OFFSET),
        std::make_shared<MMIORegister32>(handle, SERVER_MC_CH_PMON_CTL2_OFFSET),
        std::make_shared<MMIORegister32>(handle, SERVER_MC_CH_PMON_CTL3_OFFSET),
        std::make_shared<MMIORegister64>(handle, SERVER_MC_CH_PMON_CTR0_OFFSET),
        std::make_shared<MMIORegister64>(handle, SERVER_MC_CH_PMON_CTR1_OFFSET),
        std::make_shared<MMIORegister64>(handle, SERVER_MC_CH_PMON_CTR2_OFFSET),
        std::make_shared<MMIORegister64>(handle, SERVER_MC_CH_PMON_CTR3_OFFSET),
        std::make_shared<MMIORegister32>(handle, SERVER_MC_CH_PMON_FIXED_CTL_OFFSET),
        std::make_shared<MMIORegister64>(handle, SERVER_MC_CH_PMON_FIXED_CTR_OFFSET)
    );
}

const std::vector<uint32> IMC::UBOX0_DEV_IDS = { 0x3451, 0x3251 };

IMC::IMC()
{
    eventCount = 0;
    initSocket2Ubox0Bus();
    // TODO: change the hardcoded stuff
    int numChannels = 3;
    int imcno = 4;
    imcPMUs.resize(2);
    for(int socket_ = 0; socket_ < imcPMUs.size(); socket_++)
    {
        auto memBars = getServerMemBars(imcno, socket2UBOX0bus[socket_].first, socket2UBOX0bus[socket_].second);
        for (auto & memBar : memBars)
        {
            std::cout << "membar for imc " << std::hex << memBar << std::dec << std::endl;
            imcbasemmioranges[socket_].push_back(std::make_shared<MMIORange>(memBar, PCM_SERVER_IMC_MMAP_SIZE));
            for (int channel = 0; channel < numChannels; ++channel)
            {
                auto handle = std::make_shared<MMIORange>(memBar + SERVER_MC_CH_PMON_BASE_ADDR + channel * SERVER_MC_CH_PMON_STEP,
                                                          SERVER_MC_CH_PMON_SIZE, false);
                std::cout << "debug BOX CTL is at " << std::hex 
                          << (memBar + SERVER_MC_CH_PMON_BASE_ADDR + channel * SERVER_MC_CH_PMON_STEP) << std::dec << std::endl;
                imcPMUs[socket_].push_back(makeIMCPMU(handle));
            }
        }

        if (imcPMUs[socket_].empty())
        {
            std::cerr << "PCM error: no memory controllers found.\n";
            throw std::exception();
        }
    }
}


bool IMC::program(std::string configStr){
    if (eventCount == SERVER_UNCORE_COUNTER_MAX_CONTROLLERS)
    {
        std::cerr << "too many events for IMC unit" << std::endl;
        return false;
    }

    const auto configArray = split(configStr, ',');
    bool fixed = false; // TODO: find out where fixed is used and implement support
    uint32 event;
    for (auto item : configArray)
    {
        std::string f;
        if (match("config=(0[xX][0-9a-fA-F]+)", item, f)) {
            event = strtoll(f.c_str(), NULL, 16);
            std::cout << "Config read" << event << "\n";	
        }
    }

    for (auto& imcPMUsPerSocket : imcPMUs)
    {
        for (auto& imcPMU : imcPMUsPerSocket)
        {
            imcPMU.initFreeze();
            // enable fixed counter (DRAM clocks)
            *imcPMU.fixedCounterControl = MC_CH_PCI_PMON_FIXED_CTL_EN;
            // reset it
            *imcPMU.fixedCounterControl = MC_CH_PCI_PMON_FIXED_CTL_EN + MC_CH_PCI_PMON_FIXED_CTL_RST;
        
            auto ctrl = imcPMU.counterControl[eventCount];
            *ctrl = event;

            imcPMU.resetUnfreeze();
        }
    }

    eventCount++;
    return true;
}

void IMC::getDRAMClocks(std::vector<std::vector<uint64>>& M)
{
    M.resize(imcPMUs.size());
    for(int i = 0; i < imcPMUs.size(); ++i){
        M[i].resize(imcPMUs[i].size());
    }

    for(int i = 0; i < imcPMUs.size(); ++i){
        for(int j = 0; j < imcPMUs[i].size(); ++j){
            imcPMUs[i][j].freeze();
            M[i][j] = *(imcPMUs[i][j].fixedCounterValue);
            imcPMUs[i][j].unfreeze();
        }
    }
}

void IMC::getMCCounter(std::vector<std::vector<uint64>>& M, int counterId)
{
    if (counterId >= eventCount){
        std::cerr << "Trying to read unused counter " << counterId;
        return;
    }
    M.resize(imcPMUs.size());
    for(int i = 0; i < imcPMUs.size(); ++i){
        M[i].resize(imcPMUs[i].size());
    }

    for(int i = 0; i < imcPMUs.size(); ++i){
        for(int j = 0; j < imcPMUs[i].size(); ++j){
            imcPMUs[i][j].freeze();
            M[i][j] = *(imcPMUs[i][j].counterValue[counterId]);
            imcPMUs[i][j].unfreeze();
        }
    }
}

std::vector<uint64> getDRAMReads();
std::vector<uint64> getDRAMWrites();

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