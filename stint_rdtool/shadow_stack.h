#ifndef _SHADOW_STACK_H
#define _SHADOW_STACK_H

#include <internal/abi.h>
#include "stat_util.h"
#include "om/om.h"

extern "C" void do_tool_init();
extern "C" void do_tool_destroy();

extern "C" void do_sync();
extern "C" void do_task();
extern "C" void do_task_exit();

extern "C" void do_func_exit();

extern bool enable_checking;

extern int64_t numStrands;

om_node* get_hebrew();
om_node* get_english();

#ifdef STATS_MODE
#define increase_num_strands(amount) numStrands+=amount
#else
#define increase_num_strands(amount)
#endif

#endif
