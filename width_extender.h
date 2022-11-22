// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2009-2018, Intel Corporation
// written by Roman Dementiev
//            Austen Ott

#ifndef WIDTH_EXTENDER_HEADER_
#define WIDTH_EXTENDER_HEADER_

/*!     \file width_extender.h
        \brief Provides 64-bit "virtual" counters from underlying 32-bit HW counters
*/

#include <stdlib.h>
#include "utils.h"
#include "mutex.h"
#include <memory>
#include "msr.h"
#include <thread>

namespace pcm {

class CounterWidthExtender
{
public:
    struct MsrHandleCounter
    {
        std::shared_ptr<SafeMsrHandle> msr;
        uint64 msr_addr;
        MsrHandleCounter(std::shared_ptr<SafeMsrHandle> msr_, uint64 msr_addr_) : msr(msr_), msr_addr(msr_addr_) { }
        uint64 operator () ()
        {
            uint64 value = 0;
            msr->read(msr_addr, &value);
            return value;
        }
    };

private:
    std::shared_ptr<SafeMsrHandle> msr;
    uint64 msr_addr;

    Mutex CounterMutex;

    // MsrHandleCounter * raw_counter;
    uint64 extended_value;
    uint64 last_raw_value;

    CounterWidthExtender();                                           // forbidden
    CounterWidthExtender(CounterWidthExtender &);                     // forbidden
    CounterWidthExtender & operator = (const CounterWidthExtender &); // forbidden


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

    CounterWidthExtender(std::shared_ptr<SafeMsrHandle> msr_, uint64 msr_addr_) : msr(msr_), msr_addr(msr_addr_)
    {
        msr->read(msr_addr, &last_raw_value);
        // last_raw_value = (*raw_counter)();
        extended_value = last_raw_value;
        //std::cout << "Initial Value " << extended_value << "\n";
    }

    virtual ~CounterWidthExtender()
    {
        // if (msr) delete msr;
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
        // extended_value = last_raw_value = (*raw_counter)();
        CounterMutex.unlock();
    }
};

} // namespace pcm

#endif
