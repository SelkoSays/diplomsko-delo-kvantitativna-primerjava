#ifndef TRACKER_H
#define TRACKER_H

#include "../utils/common.h"
#include "../utils/logging.h"

#define tracker_create(T, tracker_ptr) (T *)tracker_alloc((tracker_ptr), sizeof(T))
#define tracker_destroy(tracker_ptr, ptr)                                        \
    tracker_free((tracker_ptr), (ptr), sizeof(*(ptr)))

typedef struct {
    usize vm_peak;   // Peak virtual memory size (KB)
    usize vm_size;   // Virtual memory size (KB) 
    usize vm_rss;    // Resident set size (KB)
    usize vm_hwm;    // Peak resident set size (KB)
    usize vm_data;   // Size of data segment (KB)
    usize vm_stk;    // Size of stack (KB)
    usize vm_exe;    // Size of text segment (KB)
    usize vm_lib;    // Shared library code size (KB)
} SystemMemoryStats;

typedef struct {
    // Your existing tracker fields
    usize peak_size_allocated;
    usize total_size_allocated;
    usize total_number_of_allocations;
    usize current_size_allocated;
    usize current_number_of_allocations;
    usize freed_allocation_size;
    
    // System-level stats
    SystemMemoryStats system_stats;
} Tracker;

void tracker_init(Tracker* tracker);

void *tracker_alloc(Tracker* tracker, usize size);
void tracker_free(Tracker* tracker, void *ptr, usize size);

void tracker_write(const Tracker* tracker, FILE *file);
void tracker_write_header(FILE *file);


#endif // TRACKER_H
