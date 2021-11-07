#include "shadow_mem_2_helper.h"
#include "shadow_mem_2.h"

int log_two(uint64_t n) {
    int count = 0;
    while (n != 0) {
        count++;
        n = n>>1;
    }
    return count-1;
}

Interval createInterval(uint64_t index, uint64_t numPos, uint64_t start, uint64_t end) {
    uint64_t startAddress = ((index << LOG_L2_TBL_SIZE) +
                             (numPos << INTSHIFT) + start) << LOG_BYTES_PER_KEY;
    uint64_t endAddress = ADDR_OFFSET + (((index << LOG_L2_TBL_SIZE) + 
                             (numPos << INTSHIFT) + end) << LOG_BYTES_PER_KEY);
    return {startAddress, endAddress};
}
