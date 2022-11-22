// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2009-2018, Intel Corporation
// written by Roman Dementiev
//            Austen Ott
//            Jim Harris (FreeBSD)

#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "types.h"
#include "msr.h"
#include "utils.h"
#include <assert.h>
#include <mutex>

namespace pcm {
// here comes a Linux version

bool noMSRMode()
{
    static int noMSR = -1;
    if (noMSR < 0)
    {
        noMSR = (safe_getenv("PCM_NO_MSR") == std::string("1")) ? 1 : 0;
    }
    return 1 == noMSR;
}

MsrHandle::MsrHandle(uint32 cpu) : fd(-1), cpu_id(cpu),
                                   last_raw_value(0),
                                   extended_value(0)
{
    if (noMSRMode()) return;
    constexpr auto allowWritesPath = "/sys/module/msr/parameters/allow_writes";
    static bool writesEnabled = false;
    if (writesEnabled == false)
    {
        if (readSysFS(allowWritesPath, true).length() > 0)
        {
            writeSysFS(allowWritesPath, "on", false);
        }
        writesEnabled = true;
    }
    char * path = new char[200];
    snprintf(path, 200, "/dev/cpu/%d/msr", cpu);
    int handle = ::open(path, O_RDWR);
    if (handle < 0)
    {   // try Android msr device path
        snprintf(path, 200, "/dev/msr%d", cpu);
        handle = ::open(path, O_RDWR);
    }
    delete[] path;
    if (handle < 0)
    {
         std::cerr << "PCM Error: can't open MSR handle for core " << cpu << " (" << strerror(errno) << ")\n";
         std::cerr << "Try no-MSR mode by setting env variable PCM_NO_MSR=1\n";
         throw std::exception();
    }
    fd = handle;
}

MsrHandle::~MsrHandle()
{
    if (fd >= 0) ::close(fd);
}

int32 MsrHandle::write(uint64 msr_number, uint64 value)
{
#if 0
    static std::mutex m;
    std::lock_guard<std::mutex> g(m);
    std::cout << "DEBUG: writing MSR 0x" << std::hex << msr_number << " value 0x" << value << " on cpu " << std::dec << cpu_id << std::endl;
#endif
    if (fd < 0) return 0;
    return ::pwrite(fd, (const void *)&value, sizeof(uint64), msr_number);
}

int32 MsrHandle::read(uint64 msr_number, uint64 * value)
{
    if (fd < 0) return 0;
    return ::pread(fd, (void *)value, sizeof(uint64), msr_number);
}

uint64 MsrHandle::read48(uint64 msr_number)
{
    uint64 new_raw_value = 0;
    read(msr_number, &new_raw_value);
    if (new_raw_value < last_raw_value)
    {
        extended_value += ((1ULL << 48) - last_raw_value) + new_raw_value;
    }
    else
    {
        extended_value += (new_raw_value - last_raw_value);
    }

    last_raw_value = new_raw_value;

    // printf("last_raw_value = %llu, new_value = %llu, extended_value = %llu\n", last_raw_value, new_raw_value, extended_value);

    return extended_value;
}

} // namespace pcm
