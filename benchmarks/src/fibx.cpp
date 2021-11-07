#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "bench.h"

#ifdef END_OF_COMPUTATION
#include "trace.h"
#endif

#ifndef TIMING_COUNT
#define TIMING_COUNT 10
#endif

#if TIMING_COUNT
#include "ktiming.h"
#endif

#define __cilkrts_accum_timing()

int fibx(int n, int even_depth) {

  int x = 0, y = 0, _tmp = 0;

  if(n < 2 && n >= 0) {
    return n;
  } else if(n < 0) {
    return 0;
  }

  if(even_depth) {
    x = cilk_spawn fibx(n - 1, !even_depth); 
  } else {
    x = cilk_spawn fibx(n - 40, !even_depth); 
  }

  if(even_depth) {
    y = fibx(n - 40, !even_depth);
  } else {
    y = fibx(n - 1, !even_depth);
  }
  cilk_sync; 

  _tmp = x+y;

  return _tmp;
}

int main(int argc, char *argv[]) 
{
  int n, res;

  if(argc != 2) {
    fprintf(stderr, "Usage: fibx [<cilk-options>] <n>\n");
    exit(1);
  }
  n = atoi(argv[1]);

#if TIMING_COUNT
  clockmark_t begin, end;
  uint64_t elapsed[TIMING_COUNT];

  for(int i=0; i < TIMING_COUNT; i++) {
    __cilkrts_set_param("local stacks", "256");
    begin= ktiming_getmark();
    res = fibx(n, 1); 
    end = ktiming_getmark();
    elapsed[i] = ktiming_diff_usec(&begin, &end);
  }
  print_runtime(elapsed, TIMING_COUNT);
#else
  res = fibx(n, 1); 
#endif
__cilkrts_accum_timing();

  printf("Result: %d\n", res);

#ifdef END_OF_COMPUTATION
  global_trace.end_of_computation();
#endif

  return 0;
}
