#pragma once

#include "global.h"
#include "pci.h"
#include "pmu.h"
#include "utils.h"

namespace pcm
{

class IMC{
  public:
    IMC();
    bool program(std::string configStr);
    void enableFixed();
    void getDRAMClocks(std::vector<std::vector<uint64>>& M);
    void getCounter(std::vector<std::vector<uint64>>& M, int counterId);
    void getDRAMReads(std::vector<uint64>& M);
    void getDRAMWrites(std::vector<uint64>& M);
    virtual ~IMC(){}

    void initFreeze();
    void run();

  private:
    std::vector<std::pair<uint32, uint32>> socket2UBOX0bus;
    static const std::vector<uint32> UBOX0_DEV_IDS;
    std::vector<std::vector<UncorePMU>> imcPMUs;
    std::array<std::vector<std::shared_ptr<MMIORange>>, SERVER_UNCORE_COUNTER_MAX_SOCKETS> imcbasemmioranges;
    int eventCount;

    void initSocket2Ubox0Bus();
    std::vector<size_t> getServerMemBars(const uint32 numIMC,
        const uint32 root_segment_ubox0, const uint32 root_bus_ubox0);

};   // class IMC

inline UncorePMU makeIMCPMU(std::shared_ptr<MMIORange> handle);

}   // namespace pcm