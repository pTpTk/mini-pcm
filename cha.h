#pragma once

#include "global.h"
#include "pci.h"
#include "pmu.h"
#include "utils.h"

namespace pcm
{

class CHA{
  public:
    CHA();
    virtual ~CHA();
    bool program(std::string configStr);
    void run();
    void getCounter(std::vector<std::vector<uint64>>& M, int counterId);

  private:
    uint32 getMaxNumOfCBoxes() const;
    int32 getNumCores() const;
    void initFreeze();

    int eventCount;
    std::vector<std::vector<UncorePMU>> cboPMUs;
};

inline auto CX_MSR_PMON_BOX_FILTER(uint32 Cbo)
    { return SPR_CHA0_MSR_PMON_BOX_FILTER + SPR_CHA_MSR_STEP * Cbo; }
inline auto CX_MSR_PMON_BOX_CTL(uint32 Cbo)
    { return SPR_CHA0_MSR_PMON_BOX_CTRL + SPR_CHA_MSR_STEP * Cbo; }
inline auto CX_MSR_PMON_CTLY(uint32 Cbo, uint32 Ctl)
    { return SPR_CHA0_MSR_PMON_CTL0 + SPR_CHA_MSR_STEP * Cbo + Ctl; }
inline auto CX_MSR_PMON_CTRY(uint32 Cbo, uint32 Ctr)
    { return SPR_CHA0_MSR_PMON_CTR0 + SPR_CHA_MSR_STEP * Cbo + Ctr; }

inline UncorePMU makeCHAPMU(std::shared_ptr<SafeMsrHandle>& handle, uint32 cbo);

}   // namespace pcm