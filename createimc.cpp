
#include <stdio.h>
#include <assert.h>
#include "createimc.h"
#include "types.h"
#include "pci.h"
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <limits>
#include <map>
#include <algorithm>
#include <thread>
#include <future>
#include <functional>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <regex>
#define SERVER_UBOX0_REGISTER_DEV_ADDR  (0)
#define SERVER_UBOX0_REGISTER_FUNC_ADDR (1)



namespace sprimc {

static const uint32 UBOX0_DEV_IDS[] = {
    0x3451,
    0x3251
};


std::array<UncorePMUVector, ServerUncoreCounterState::maxsockets> imcPMUs;
std::array<std::vector<std::shared_ptr<MMIORange> >, ServerUncoreCounterState::maxsockets> imcbasemmioranges;
std::vector<uint32> num_imc_channels; // number of memory channels in each memory controller

//typedef std::istringstream pcm_sscanf;	
std::vector<std::string> split(const std::string & str, const char delim)
{
    std::string token;
    std::vector<std::string> result;
    std::istringstream strstr(str);
    while (std::getline(strstr, token, delim))
    {
        result.push_back(token);
    }
    return result;
}
/*
uint64 read_number(char* str)
{
    std::istringstream stream(str);
    if (strstr(str, "x")) stream >> std::hex;
    uint64 result = 0;
    stream >> result;
    return result;
}
*/

// emulates scanf %i for hex 0x prefix otherwise assumes dec (no oct support)
/*
bool match(const std::string& subtoken, const std::string& sname, uint64* result)
{
    if (pcm_sscanf(subtoken) >> s_expect(sname + "0x") >> std::hex >> *result)
        return true;

    if (pcm_sscanf(subtoken) >> s_expect(sname) >> std::dec >> *result)
        return true;

    return false;
}

*/
bool match(const std::string& subtoken, const std::string& sname, std::string& result)
{

	std::regex rgx(subtoken);
	std::smatch matched;
	if (std::regex_search(sname.begin(), sname.end(), matched, rgx)){
		std::cout << "match: " << matched[1] << '\n';
		result= matched[1];
		return true;
	}
	return false;

}




std::vector<std::pair<uint32, uint32> > socket2UBOX0bus;

void initSocket2Bus(std::vector<std::pair<uint32, uint32> > & socket2bus, uint32 device, uint32 function, const uint32 DEV_IDS[], uint32 devIdsSize);

void initSocket2Ubox0Bus()
{
    initSocket2Bus(socket2UBOX0bus, SERVER_UBOX0_REGISTER_DEV_ADDR, SERVER_UBOX0_REGISTER_FUNC_ADDR,
        UBOX0_DEV_IDS, (uint32)sizeof(UBOX0_DEV_IDS) / sizeof(UBOX0_DEV_IDS[0]));
}





ErrorCode  programevent(const RawPMUConfigs& allPMUConfigs_)
{
    RawPMUConfigs allPMUConfigs = allPMUConfigs_;
    constexpr auto globalRegPos = 0;
    for (auto pmuConfig : allPMUConfigs)
    {
        const auto & type = pmuConfig.first;
        const auto & events = pmuConfig.second;
        if (events.programmable.empty() && events.fixed.empty())
        {
            continue;
        }
        if (events.programmable.size() > ServerUncoreCounterState::maxCounters)
        {
            std::cerr << "ERROR: trying to program " << events.programmable.size() << " core PMU counters, which exceeds the max num possible (" << ServerUncoreCounterState::maxCounters << ").";
            return ErrorCode::UnknownError;
        }
        uint32 events32[] = { 0,0,0,0 };
        uint64 events64[] = { 0,0,0,0 };
        for (size_t c = 0; c < events.programmable.size() && c < ServerUncoreCounterState::maxCounters; ++c)
        {
            events32[c] = (uint32)events.programmable[c].first[0];
            events64[c] = events.programmable[c].first[0];
        }
        if (type == "imc")
        {
		for (uint32 i =0; i <  ServerUncoreCounterState::maxsockets; i++){
                	programIMC(events32, i);
		}
        }
        else if (type == "cbo" || type == "cha")
        {
        }
        else
        {
            std::cerr << "ERROR: unrecognized PMU type \"" << type << "\"\n";
            return ErrorCode::UnknownError;
        }
    }
    return ErrorCode::Success;
}









// Explicit instantiation needed in topology.cpp



ServerUncoreCounterState getServerUncoreCounterState(uint32 socket)
{
    ServerUncoreCounterState result;
    uint64 tmp;
        for (uint32 channel = 0; channel < 8; ++channel)
        {
            assert(channel < result.DRAMClocks[socket].size());
            result.DRAMClocks[socket][channel] = getDRAMClocks(socket, channel);
            for (uint32 cnt = 0; cnt < ServerUncoreCounterState::maxCounters; ++cnt) {
		    tmp = getMCCounter(socket, channel, cnt);
                result.MCCounter[channel][cnt] = tmp;
	    }
        }
    // std::cout << std::flush;
    for (auto & mmio : imcbasemmioranges[socket])
    {       //int i =0;
	    result.DRAMreads[socket] += mmio->read64(PCM_SERVER_IMC_DRAM_DATA_READS);
	    result.DRAMwrites[socket] += mmio->read64(PCM_SERVER_IMC_DRAM_DATA_WRITES);
	    //++i;
    }

    return result;
}



static const uint32 IMC_DEV_IDS[] = {
    0x03cb0,
    0x03cb1,
    0x03cb4,
    0x03cb5,
    0x0EB4,
    0x0EB5,
    0x0EB0,
    0x0EB1,
    0x0EF4,
    0x0EF5,
    0x0EF0,
    0x0EF1,
    0x2fb0,
    0x2fb1,
    0x2fb4,
    0x2fb5,
    0x2fd0,
    0x2fd1,
    0x2fd4,
    0x2fd5,
    0x6fb0,
    0x6fb1,
    0x6fb4,
    0x6fb5,
    0x6fd0,
    0x6fd1,
    0x6fd4,
    0x6fd5,
    0x2042,
    0x2046,
    0x204a,
    0x7840,
    0x7841,
    0x7842,
    0x7843,
    0x7844,
    0x781f
};

static const uint32 UPI_DEV_IDS[] = {
    0x2058,
    0x3441,
    0x3241,
};

static const uint32 M2M_DEV_IDS[] = {
    0x2066,
    0x344A,
    0x324A
};


void initSocket2Bus(std::vector<std::pair<uint32, uint32> > & socket2bus, uint32 device, uint32 function, const uint32 DEV_IDS[], uint32 devIdsSize)
{
    if (device == PCM_INVALID_DEV_ADDR || function == PCM_INVALID_FUNC_ADDR)
    {
        return;
    }
    if(!socket2bus.empty()) return;

    const std::vector<MCFGRecord> & mcfg = PciHandleMM::getMCFGRecords();

    for(uint32 s = 0; s < (uint32)mcfg.size(); ++s)
    for (uint32 bus = (uint32)mcfg[s].startBusNumber; bus <= (uint32)mcfg[s].endBusNumber; ++bus)
    {
        uint32 value = 0;
        try
        {
            PciHandleType h(mcfg[s].PCISegmentGroupNumber, bus, device, function);
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

        for (uint32 i = 0; i < devIdsSize; ++i)
        {
           // match
           if(DEV_IDS[i] == device_id)
           {
                std::cout << "DEBUG: found bus " << std::hex << bus << " with device ID " << device_id << std::dec << "\n";
               socket2bus.push_back(std::make_pair(mcfg[s].PCISegmentGroupNumber,bus));
               break;
           }
        }
    }
    //std::cout << std::flush;
}




std::vector<size_t> getServerMemBars(const uint32 numIMC, const uint32 root_segment_ubox0, const uint32 root_bus_ubox0)
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




void instantiateIMC(uint32 socket_)
{

    int imcno = 4;
    int numChannels = 2;

    if (numChannels > 0)
    {
        initSocket2Ubox0Bus();
        if (socket_ < socket2UBOX0bus.size())
        {
            auto memBars = getServerMemBars(imcno, socket2UBOX0bus[socket_].first, socket2UBOX0bus[socket_].second);
            for (auto & memBar : memBars)
            {
		std::cout << "membar for imc " << std::hex << memBar << std::dec << std::endl;
		imcbasemmioranges[socket_].push_back(std::make_shared<MMIORange>(memBar, PCM_SERVER_IMC_MMAP_SIZE));
                for (int channel = 0; channel < numChannels; ++channel)
                {
                    auto handle = std::make_shared<MMIORange>(memBar + SERVER_MC_CH_PMON_BASE_ADDR + channel * SERVER_MC_CH_PMON_STEP, SERVER_MC_CH_PMON_SIZE, false);
		    std::cout << "debug BOX CTL is at " << std::hex << (memBar + SERVER_MC_CH_PMON_BASE_ADDR + channel * SERVER_MC_CH_PMON_STEP) << std::dec << std::endl;
                    imcPMUs[socket_].push_back(
                        UncorePMU(
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
                        )
                    );
                }
                num_imc_channels.push_back(numChannels);
            }
        }
        else
        {
            std::cerr << "ERROR: socket " << socket_ << " is not found in socket2UBOX0bus. socket2UBOX0bus.size =" << socket2UBOX0bus.size() << std::endl;
        }
    }

    if (imcPMUs[socket_].empty())
    {
        std::cerr << "PCM error: no memory controllers found.\n";
        throw std::exception();
    }


}











/*
 * Verify that PMU counters are programmed accordingly before using these functions.
uint64 ServerPCICFGUncore::getImcReads()
{
    return getImcReadsForChannels((uint32)0, (uint32)imcPMUs.size());
}


uint64 ServerPCICFGUncore::getImcReadsForChannels(uint32 beginChannel, uint32 endChannel)
{
    uint64 result = 0;
    for (uint32 i = beginChannel; i < endChannel && i < imcPMUs.size(); ++i)
    {
        result += getMCCounter(i, EventPosition::READ);
    }
    return result;
}

uint64 ServerPCICFGUncore::getImcWrites()
{
    uint64 result = 0;
    for (uint32 i = 0; i < (uint32)imcPMUs.size(); ++i)
    {
        result += getMCCounter(i, EventPosition::WRITE);
    }

    return result;
}
this is just programming one of the 4 counters
*/

void programIMC(const uint32 * MCCntConfig, uint32 socket)
{
    //const uint32 extraIMC = UNC_PMON_UNIT_CTL_FRZ_EN; SPR does not need extra FRZ enable
    for (uint32 i = 0; i < (uint32)imcPMUs[socket].size(); ++i)
    {
        // imc PMU
	int check = 0;
        imcPMUs[socket][i].initFreeze();


        // enable fixed counter (DRAM clocks)
        *imcPMUs[socket][i].fixedCounterControl = MC_CH_PCI_PMON_FIXED_CTL_EN;

        // reset it
        *imcPMUs[socket][i].fixedCounterControl = MC_CH_PCI_PMON_FIXED_CTL_EN + MC_CH_PCI_PMON_FIXED_CTL_RST;

	std::cout << "Setting control for socket " << socket << "and channel " << i << "\n"; 
	std::cout << "counter0 unitcontrol is at " << std::hex << (imcPMUs[socket][i].unitControl) << std::dec << std::endl; 
        program(imcPMUs[socket][i], MCCntConfig, MCCntConfig + 4);
    }
}




//beloiw functions all assuming imcPMUs are instantiated at the highest level

uint64 getDRAMClocks(uint32 socket, uint32 channel)
{
    uint64 result = 0;

    if (channel < (uint32)imcPMUs[socket].size()) {
	imcPMUs[socket][channel].freeze();
        result = *(imcPMUs[socket][channel].fixedCounterValue);
    	imcPMUs[socket][channel].unfreeze();
    }

    // std::cout << "DEBUG: DRAMClocks on channel " << channel << " = " << result << "\n";
    return result;
}

//this is a method defined on UncorePMU and hence can access all its variables
uint64 getPMUCounter(std::vector<UncorePMU> & pmu, const uint32 id, const uint32 counter)
{
    uint64 result = 0;

    if (id < (uint32)pmu.size() && counter < 4 && pmu[id].counterValue[counter].get() != nullptr)
    {
	pmu[id].freeze();
	//pmu[id].initFreeze();
        result = *(pmu[id].counterValue[counter]);
	//std::cout << "channel:" << id << "value=" << result << "\n";
	pmu[id].unfreeze();
	/*
	if(counter == 0){
		std::cout << "::Ch:" << id  ;
		pmu[id].print(counter, counter); 
	} 
	*/
    }
    else
    {
        //std::cout << "DEBUG: Invalid ServerPCICFGUncore::getPMUCounter(" << id << ", " << counter << ") \n";
    }
    // std::cout << "DEBUG: ServerPCICFGUncore::getPMUCounter(" << id << ", " << counter << ") = " << result << "\n";
    return result;
}

uint64 getMCCounter(uint32 socket, uint32 channel, uint32 counter)
{
//	std::cout << "No of imcPMUS is " << imcPMUs[socket].size() << "\n";
     //std::cout << "socket-id:" << socket  ;
    return getPMUCounter(imcPMUs[socket], channel, counter);
}


// Return the first device found with specific vendor/device IDs



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
    unitControl(unitControl_),
    counterControl{ counterControl0, counterControl1, counterControl2, counterControl3 },
    counterValue{ counterValue0, counterValue1, counterValue2, counterValue3 },
    fixedCounterControl(fixedCounterControl_),
    fixedCounterValue(fixedCounterValue_),
    filter{ filter0 , filter1 }
{
}


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

}
