/*
Copyright (c) 2009-2020, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// written by Roman Dementiev
//            Thomas Willhalm


/*!     \file cpucounters.h
        \brief Main CPU counters header

        Include this header file if you want to access CPU counters (core and uncore - including memory controller chips and QPI)
*/




#include <iostream>
#include <cstdio>
#include <cstring>
#include <fstream>
#include "types.h"
#include "pci.h"
#include <stdio.h>
#include <vector>
#include <array>
#include <limits>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>
#include <string.h>
#include <assert.h>


namespace sprimc {

#define O_RDONLY 0
#define O_RDWR 2
//typedef unsigned int uint32;
//typedef unsigned long uint64;

/*
        CPU performance monitoring routines

        A set of performance monitoring routines for recent Intel CPUs
*/

#define PCM_INTEL_PCI_VENDOR_ID (0x8086)
#define PCM_INVALID_DEV_ADDR (~(uint32)0UL)
#define PCM_INVALID_FUNC_ADDR (~(uint32)0UL)
#define SERVER_MC_CH_PMON_BASE_ADDR        (SERVER_MC_CH_PMON_REAL_BASE_ADDR - SERVER_MC_CH_PMON_BASE_ALIGN_DELTA)

#define SERVER_MC_CH_PMON_REAL_BASE_ADDR   (0x22800)
#define SERVER_MC_CH_PMON_BASE_ALIGN_DELTA (0x00800)
#define SERVER_MC_CH_PMON_BASE_ADDR        (SERVER_MC_CH_PMON_REAL_BASE_ADDR - SERVER_MC_CH_PMON_BASE_ALIGN_DELTA)		
//#define SERVER_MC_CH_PMON_STEP             (0x4000)
#define SERVER_MC_CH_PMON_STEP             (0x8000) // It seems 3 channels were assigned and currently ch0 is mapped to ch0 and ch1 is mapped to ch2
#define SERVER_MC_CH_PMON_SIZE             (0x1000)
#define SERVER_MC_CH_PMON_BOX_CTL_OFFSET   (0x00 + SERVER_MC_CH_PMON_BASE_ALIGN_DELTA)
#define SERVER_MC_CH_PMON_CTL0_OFFSET      (0x40 + SERVER_MC_CH_PMON_BASE_ALIGN_DELTA)
#define SERVER_MC_CH_PMON_CTL1_OFFSET      (SERVER_MC_CH_PMON_CTL0_OFFSET + 4*1)
#define SERVER_MC_CH_PMON_CTL2_OFFSET      (SERVER_MC_CH_PMON_CTL0_OFFSET + 4*2)
#define SERVER_MC_CH_PMON_CTL3_OFFSET      (SERVER_MC_CH_PMON_CTL0_OFFSET + 4*3)
#define SERVER_MC_CH_PMON_CTR0_OFFSET      (0x08 + SERVER_MC_CH_PMON_BASE_ALIGN_DELTA)
#define SERVER_MC_CH_PMON_CTR1_OFFSET      (SERVER_MC_CH_PMON_CTR0_OFFSET + 8*1)
#define SERVER_MC_CH_PMON_CTR2_OFFSET      (SERVER_MC_CH_PMON_CTR0_OFFSET + 8*2)
#define SERVER_MC_CH_PMON_CTR3_OFFSET      (SERVER_MC_CH_PMON_CTR0_OFFSET + 8*3)
#define SERVER_MC_CH_PMON_FIXED_CTL_OFFSET (0x54 + SERVER_MC_CH_PMON_BASE_ALIGN_DELTA)
#define SERVER_MC_CH_PMON_FIXED_CTR_OFFSET (0x38 + SERVER_MC_CH_PMON_BASE_ALIGN_DELTA)
#define SPR_UNC_PMON_UNIT_CTL_FRZ          (1 << 0)
#define SPR_UNC_PMON_UNIT_CTL_RST_CONTROL  (1 << 8)
#define SPR_UNC_PMON_UNIT_CTL_RST_COUNTERS (1 << 9)
#define MC_CH_PCI_PMON_FIXED_CTL_RST (1 << 19)
#define MC_CH_PCI_PMON_FIXED_CTL_EN (1 << 22)
#define PCM_SERVER_IMC_PMM_DATA_READS   (0x22a0)
#define PCM_SERVER_IMC_PMM_DATA_WRITES  (0x22a8)
#define PCM_SERVER_IMC_MMAP_SIZE        (0x4000)
#define PCM_SERVER_IMC_DRAM_DATA_READS  (0x2290)
#define PCM_SERVER_IMC_DRAM_DATA_WRITES (0x2298)
constexpr auto SPR_CHA0_MSR_PMON_BOX_CTRL   = 0x2000;
constexpr auto SPR_CHA0_MSR_PMON_CTL0       = 0x2002;
constexpr auto SPR_CHA0_MSR_PMON_CTR0       = 0x2008;
constexpr auto SPR_CHA0_MSR_PMON_BOX_FILTER = 0x200E;
constexpr auto SPR_CHA_MSR_STEP = 0x10;
constexpr auto SPR_M2IOSF_IIO_UNIT_CTL = 0x3000;
constexpr auto SPR_M2IOSF_IIO_CTR0     = 0x3008;
constexpr auto SPR_M2IOSF_IIO_CTL0     = 0x3002;
constexpr auto SPR_M2IOSF_REG_STEP = 0x10;
constexpr auto SPR_M2IOSF_NUM      = 12;




class s_expect : public std::string
{
public:
    explicit s_expect(const char * s) : std::string(s) {}
    explicit s_expect(const std::string & s) : std::string(s) {}
    friend std::istream & operator >> (std::istream & istr, s_expect && s);
    friend std::istream & operator >> (std::istream && istr, s_expect && s);
private:

    void match(std::istream & istr) const
    {
        istr >> std::noskipws;
        const auto len = length();
        char * buffer = new char[len + 2];
        buffer[0] = 0;
        istr.get(buffer, len+1);
        if (*this != std::string(buffer))
        {
            istr.setstate(std::ios_base::failbit);
        }
        delete [] buffer;
    }
};
std::vector<std::string> split(const std::string & str, const char delim);

//bool match(const std::string& subtoken, const std::string& sname, uint64* result);
bool match(const std::string& subtoken, const std::string& sname, std::string& result);
typedef std::istringstream pcm_sscanf;
	
	
enum CsvOutputType
{
    Header1,
    Header2,
    Data
};

template <class H1, class H2, class D>
inline void choose(const CsvOutputType outputType, H1 h1Func, H2 h2Func, D dataFunc)
{
    switch (outputType)
    {
    case Header1:
        h1Func();
        break;
    case Header2:
        h2Func();
        break;
    case Data:
        dataFunc();
        break;
    default:
        std::cerr << "PCM internal error: wrong CSvOutputType\n";
    }
}
	
	
	
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


enum ErrorCode {
Success = 0,
MSRAccessDenied = 1,
PMUBusy = 2,
UnknownError
};

class HWRegister
{
public:
    virtual void operator = (uint64 val) = 0; // write operation
    virtual operator uint64 () = 0; //read operation
    virtual ~HWRegister() {}
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

/*class MSRRegister : public HWRegister
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
*/

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




class ServerUncoreCounterState
{
public:
    enum {
        maxControllers = 4,
        //maxChannels = 12,
        maxChannels = 8,
        maxXPILinks = 6,
        maxCBOs = 128,
        maxIIOStacks = 16,
        maxCounters = 4,
	maxsockets = 2
    };
    enum FreeRunningCounterID
    {
        ImcReads,
        ImcWrites,
        PMMReads,
        PMMWrites
    };
    std::array<std::array<uint64, maxChannels>, maxsockets> DRAMClocks;
    std::array<uint64, maxsockets> DRAMreads;
    std::array<uint64, maxsockets> DRAMwrites;
    //std::array<std::array<std::array<uint64, maxCounters>, maxChannels>, maxsockets> MCCounter; // channel X counter
    std::array<std::array<uint64, maxCounters>, maxChannels>  MCCounter; // channel X counter
    std::array<std::unordered_map<int, uint64>,  maxsockets> freeRunningCounter;
    uint64 InvariantTSC;    // invariant time stamp counter

    //! Returns current thermal headroom below TjMax
    ServerUncoreCounterState() :
        DRAMClocks{{}},
        MCCounter{{}},
        InvariantTSC(0)
    {
    }
};

//event variables

    typedef std::pair<std::array<uint64, 3>, std::string> RawEventConfig;
    struct RawPMUConfig
    {
        std::vector<RawEventConfig> programmable;
        std::vector<RawEventConfig> fixed;
    };
    typedef std::map<std::string, RawPMUConfig> RawPMUConfigs;
    ErrorCode programevent(const RawPMUConfigs& allPMUConfigs);




//! Object to access uncore counters in a socket/processor with microarchitecture codename SandyBridge-EP (Jaketown) or Ivytown-EP or Ivytown-EX
    typedef std::vector<UncorePMU> UncorePMUVector;
    //std::array<UncorePMUVector, ServerUncoreCounterState::maxsockets> imcPMUs;
    //std::vector<uint32> num_imc_channels; // number of memory channels in each memory controller

    template <class Iterator>
    static void program(UncorePMU& pmu, const Iterator& eventsBegin, const Iterator& eventsEnd)
    {
        if (!eventsBegin) return;
        Iterator curEvent = eventsBegin;
        for (int c = 0; curEvent != eventsEnd; ++c, ++curEvent)
        {
            auto ctrl = pmu.counterControl[c];
            if (ctrl.get() != nullptr)
            {
                    *ctrl = *curEvent;
            }
        }
            pmu.resetUnfreeze();
    }

    void programIMC(const uint32 * MCCntConfig, uint32 socket);
    void instantiateIMC(uint32 socket_);
    uint64 getPMUCounter(std::vector<UncorePMU> & pmu, const uint32 id, const uint32 counter);
    uint64 getMCCounter(uint32 socket, uint32 channel, uint32 counter);
    uint64 getDRAMClocks(uint32 socket, uint32 channel);
    ServerUncoreCounterState getServerUncoreCounterState(uint32 socket);
    }
