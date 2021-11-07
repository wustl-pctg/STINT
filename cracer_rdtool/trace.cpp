#include <iostream>
#include "trace.h"

extern uint64_t numReads;
extern uint64_t sizeReads;
extern uint64_t numWrites;
extern uint64_t sizeWrites;
extern uint64_t numStrands;
extern uint64_t num_check_race_update;
#ifdef STATS_MODE
#define print_mem_access() fprintf(stdout, "reads: %zu with total size %zu\n writes: %zu with total size %zu\n strands: %zu\n", numReads,sizeReads,numWrites,sizeWrites,numStrands)
#else
#define print_mem_access()
#endif

void Trace::end_of_computation() {
	std::cout << "###stats:###" << std::endl;
	print_mem_access();
	//std::cout << "num_check_race_update: " << num_check_race_update << std::endl;
}

void Trace::end_of_strand() {

}

Trace global_trace;
