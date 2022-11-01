#ifndef TYPE_H
#define TYPE_H

#include <string.h>

namespace sprimc {

typedef unsigned long long uint64;
typedef signed long long int64;
typedef unsigned int uint32;
typedef signed int int32;

struct MCFGRecord
{
    unsigned long long baseAddress;
    unsigned short PCISegmentGroupNumber;
    unsigned char startBusNumber;
    unsigned char endBusNumber;
    char reserved[4];
    MCFGRecord()
    {
           memset(this, 0, sizeof(MCFGRecord));
    }
    void print()
    {
        std::cout << "BaseAddress=" << (std::hex) << "0x" << baseAddress << " PCISegmentGroupNumber=0x" << PCISegmentGroupNumber <<
            " startBusNumber=0x" << (unsigned)startBusNumber << " endBusNumber=0x" << (unsigned)endBusNumber << "\n";
    }
};
struct MCFGHeader
{
    char signature[4];
    unsigned length;
    unsigned char revision;
    unsigned char checksum;
    char OEMID[6];
    char OEMTableID[8];
    unsigned OEMRevision;
    unsigned creatorID;
    unsigned creatorRevision;
    char reserved[8];

    unsigned nrecords() const
    {
        return (length - sizeof(MCFGHeader)) / sizeof(MCFGRecord);
    }

    void print()
    {
        std::cout << "Header: length=" << length << " nrecords=" << nrecords() << "\n";
    }
};
}

#endif
