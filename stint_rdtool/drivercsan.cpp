#include "shadow_stack.h"
#include "drivercsan.h"
#include <execinfo.h>
#include <tuple>
#include "csan.h"
#include "rd.h"
#include "shadow_mem_2.h"

// Table to store addresses.
ShadowMem2 shadow_mem_read;
ShadowMem2 shadow_mem_write;

// Read and write treap.
Node* read_treap = NULL;
Node* write_treap = NULL;

void do_read(const csi_id_t load_id,
                     uintptr_t addr, size_t mem_size) {
    DBG_TRACE(DEBUG_MEMORY, "record read %lu: %lu bytes at addr %p and rip %p.\n",
            load_id, mem_size, addr, load_pc[load_id]);
    shadow_mem_read.simple_bulk_insert((uint64_t)addr, mem_size);
    shadow_mem_read.inst_addr = (uint64_t)CALLERPC;
}

void do_write(const csi_id_t store_id,
                      uintptr_t addr, size_t mem_size) {
    DBG_TRACE(DEBUG_MEMORY, "record write %ld: %lu bytes at addr %p and rip %p.\n",
            store_id, mem_size, addr, store_pc[store_id]);
    shadow_mem_write.simple_bulk_insert((uint64_t)addr, mem_size);
    shadow_mem_write.inst_addr = (uint64_t)CALLERPC;
}

CILKSAN_API void __csan_set_MAAP_flag(uint64_t val, csi_id_t id) {
    print_debug_info("csan set MAAP flag");
}

CILKSAN_API void __csan_get_MAAP_flag(uint64_t *ptr, csi_id_t id,
				      unsigned idx) {
    print_debug_info("csan get MAAP flag");
}

CILKSAN_API void __csi_init() {
    print_debug_info("csi init");
}

CILKSAN_API
void __csan_unit_init(const char * const file_name,
                      const csan_instrumentation_counts_t counts) {
    print_debug_info("csan unit init");
}

void print_caller() {
    void *buffer[3];
    int size = backtrace(buffer, 3);
    char **symbols = backtrace_symbols(buffer, size);
    print_debug_info(symbols[2]);
    free(symbols);
}

CILKSAN_API void __csan_func_entry(const csi_id_t func_id,
                                   const void *bp, const void *sp,
                                   const func_prop_t prop) {

    print_debug_info("csan func entry");
    //print_caller();
}

CILKSAN_API void __csan_func_exit(const csi_id_t func_exit_id,
                                  const csi_id_t func_id,  
                                  const func_exit_prop_t prop) {

    print_debug_info("csan func exit");
    do_func_exit();
}

CILKSAN_API void __csan_before_loop(const csi_id_t loop_id,
                                    const int64_t trip_count, 
                                    const loop_prop_t prop) {
    print_debug_info("csan before loop");
}

CILKSAN_API void __csan_after_loop(const csi_id_t loop_id,
                                   const unsigned sync_reg,
                                   const loop_prop_t prop) {

    print_debug_info("csan after loop");
}

CILKSAN_API void __csan_before_call(const csi_id_t call_id, 
                                    const csi_id_t func_id,
                                    unsigned MAAP_count,
                                    const call_prop_t prop) {

    print_debug_info("csan before call");
}

CILKSAN_API void __csan_after_call(const csi_id_t call_id, 
                                    const csi_id_t func_id,
                                    unsigned MAAP_count,
                                    const call_prop_t prop) {

    print_debug_info("csan after call");
}

//
CILKSAN_API void __csan_detach(const csi_id_t detach_id,
                               const unsigned sync_reg) {
    print_debug_info("csan detach");
    //do_detach();
}

CILKSAN_API void __csan_task(const csi_id_t task_id, const csi_id_t detach_id, 
                             const void *bp, const void *sp,
                             const task_prop_t prop) {
    print_debug_info("csan task");
	do_task();
}

CILKSAN_API void __csan_task_exit(const csi_id_t task_exit_id, 
                                  const csi_id_t task_id,
                                  const csi_id_t detach_id,
                                  const unsigned sync_reg, 
                                  const task_exit_prop_t prop) {
    print_debug_info("csan task exit");
	do_task_exit();
}

CILKSAN_API void __csan_detach_continue(const csi_id_t detach_continue_id, 
                                        const csi_id_t detach_id) {   
    print_debug_info("csan detach continue");
    //detach_continue();
}

CILKSAN_API void __csan_sync(csi_id_t sync_id, const unsigned sync_reg) {
    print_debug_info("csan sync");
    do_sync();
}

CILKSAN_API
void __csan_load(csi_id_t load_id, const void *addr, int32_t size,
                 load_prop_t prop) {
    print_debug_info("csan load");
    if (!enable_checking) return;
	
	do_read(load_id, (uintptr_t)addr, size);
    increase_num_reads();
    increase_size_reads(size);
}

CILKSAN_API
void __csan_large_load(csi_id_t load_id, const void *addr, size_t size,
                       load_prop_t prop) {
    print_debug_info("csan large load");
    if (!enable_checking) return;
	
	do_read(load_id, (uintptr_t)addr, size);
    increase_num_reads();
    increase_size_reads(size);
}

CILKSAN_API
void __csan_store(csi_id_t store_id, const void *addr, int32_t size,
                  store_prop_t prop) { 
    print_debug_info("csan store");
    if (!enable_checking) return;
	
	do_write(store_id, (uintptr_t)addr, size);
    increase_num_writes();
    increase_size_writes(size);
}

CILKSAN_API
void __csan_large_store(csi_id_t store_id, const void *addr, size_t size, 
                        store_prop_t prop) {
    print_debug_info("csan large store");
    if (!enable_checking) return;
	
	do_write(store_id, (uintptr_t)addr, size);
    increase_num_writes();
    increase_size_writes(size);
}

CILKSAN_API
void __csi_after_alloca(const csi_id_t alloca_id, const void *addr,  
                        size_t size, const alloca_prop_t prop) { 
    print_debug_info("csi after alloca");

    assert(size % 4 == 0);
    
    // shadow_mem_2 treats intervals as end-inclusive
    shadow_mem_read.bulk_clear_range((size_t)addr, (size_t)addr + size - 1);
    shadow_mem_write.bulk_clear_range((size_t)addr, (size_t)addr + size - 1);
   
    Interval removeInterval = {(size_t)addr, (size_t)addr+size-1}; 
    read_treap = removeRange(removeInterval, read_treap);
    write_treap = removeRange(removeInterval, write_treap);
}

CILKSAN_API
void __csan_after_allocfn(const csi_id_t allocfn_id, const void *addr, 
                          size_t size, size_t num, size_t alignment,
                          const void *oldaddr, const allocfn_prop_t prop) {
    print_debug_info("csan after allocfn");
    
    assert(size % 4 == 0);

    // shadow_mem_2 treats intervals as end-inclusive
    shadow_mem_read.bulk_clear_range((size_t)addr, (size_t)addr + size - 1);
    shadow_mem_write.bulk_clear_range((size_t)addr, (size_t)addr + size - 1);
    
    Interval removeInterval = {(size_t)addr, (size_t)addr+size-1};
    read_treap = removeRange(removeInterval, read_treap);
    write_treap = removeRange(removeInterval, write_treap);
}

CILKSAN_API
void __csan_after_free(const csi_id_t free_id, const void *ptr,
                       const free_prop_t prop) {
    print_debug_info("csan after free");
}

typedef struct {
  int64_t num_entries;
  csi_id_t *id_base;
  const csan_source_loc_t *entries;
} unit_fed_table_t;

typedef struct {
  int64_t num_entries;
  const obj_source_loc_t *entries;
} unit_obj_table_t;

// Function signature for the function (generated by the CSI compiler
// pass) that updates the callsite to function ID mappings.
typedef void (*__csi_init_callsite_to_functions)();

// A call to this is inserted by the CSI compiler pass, and occurs
// before main().
CILKSAN_API
void __csirt_unit_init(const char * const name,
                       unit_fed_table_t *unit_fed_tables,
                       unit_obj_table_t *unit_obj_tables,
                       __csi_init_callsite_to_functions callsite_to_func_init) {

    print_debug_info("csirt unit init");
}

CILKSAN_API void __cilksan_enable_checking() {
    print_debug_info("enable checking");
}

CILKSAN_API void __cilksan_disable_checking() {
    print_debug_info("disable checking");
}

extern "C" void cilk_tool_init(void) {
    print_debug_info("cilk tool init");
    do_tool_init();
}

extern "C" void cilk_tool_destroy(void) {
    print_debug_info("cilk tool destroy");
    do_tool_destroy();
}

extern "C" void cilk_steal_success(__cilkrts_worker* w, __cilkrts_worker* victim,
                                   __cilkrts_stack_frame* sf) {
    print_debug_info("cilk steal success");
    //do_steal_success(w, victim, sf);    
}

extern "C" void __attribute__((noinline))
cilk_leave_stolen(__cilkrts_worker* w,
                  __cilkrts_stack_frame *saved_sf, 
                  int is_original, 
                  char* stack_base) {
    print_debug_info("cilk leave stolen");
    //do_leave_stolen();
}

extern "C" void cilk_sync_abandon(__cilkrts_stack_frame* sf) {
    print_debug_info("cilk sync abandon");
}

extern "C" void cilk_done_with_stack(__cilkrts_stack_frame *sf_at_sync,
                                     char* stack_base) {
    print_debug_info("cilk done with stack");
}

extern "C" void cilk_continue(__cilkrts_stack_frame* sf, char* new_sp) {
    print_debug_info("cilk continue");
}

extern "C" void cilk_return_to_first_frame(__cilkrts_worker* w,
                                           __cilkrts_worker* team,
                                           __cilkrts_stack_frame* sf) {
    print_debug_info("cilk return to first frame");
    //do_return_to_first_frame(w, team, sf);
}

