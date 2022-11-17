#pragma once

#include <cstdint>
#include <vector>
#include <utility>
#include <ios>
#include <iostream>
#include <memory>
#include <map>
#include <array>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "types.h"

namespace pcm
{

constexpr auto PCM_INTEL_PCI_VENDOR_ID            = 0x8086;
constexpr auto PCM_INVALID_DEV_ADDR               = ~(uint32)0UL;
constexpr auto PCM_INVALID_FUNC_ADDR              = ~(uint32)0UL;

constexpr auto SERVER_MC_CH_PMON_REAL_BASE_ADDR   = 0x22800;
constexpr auto SERVER_MC_CH_PMON_BASE_ALIGN_DELTA = 0x00800;
constexpr auto SERVER_MC_CH_PMON_BASE_ADDR        = SERVER_MC_CH_PMON_REAL_BASE_ADDR - SERVER_MC_CH_PMON_BASE_ALIGN_DELTA;
constexpr auto SERVER_MC_CH_PMON_STEP             = 0x8000; // It seems 3 channels were assigned and currently ch0 is mapped to ch0 and ch1 is mapped to ch2
constexpr auto SERVER_MC_CH_PMON_SIZE             = 0x1000;
constexpr auto SERVER_MC_CH_PMON_BOX_CTL_OFFSET   = 0x00 + SERVER_MC_CH_PMON_BASE_ALIGN_DELTA;
constexpr auto SERVER_MC_CH_PMON_CTL0_OFFSET      = 0x40 + SERVER_MC_CH_PMON_BASE_ALIGN_DELTA;
constexpr auto SERVER_MC_CH_PMON_CTL1_OFFSET      = SERVER_MC_CH_PMON_CTL0_OFFSET + 4*1;
constexpr auto SERVER_MC_CH_PMON_CTL2_OFFSET      = SERVER_MC_CH_PMON_CTL0_OFFSET + 4*2;
constexpr auto SERVER_MC_CH_PMON_CTL3_OFFSET      = SERVER_MC_CH_PMON_CTL0_OFFSET + 4*3;
constexpr auto SERVER_MC_CH_PMON_CTR0_OFFSET      = 0x08 + SERVER_MC_CH_PMON_BASE_ALIGN_DELTA;
constexpr auto SERVER_MC_CH_PMON_CTR1_OFFSET      = SERVER_MC_CH_PMON_CTR0_OFFSET + 8*1;
constexpr auto SERVER_MC_CH_PMON_CTR2_OFFSET      = SERVER_MC_CH_PMON_CTR0_OFFSET + 8*2;
constexpr auto SERVER_MC_CH_PMON_CTR3_OFFSET      = SERVER_MC_CH_PMON_CTR0_OFFSET + 8*3;
constexpr auto SERVER_MC_CH_PMON_FIXED_CTL_OFFSET = 0x54 + SERVER_MC_CH_PMON_BASE_ALIGN_DELTA;
constexpr auto SERVER_MC_CH_PMON_FIXED_CTR_OFFSET = 0x38 + SERVER_MC_CH_PMON_BASE_ALIGN_DELTA;

constexpr auto SERVER_UBOX0_REGISTER_DEV_ADDR     = 0;
constexpr auto SERVER_UBOX0_REGISTER_FUNC_ADDR    = 1;

constexpr auto SPR_UNC_PMON_UNIT_CTL_FRZ          = 1 << 0;
constexpr auto SPR_UNC_PMON_UNIT_CTL_RST_CONTROL  = 1 << 8;
constexpr auto SPR_UNC_PMON_UNIT_CTL_RST_COUNTERS = 1 << 9;

constexpr auto MC_CH_PCI_PMON_FIXED_CTL_RST       = 1 << 19;
constexpr auto MC_CH_PCI_PMON_FIXED_CTL_EN        = 1 << 22;

constexpr auto PCM_SERVER_IMC_PMM_DATA_READS      = 0x22a0;
constexpr auto PCM_SERVER_IMC_PMM_DATA_WRITES     = 0x22a8;
constexpr auto PCM_SERVER_IMC_MMAP_SIZE           = 0x4000;
constexpr auto PCM_SERVER_IMC_DRAM_DATA_READS     = 0x2290;
constexpr auto PCM_SERVER_IMC_DRAM_DATA_WRITES    = 0x2298;

constexpr auto SPR_CHA0_MSR_PMON_BOX_CTRL   = 0x2000;
constexpr auto SPR_CHA0_MSR_PMON_CTL0       = 0x2002;
constexpr auto SPR_CHA0_MSR_PMON_CTR0       = 0x2008;
constexpr auto SPR_CHA0_MSR_PMON_BOX_FILTER = 0x200E;
constexpr auto SPR_CHA_MSR_STEP             = 0x10;

constexpr auto SPR_M2IOSF_IIO_UNIT_CTL      = 0x3000;
constexpr auto SPR_M2IOSF_IIO_CTR0          = 0x3008;
constexpr auto SPR_M2IOSF_IIO_CTL0          = 0x3002;
constexpr auto SPR_M2IOSF_REG_STEP          = 0x10;
constexpr auto SPR_M2IOSF_NUM               = 12;

constexpr auto SERVER_UNCORE_COUNTER_MAX_CONTROLLERS = 4;
constexpr auto SERVER_UNCORE_COUNTER_MAX_CHANNELS    = 8;
constexpr auto SERVER_UNCORE_COUNTER_MAX_XPI_LINKS   = 6;
constexpr auto SERVER_UNCORE_COUNTER_MAX_CBOS        = 128;
constexpr auto SERVER_UNCORE_COUNTER_MAX_IIO_STACKS  = 16;
constexpr auto SERVER_UNCORE_COUNTER_MAX_COUNTERS    = 4;
constexpr auto SERVER_UNCORE_COUNTER_MAX_SOCKETS     = 2;

} // namespace pcm