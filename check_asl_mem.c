
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2009-2012, Intel Corporation
// written by Roman Dementiev
//

//#include "cpucounters.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <emmintrin.h>
#include <assert.h>
#include <smmintrin.h>
#include <sys/mman.h>
#include <errno.h>
#define CACHELINE 64
typedef unsigned long long uint64;



char * allocatemem(size_t kbyte){
	char * buf;
	uint64 len = (kbyte * 1024);
	uint64 nele = (len / sizeof(uint64))+ 1 ;
        buf = (char*)mmap(0, nele*sizeof(uint64), PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (buf == MAP_FAILED){
		perror("mmap");
		exit(0);
	}
	return buf;
}


uint64 * create_circ_ptr_buf(size_t kbyte){
	 uint64 * buf;
	 
	 uint64 **tmp;
	 buf = (uint64 *)(allocatemem(kbyte));
	 tmp = (uint64 **)buf;
	 uint64 nele = (kbyte * 1024) / sizeof(uint64) ;
	 uint64 i, ncachel = nele / 8;

	 for (i=0; i < ncachel ; i++){
		 *(uint64 **)tmp = (uint64 *)(tmp + 8);
		  tmp = tmp + 8;
	 }
	*(uint64 **)tmp = buf;

	return buf;
}


void touchmem(char * buf, uint64 noofint64ele){
	uint64 *ptr, i;
	ptr = (uint64 *)buf;
	for(i=0; i < noofint64ele / 8; i++){
		ptr[i]=i;
	}

	for(i=0; i < noofint64ele / 8; i++){
		asm volatile ("clflush (%0)" :: "r"(&ptr[i]));
	}
}

char * temp_2r_1w(char *rbuf, char *wrbuf, char * endp, int dlycyc)
	{
		char *ret_ptr;
		uint64 lsz = 64;

		     asm(	
			"xor %1,%1\n\t"
			"xor %%r11,%%r11\n\t"
			"mov %7,%%r10\n\t"
		      	"movq %3,%%xmm0\n\t"   

			"LOOPWR:\n\t"
			"movdqa %%xmm0,(%4)\n\t"
			"movdqa %%xmm0,0x10(%4)\n\t"
			"movdqa %%xmm0,0x20(%4)\n\t"
			"movdqa %%xmm0,0x30(%4)\n\t"
			"add %%r10,%4\n\t"

			"inc %%r11\n\t"   // Needed to check no of iterations

			"DLYLP:\n\t"
			"inc %1\n\t"
			"cmp %6,%1\n\t"
			"jl DLYLP\n\t"

			"STOP_CHECK:\n\t"
			"cmp %3, %4\n\t"
			"jl LOOPWR\n\t"

		     "EXITRUN:\n\t"
			"mov %4, %0\n\t"
			:"=a"(ret_ptr)
			:"b"(0), "c"(rbuf), "d"(endp), "S"(wrbuf), "D"(0), "r"(dlycyc), "m"(lsz):"r10", "r11");

		return ret_ptr;
	}

int main(int argc , char * argv[]){


printf("Compile check\n");

char *startptr, *lastptr, *retptr;
uint64 bufsize = 1024*1024;
uint64 dly = 2000, i;
uint64 nele = (bufsize*1024 / sizeof(uint64))+ 1 ;
startptr = allocatemem(bufsize);  // first byte of the allocated buffer
lastptr = startptr + nele * sizeof(uint64);  // converting total element multiplied by 8 bytes each i.e size of uint64

touchmem(startptr, nele);

for (i =0; i < 20000000; i++){

retptr = temp_2r_1w(startptr, startptr, lastptr,dly);
}

printf("test completed and return ptr is %llu\n", retptr);


}
