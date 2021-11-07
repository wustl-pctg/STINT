#ifndef _SHADOW_STACK_H
#define _SHADOW_STACK_H

#include <internal/abi.h>
#include "stat_util.h"

extern "C" void do_tool_init();
extern "C" void do_tool_destroy();

extern "C" void do_sync();
extern "C" void do_task();
extern "C" void do_task_exit();
extern "C" void clear_shadow_memory(size_t start, size_t end);

extern "C" void do_func_exit();

extern bool enable_checking;

extern uint64_t numStrands;

#ifdef STATS_MODE
#define increase_num_strands(amount) numStrands+=amount
#else
#define increase_num_strands(amount)
#endif

#endif
