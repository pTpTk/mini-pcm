// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2009-2012, Intel Corporation
// written by Roman Dementiev
//            Austen Ott

#ifndef CPUCounters_MSR_H
#define CPUCounters_MSR_H

/*!     \file msr.h
        \brief Low level interface to access hardware model specific registers

        Implemented and tested for Linux and 64-bit Windows 7
*/

#include "types.h"
#include "mutex.h"
#include <memory>
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

bool noMSRMode();

class MsrHandle
{
    int32 fd;
    uint32 cpu_id;
    MsrHandle();                                // forbidden
    MsrHandle(const MsrHandle &);               // forbidden
    MsrHandle & operator = (const MsrHandle &); // forbidden

    uint64 last_raw_value;
    uint64 extended_value;

public:
    MsrHandle(uint32 cpu);
    int32 read(uint64 msr_number, uint64 * value);
    int32 write(uint64 msr_number, uint64 value);
    int32 getCoreId() { return (int32)cpu_id; }
    virtual ~MsrHandle();

    uint64 read48(uint64 msr_number);
};

class SafeMsrHandle
{
    std::shared_ptr<MsrHandle> pHandle;
    Mutex mutex;

    SafeMsrHandle(const SafeMsrHandle &);               // forbidden
    SafeMsrHandle & operator = (const SafeMsrHandle &); // forbidden

public:
    SafeMsrHandle() { }

    SafeMsrHandle(uint32 core_id) : pHandle(new MsrHandle(core_id))
    { }

    int32 read(uint64 msr_number, uint64 * value)
    {
        if (pHandle)
            return pHandle->read(msr_number, value);

        *value = 0;

        return (int32)sizeof(uint64);
    }

    int32 write(uint64 msr_number, uint64 value)
    {
        if (pHandle)
            return pHandle->write(msr_number, value);

        return (int32)sizeof(uint64);
    }

    uint64 read48(uint64 msr_number)
    {
        uint64 value = 0;
        if (pHandle){
            lock();
            value = pHandle->read48(msr_number);
            unlock();
        }

        return value;
    }

    int32 getCoreId()
    {
        if (pHandle)
            return pHandle->getCoreId();

        throw std::runtime_error("Core is offline");
    }

    void lock()
    {
        mutex.lock();
    }

    void unlock()
    {
        mutex.unlock();
    }

    virtual ~SafeMsrHandle()
    { }
};

} // namespace pcm

#endif
