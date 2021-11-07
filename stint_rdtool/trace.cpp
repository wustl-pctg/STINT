#include <iostream>
#include "trace.h"
#include "shadow_mem_2.h"
#include "utility.h"
#include "shadow_stack.h"

extern int64_t numReads;
extern int64_t sizeReads;
extern int64_t numWrites;
extern int64_t sizeWrites;
extern int64_t numStrands;
extern int64_t numIntervals;
extern int64_t numIntervalsRead;
extern int64_t numIntervalsWrite;
extern int64_t intervalSize;
extern int64_t intervalSizeRead;
extern int64_t intervalSizeWrite;
extern uint64_t total_ops;
extern uint64_t total_height;
extern uint64_t total_nodes;
int maxDepth = 0;
int maxNumIntervals = 0;

#ifdef SAMPLING
extern uint64_t num_of_sp_queries;
extern uint64_t num_of_nodes_visited;
#endif

#ifdef STATS_MODE
#define updateMaxDepth() updateMaxNumDepthHelper()
#define updateNumIntervals() updateMaxNumIntervalsHelper()
#define printTreapStats() fprintf(stdout, "max numIntervals = %d\n maxDepth = %d\n", maxNumIntervals,maxDepth)
#define print_mem_access() fprintf(stdout, "reads: %zu with total size %zu\n writes: %zu with total size %zu\n strands: %zu\n intervals read: %zu intervals write: %zu\n interval size read: %zu interval size write: %zu\n total ops: %zu total height: %zu total nodes: %zu\n", numReads,sizeReads,numWrites,sizeWrites,numStrands,numIntervalsRead,numIntervalsWrite,intervalSizeRead,intervalSizeWrite, total_ops, total_height, total_nodes);
#else
#define updateMaxDepth()
#define updateNumIntervals()
#define printTreapStats()
#define print_mem_access()
#endif

extern ShadowMem2 shadow_mem_read;
extern ShadowMem2 shadow_mem_write;

void Trace::end_of_computation() {
	std::cout << "###stats:###" << std::endl;
	print_mem_access();
	//printTreapStats();
#ifdef SAMPLING
	printf("measured ops: %zu total nodes visited: %zu total queries: %zu\n", total_ops, num_of_nodes_visited, num_of_sp_queries);
#endif
}

void Trace::end_of_strand() {
	// to address
	IntervalChunk *read_intervals = shadow_mem_read.to_address();
#ifdef STATS_MODE
	numIntervalsRead += numIntervals;
	numIntervals = 0;
	intervalSizeRead += intervalSize;
	intervalSize = 0;
#endif

	IntervalChunk *write_intervals = shadow_mem_write.to_address();
#ifdef STATS_MODE
	numIntervalsWrite += numIntervals;
	numIntervals = 0;
	intervalSizeWrite += intervalSize;
	intervalSize = 0;
#endif
	MemAccess_t *accessor = new MemAccess_t(get_english(),get_hebrew(),shadow_mem_read.inst_addr);
	// AnnC: accessor should be the same if I do with shadow_mem_write.inst_addr?

	// check race
	// AnnC: should I pass in an arry of intervals here or just a pointer to the array?
	insertAndCheckWriteTreap(write_intervals,read_intervals,accessor);
	insertAndCheckReadTreap(write_intervals,read_intervals,accessor);
	/*
	while (read_intervals != NULL) {
		IntervalChunk* old = read_intervals;
		read_intervals = read_intervals->next_chunk;
		delete old;
	}

	while (write_intervals != NULL) {
		IntervalChunk* old = write_intervals;
		write_intervals = write_intervals->next_chunk;
		delete old;
	}*/

	//updateMaxDepth();
	//updateNumIntervals();

	// clear local shadow mem
	// Yifan: I think we should never
	// call this here
	//shadow_mem_read.clear_table();
	//shadow_mem_write.clear_table();
}

Trace global_trace;
