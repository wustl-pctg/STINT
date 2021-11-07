#ifndef _DRIVERCSAN_H
#define _DRIVERCSAN_H

#include <stdio.h>
#include <internal/abi.h>

#define CALLERPC ((uintptr_t)__builtin_return_address(0))
#define CILKSAN_API extern "C" __attribute__((visibility("default")))
#define CSIRT_API __attribute__((visibility("default")))

#ifdef DEBUG_MODE
#define print_debug_info(str) fprintf(stderr, "%s: %d\n", str, \
                                      __cilkrts_get_tls_worker() != NULL ? __cilkrts_get_tls_worker()->self \
                                      : -1)
#else
#define print_debug_info(str) 
#endif

uint64_t numReads = 0;
uint64_t sizeReads = 0;
uint64_t numWrites = 0;
uint64_t sizeWrites = 0;
uint64_t numStrands = 0;
uint64_t numOps = 0;
#ifdef STATS_MODE
#define increase_num_reads() numReads ++
#define increase_size_reads(size) sizeReads += size
#define increase_num_writes() numWrites ++
#define increase_size_writes(size) sizeWrites += size
#define print_mem_access() fprintf(stdout, "reads: %d with total size %d\n writes: %d with total size %d\n strands: %d\n", numReads,sizeReads,numWrites,sizeWrites,numStrands)
#else
#define increase_num_reads()
#define increase_size_reads(size)
#define increase_num_writes()
#define increase_size_writes(size)
#define print_mem_access()
#endif

#endif
