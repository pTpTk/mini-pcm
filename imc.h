#pragma once

#include "global.h"
#include "pci.h"
#include "pmu.h"

namespace pcm
{

class IMC{
  public:
    IMC();
    virtual ~IMC() = 0;

  private:
    std::vector<std::pair<uint32, uint32>> socket2UBOX0bus;
    static const std::vector<uint32> UBOX0_DEV_IDS;
    std::vector<std::vector<UncorePMU>> imcPMUs;


    void initSocket2Ubox0Bus();
    std::vector<size_t> getServerMemBars(const uint32 numIMC,
        const uint32 root_segment_ubox0, const uint32 root_bus_ubox0);

};   // class IMC


}   // namespace pcm