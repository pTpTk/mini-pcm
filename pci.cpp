/*
Copyright (c) 2009-2018, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// written by Roman Dementiev,
//            Pat Fay
//	      Austen Ott
//            Jim Harris (FreeBSD)

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "pci.h"
#include <sstream>
#include <iomanip>
#include <sys/mman.h>
#include <errno.h>
#include <strings.h>

namespace pcm {

// Linux implementation


int openHandle(uint32 groupnr_, uint32 bus, uint32 device, uint32 function)
{
    std::ostringstream path(std::ostringstream::out);

    path << std::hex << "/proc/bus/pci/";
    if (groupnr_)
    {
        path << std::setw(4) << std::setfill('0') << groupnr_ << ":";
    }
    path << std::setw(2) << std::setfill('0') << bus << "/" << std::setw(2) << std::setfill('0') << device << "." << function;

//    std::cout << "PciHandle: Opening "<<path.str()<<"\n";

    int handle = ::open(path.str().c_str(), O_RDWR);
    if (handle < 0)
    {
       if (errno == 24) std::cerr << "ERROR: try executing 'ulimit -n 100000' to increase the limit on the number of open files.\n";
       handle = ::open((std::string("/pcm") + path.str()).c_str(), O_RDWR);
    }
    return handle;
}

PciHandle::PciHandle(uint32 groupnr_, uint32 bus_, uint32 device_, uint32 function_) :
    fd(-1),
    bus(bus_),
    device(device_),
    function(function_)
{
    int handle = openHandle(groupnr_, bus_, device_, function_);
    if (handle < 0)
    {
        throw std::runtime_error(std::string("PCM error: can't open PciHandle ")
            + std::to_string(groupnr_) + ":" + std::to_string(bus_) + ":" + std::to_string(device_) + ":" + std::to_string(function_));
    }
    fd = handle;

    // std::cout << "DEBUG: Opened "<< path.str().c_str() << " on handle "<< fd << "\n";
}


bool PciHandle::exists(uint32 groupnr_, uint32 bus_, uint32 device_, uint32 function_)
{
    int handle = openHandle(groupnr_, bus_, device_, function_);

    if (handle < 0) return false;

    ::close(handle);

    return true;
}

int32 PciHandle::read32(uint64 offset, uint32 * value)
{
    return ::pread(fd, (void *)value, sizeof(uint32), offset);
}

int32 PciHandle::write32(uint64 offset, uint32 value)
{
    return ::pwrite(fd, (const void *)&value, sizeof(uint32), offset);
}

int32 PciHandle::read64(uint64 offset, uint64 * value)
{
    size_t res = ::pread(fd, (void *)value, sizeof(uint64), offset);
    if(res != sizeof(uint64))
    {
        std::cerr << " ERROR: pread from " << fd << " with offset 0x" << std::hex << offset << std::dec << " returned " << res << " bytes \n";
    }
    return res;
}

PciHandle::~PciHandle()
{
    if (fd >= 0) ::close(fd);
}

int PciHandle::openMcfgTable() {
    const std::vector<std::string> base_paths = {"/sys/firmware/acpi/tables/MCFG", "/sys/firmware/acpi/tables/MCFG1"};
    std::vector<std::string> paths = base_paths;
    for (auto p: base_paths)
    {
        paths.push_back(std::string("/pcm") + p);
    }
    int handle = -1;
    for (auto p: paths)
    {
        if (handle < 0)
        {
            handle = ::open(p.c_str(), O_RDONLY);
        }
    }
    if (handle < 0)
    {
        for (auto p: paths)
        {
            std::cerr << "Can't open MCFG table. Check permission of " << p << "\n";
        }
    }
    return handle;
}

// mmaped I/O version

const std::vector<MCFGRecord> & PciHandle::getMCFGRecords()
{
    static MCFGHeader mcfgHeader;
    static std::vector<MCFGRecord> mcfgRecords;

    if (mcfgRecords.size() > 0)
        return mcfgRecords; // already initialized

    int mcfg_handle = openMcfgTable();

    if (mcfg_handle < 0)
    {
        throw std::exception();
    }

    ssize_t read_bytes = ::read(mcfg_handle, (void *)&mcfgHeader, sizeof(MCFGHeader));

    if (read_bytes == 0)
    {
        ::close(mcfg_handle);
        std::cerr << "PCM Error: Cannot read MCFG-table\n";
        throw std::exception();
    }

    const unsigned segments = mcfgHeader.nrecords();
#ifdef PCM_DEBUG
    mcfgHeader.print();
    std::cout << "PCM Debug: total segments: " << segments << "\n";
#endif

    for (unsigned int i = 0; i < segments; ++i)
    {
        MCFGRecord record;
        read_bytes = ::read(mcfg_handle, (void *)&record, sizeof(MCFGRecord));
        if (read_bytes == 0)
        {
            ::close(mcfg_handle);
            std::cerr << "PCM Error: Cannot read MCFG-table (2)\n";
            throw std::exception();
        }
#ifdef PCM_DEBUG
        std::cout << "PCM Debug: segment " << std::dec << i << " ";
        record.print();
#endif
        mcfgRecords.push_back(record);
    }

    ::close(mcfg_handle);

    return mcfgRecords;
}

}
