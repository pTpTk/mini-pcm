#pragma once

#include "global.h"
#include "pci.h"
#include "pmu.h"

namespace pcm
{

class CHA{
  public:
    CHA();
    virtual ~CHA();
    bool program(std::string configStr);

    void initFreeze();
    void run();

  private:
    uint32 getMaxNumOfCBoxes() const;

    std::vector<std::vector<UncorePMU>> cboPMUs;
};

}   // namespace pcm