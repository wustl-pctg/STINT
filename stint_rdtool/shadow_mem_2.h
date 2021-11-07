#include <cstdio>
#include "debug_util.h"
#include <vector>
#include <algorithm> 
#include <time.h>
#include "treap.h"
#include "shadow_mem_2_helper.h"

#ifndef __SHADOWMEM_2_H__
#define __SHADOWMEM_2_H__

/* 64 bit address, last 48 bits are used. tsan_read/write read/write multiples of 4. Table contains 2^24/64 uint64_t.*/
#define KEY_SIZE 2
#define TBL_SIZE 24
#define ENTRY_SIZE (48 - KEY_SIZE - TBL_SIZE)
#define INT_SIZE 64
#define INTSHIFT 6
#define INTBITS 63
#define ADDR_MULTIPLE 4
#define ADDR_OFFSET 3
#define NUM_INT 1 << TBL_SIZE >> INTSHIFT

#define ADDR_TO_KEY_2(addr) ((uint64_t) ((uint64_t)(addr) >> KEY_SIZE))

// #define LOG__KEY 4
// #define ADDR_TO_KEY(addr) ((uint64_t) ((uint64_t)addr >> LOG__KEY))

// this is the name mapping in the new file ...
#define LOG_BYTES_PER_KEY KEY_SIZE
#define LOG_L1_TBL_SIZE ENTRY_SIZE
#define LOG_L2_TBL_SIZE TBL_SIZE

extern int64_t numIntervals;
extern int64_t intervalSize;
#ifdef STATS_MODE
#define increase_num_intervals() numIntervals++
#define increase_interval_size(size) intervalSize += size
#else
#define increase_num_intervals()
#define increase_interval_size(size)
#endif

using namespace std;

class ShadowMem2 {
private:

  typedef struct num_pair_t {
    uint64_t table_index;
    uint64_t integer_index;

    bool operator<(struct num_pair_t other) const {
      if (table_index == other.table_index) {
        return integer_index < other.integer_index;
      } else {
        return table_index < other.table_index;
      }
    }
  } num_pair_t;

  vector<num_pair_t> touched_indices;

  struct shadow_tbl { 
    uint64_t shadow_entries[NUM_INT]; 
  };

  struct shadow_tbl **shadow_dir;

public:
  uint64_t inst_addr;

  ShadowMem2() {
    shadow_dir =
      new struct shadow_tbl *[1 << ENTRY_SIZE]();
    //srand(time(0));
    inst_addr = -1;
  }
  
/* deprecated
  void insert(uint64_t addr, size_t mem_size) {
    for (int i = 0; i < mem_size; i += ADDR_MULTIPLE) {
    uint64_t addr2 = ADDR_TO_KEY_2(addr + i);
    uint64_t index = addr2 >> TBL_SIZE;
    uint64_t offset = addr2 & (0xFFFFFF);
    shadow_tbl *tbl = shadow_dir[index];
    if (tbl == NULL) {
      // Allocate
      shadow_dir[index] = new struct shadow_tbl();
      tbl = shadow_dir[index];
      indexes.push_back(index);
    }
    // Mark the bit to be 1.
    uint64_t intPos = offset >> INTSHIFT;
    uint64_t markPos = offset % INT_SIZE;
    shadow_dir[index]->shadow_entries[intPos] |= (uint64_t)1 << markPos;
    }
  }
*/

  void bulk_insert(uint64_t addr, size_t mem_size) {
    assert(mem_size%4 == 0);
    mem_size  /= 4;
    uint64_t addr2 = ADDR_TO_KEY_2(addr);
    uint64_t index = addr2 >> TBL_SIZE;
    uint64_t offset = addr2 & (0xFFFFFF);
    uint64_t intPos = offset >> INTSHIFT;
    uint64_t markPos = offset % INT_SIZE;

    shadow_tbl *tbl = shadow_dir[index];
    if (tbl == NULL) {
      tbl = new struct shadow_tbl();
      shadow_dir[index] = tbl;
      // indexes.push_back(index);
    }

    uint64_t entry = tbl->shadow_entries[intPos];
    if (entry == 0) {
      // this is the first time that we touch this entry
      num_pair_t curr_index = {index,intPos};
      touched_indices.push_back(curr_index);
    }

    uint64_t mask = (~(uint64_t)0) << markPos;

    // When the address goes from low to high, we mark the bits from
    // left to right (most sig to least sig)
    // handle cases like 001111000
    // ANGE: should mem_size be mem_size / 4 everywhere?
    int shift = INT_SIZE - markPos - mem_size;
    if (shift > 0) {
        mask = (mask << shift) >> shift;
        tbl->shadow_entries[intPos] |= mask;
        return;
    }

    // handle the first integer
    tbl->shadow_entries[intPos] |= mask;

    // ANGE: I don't think the size2 calculation is correct
    // should have 4*(INT_SIZE-markPOS)
    uint64_t size2 = mem_size - (INT_SIZE-markPos);
    int numIntegersInBetween = size2 >> INTSHIFT;
    uint64_t markPosLastInt = size2 % INT_SIZE;

    // handle the integers in between
    // AnnC: would this be necessary?
    uint64_t position = intPos;
    for (int i=0; i<numIntegersInBetween; i++) {
        position++;
        if (position >= NUM_INT) {
            position = position % NUM_INT;
            index++;
            // assert(index < (1 << ENTRY_SIZE))
            tbl = shadow_dir[index];
            if (tbl == NULL) {
              tbl = new struct shadow_tbl();
              shadow_dir[index] = tbl;
              // indexes.push_back(index);
            }
        }

        entry = tbl->shadow_entries[position];
        if (entry == 0) {
          num_pair_t curr_index = {index,position};
          touched_indices.push_back(curr_index);
        }

        tbl->shadow_entries[position] = (~(uint64_t)0);
    }
        
    // handle the last integer
    uint64_t intPosLastInt = position + 1;
    if (intPosLastInt >= NUM_INT) {
        intPosLastInt = intPosLastInt % NUM_INT;
        index++;
        // assert(index < (1 << ENTRY_SIZE))
        tbl = shadow_dir[index];
        if (tbl == NULL) {
          tbl = new struct shadow_tbl();
          shadow_dir[index] = tbl;
          // indexes.push_back(index);
        }
    }

    entry = tbl->shadow_entries[intPosLastInt];
    if (entry == 0) {
      num_pair_t curr_index = {index,intPosLastInt};
      touched_indices.push_back(curr_index);
    }

    mask = ~( (~(uint64_t)0) << markPosLastInt );
    tbl->shadow_entries[intPosLastInt] |= mask;
  }

  void compute(uint64_t startKey, uint64_t endKey, shadow_tbl *tbl, uint64_t index) {
    uint64_t startOffset = startKey & (0xFFFFFF);
    uint64_t startIntPos = startOffset >> INTSHIFT;
    uint64_t startMarkPos = startOffset % INT_SIZE;
    uint64_t endOffset = endKey & (0xFFFFFF);
    uint64_t endIntPos = endOffset >> INTSHIFT;
    uint64_t endMarkPos = endOffset % INT_SIZE;
    endMarkPos = INT_SIZE - 1 - endMarkPos;
    assert(startIntPos == endIntPos);

    uint64_t entry = tbl->shadow_entries[startIntPos];
    if (entry == 0) {
      num_pair_t curr_index = {index,startIntPos};
      touched_indices.push_back(curr_index);
    }
    
    uint64_t mask = (~(uint64_t)0) << (startMarkPos + endMarkPos);
    mask = mask >> endMarkPos;
    tbl->shadow_entries[startIntPos] |= mask;
  }

  void simple_bulk_insert(uint64_t start, size_t mem_size) {
    assert(mem_size%4 == 0);
    mem_size /= 4;
    uint64_t startKey = ADDR_TO_KEY_2(start);
    uint64_t startIndex = startKey >> TBL_SIZE;
    uint64_t endKey = startKey + mem_size - 1;
    uint64_t endIndex = endKey >> TBL_SIZE;

    uint64_t finalEndKey = endKey;
    uint64_t startMarkPos = (startKey & (0xFFFFFF)) % INT_SIZE;
    if (mem_size+startMarkPos > INT_SIZE) {
      endKey = startKey + (INT_SIZE - startMarkPos - 1);
    }

    for (uint64_t i=startIndex; i <= endIndex; i++) {
      shadow_tbl *tbl = shadow_dir[i];
      if (tbl == NULL) {
        tbl = new struct shadow_tbl();
        shadow_dir[i] = tbl;
        // indexes.push_back(i);
      }

      uint64_t nextStartKey = (i+1) << TBL_SIZE;
      while (startKey < nextStartKey) {
	compute(startKey, endKey, tbl, i);
        startKey = endKey + 1;
        endKey = startKey + INT_SIZE - 1;
        if (endKey > finalEndKey) endKey = finalEndKey;
        if (startKey > finalEndKey) return;
      }
    }
  }

  // the function calling to_address knows whether the returned intervals are supposed to be
  // write_intervals or read_intervals
  IntervalChunk* to_address() {
    sort(touched_indices.begin(), touched_indices.end());
    //vector<addr_pair_t> intervals{};
    IntervalChunk* head_chunk = new IntervalChunk();
    IntervalChunk* cur_chunk = head_chunk;

    int i=0;
    // AnnC: is this truly an impossible table_index?
    uint64_t prev_table_index = (uint64_t)~0;
    num_pair_t aug_index; uint64_t table_index; uint64_t integer_index;
    shadow_tbl *tbl;
    while (i<touched_indices.size()) {
      aug_index = touched_indices[i];
      table_index = aug_index.table_index;
      integer_index = aug_index.integer_index;

      if (prev_table_index != table_index) {
        prev_table_index = table_index;
        tbl = shadow_dir[table_index];
      }

      uint64_t num = tbl->shadow_entries[integer_index];

      int a; int b; uint64_t mask; uint64_t clearMask; uint64_t x; uint64_t y;
      while (num != 0) {
        x = num;
        x = x & -x; // return a value with only the least sig bit set in x
        a = log_two(x); // return which bit is set (least sig is bit 0)
        if (a == 63) {
		mask = 0;
	} else {
		mask = (~(uint64_t)0) << (a+1);
	}
        y = (~num) & mask;
        y = y & -y;
        if (y==0) {
            b = INT_SIZE;
            clearMask = 0;
        } else {
            b = log_two(y);
            clearMask = (~(uint64_t)0) << b;
        }
        Interval addr = createInterval(table_index,integer_index,a,b-1);
		//if (intervals.size() > 0 && intervals.back().end+1 == addr.start) {
        //    intervals.back().end = addr.end;
        //} else {
        //    intervals.push_back(addr);
        //}
        increase_interval_size((addr.end + 1 - addr.start));
		if (cur_chunk->size() > 0 && cur_chunk->back().end + 1 == addr.start) {
          cur_chunk->back().end = addr.end;
        } else {
          increase_num_intervals(); 
		  if (!cur_chunk->push_interval(addr)) {
            // current chunk is full, create a new one and push again
            IntervalChunk* new_chunk = new IntervalChunk();
            cur_chunk->set_next_chunk_to(new_chunk);
            cur_chunk = new_chunk;
            cur_chunk->push_interval(addr);
          }
        }
        num = num & clearMask;
      }

      // clear the entry
      tbl->shadow_entries[integer_index] = 0;

      i++;
    }

    // AnnC: IMPORTANT NOTE: need to clear vector<table_info *> initialized_tables;
    touched_indices.clear();

    //return intervals;
	  return head_chunk;
  }

/* deprecated
  // Clear all bits from start to end.
  void clear_range(uint64_t start, uint64_t end) {
    uint64_t mem_size = end - start + 1;
    for (int i = 0; i < mem_size; i += ADDR_MULTIPLE) {
        uint64_t addr2 = ADDR_TO_KEY_2(start + i);
        uint64_t index = addr2 >> TBL_SIZE;
        if (shadow_dir[index] != NULL) {
            uint64_t offset = addr2 & (0xFFFFFF);
            // Clear this bit.
            uint64_t intPos = offset >> INTSHIFT;
            uint64_t markPos = offset % INT_SIZE;
            shadow_dir[index]->shadow_entries[intPos] &= ~((uint64_t)1 << markPos);
        }
    }
  }
*/

  void erase(uint64_t startKey, uint64_t endKey, shadow_tbl *tbl) {
    uint64_t startOffset = startKey & (0xFFFFFF);
    uint64_t startIntPos = startOffset >> INTSHIFT;
    uint64_t startMarkPos = startOffset % INT_SIZE;
    uint64_t endOffset = endKey & (0xFFFFFF);
    uint64_t endIntPos = endOffset >> INTSHIFT;
    uint64_t endMarkPos = endOffset % INT_SIZE;
    endMarkPos = INT_SIZE - 1 - endMarkPos;
    assert(startIntPos == endIntPos);
    
    uint64_t mask = (~(uint64_t)0) << (startMarkPos + endMarkPos);
    mask = mask >> endMarkPos;
    tbl->shadow_entries[startIntPos] &= ~mask;
  }

  // AnnC: if we are not freeing then, in the sense we are not really maintaining indexes here?
  void bulk_clear_range(uint64_t start, uint64_t end) {
    assert((end - start + 1) % 4 == 0);
    uint64_t startKey = ADDR_TO_KEY_2(start);
    uint64_t startIndex = startKey >> TBL_SIZE;
    uint64_t endKey = ADDR_TO_KEY_2(end);
    uint64_t endIndex = endKey >> TBL_SIZE;
    size_t mem_size = endKey - startKey + 1;

    uint64_t finalEndKey = endKey;
    uint64_t startMarkPos = (startKey & (0xFFFFFF)) % INT_SIZE;
    if (mem_size+startMarkPos > INT_SIZE) {
      endKey = startKey + (INT_SIZE - startMarkPos - 1);
    }

    // AnnC: same question here- is this truly an impossible table index?
    shadow_tbl *tbl;
    assert(startIndex <= endIndex);
    for (int i=startIndex; i<=endIndex; i++) {
        tbl = shadow_dir[i];
	if (tbl == NULL) continue;
      uint64_t nextStartKey = (i+1) << TBL_SIZE;
      while (startKey < nextStartKey) {
	erase(startKey, endKey, tbl);
        startKey = endKey + 1;
        endKey = startKey + INT_SIZE - 1;
        if (endKey > finalEndKey) endKey = finalEndKey;
        if (startKey > finalEndKey) return;
      }
    }
  }

/* deprecated
  void clear_table() {
    for (int i = 0; i < (NUM_INT); i ++) {
      if (shadow_dir[i] != NULL) free(shadow_dir[i]);
    }
    shadow_dir = new struct shadow_tbl *[1 << ENTRY_SIZE]();
    indexes = vector<uint64_t>();
    inst_addr = -1;
  } 
*/

  ~ShadowMem2() { }

};
#endif
