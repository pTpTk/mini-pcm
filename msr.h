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

class MsrHandle
{
    int32 fd;
    uint32 cpu_id;
    MsrHandle();                                // forbidden
    MsrHandle(const MsrHandle &);               // forbidden
    MsrHandle & operator = (const MsrHandle &); // forbidden

public:
    MsrHandle(uint32 cpu);
    int32 read(uint64 msr_number, uint64 * value);
    int32 write(uint64 msr_number, uint64 value);
    int32 getCoreId() { return (int32)cpu_id; }
    virtual ~MsrHandle();
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

class SafeMsrHandle48
{
private:
    std::shared_ptr<SafeMsrHandle> msr;
    uint64 msr_addr;

    Mutex CounterMutex;

    uint64 extended_value;
    uint64 last_raw_value;

    SafeMsrHandle48();                                           // forbidden
    SafeMsrHandle48(SafeMsrHandle48 &);                     // forbidden
    SafeMsrHandle48 & operator = (const SafeMsrHandle48 &); // forbidden


    uint64 internal_read()
    {
        uint64 result = 0, new_raw_value = 0;
        CounterMutex.lock();

        msr->read(msr_addr, &new_raw_value);
        if (new_raw_value < last_raw_value)
        {
            extended_value += ((1ULL << 48) - last_raw_value) + new_raw_value;
        }
        else
        {
            extended_value += (new_raw_value - last_raw_value);
        }

        last_raw_value = new_raw_value;

        result = extended_value;

        CounterMutex.unlock();
        return result;
    }

public:

    SafeMsrHandle48(std::shared_ptr<SafeMsrHandle> msr_, uint64 msr_addr_) : msr(msr_), msr_addr(msr_addr_)
    {
        msr->read(msr_addr, &last_raw_value);
        extended_value = last_raw_value;
        //std::cout << "Initial Value " << extended_value << "\n";
    }

    virtual ~SafeMsrHandle48()
    {
    }

    uint64 read() // read extended value
    {
        return internal_read();
    }
    void reset()
    {
        CounterMutex.lock();
        msr->read(msr_addr, &last_raw_value);
        extended_value = last_raw_value;
        CounterMutex.unlock();
    }
};

} // namespace pcm

#endif
