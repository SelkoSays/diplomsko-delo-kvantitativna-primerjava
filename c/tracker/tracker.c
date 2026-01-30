#include "tracker.h"

// Global instance
Tracker global_tracker = {0};

static char line_buf[1024];
static int read_system_memory_stats(SystemMemoryStats *stats) {
    FILE *file = fopen("/proc/self/status", "r");
    if (!file) {
        return -1;
    }

    memset(stats, 0, sizeof(SystemMemoryStats));

    while (fgets(line_buf, sizeof(line_buf), file)) {
        size_t value;

        if (sscanf(line_buf, "VmPeak: %zu kB", &value) == 1) {
            stats->vm_peak = value;
        } else if (sscanf(line_buf, "VmSize: %zu kB", &value) == 1) {
            stats->vm_size = value;
        } else if (sscanf(line_buf, "VmRSS: %zu kB", &value) == 1) {
            stats->vm_rss = value;
        } else if (sscanf(line_buf, "VmHWM: %zu kB", &value) == 1) {
            stats->vm_hwm = value;
        } else if (sscanf(line_buf, "VmData: %zu kB", &value) == 1) {
            stats->vm_data = value;
        } else if (sscanf(line_buf, "VmStk: %zu kB", &value) == 1) {
            stats->vm_stk = value;
        } else if (sscanf(line_buf, "VmExe: %zu kB", &value) == 1) {
            stats->vm_exe = value;
        } else if (sscanf(line_buf, "VmLib: %zu kB", &value) == 1) {
            stats->vm_lib = value;
        }
    }

    fclose(file);
    return 0;
}

static void update_system_stats(Tracker *tracker) {
    read_system_memory_stats(&tracker->system_stats);
}

void tracker_init(Tracker *tracker) {
    memset(tracker, 0, sizeof(Tracker));
    update_system_stats(tracker);
}

static void tracker_add_alloc(Tracker *tracker, size_t size) {
    tracker->current_number_of_allocations++;
    tracker->total_number_of_allocations++;
    tracker->current_size_allocated += size;
    tracker->total_size_allocated += size;

    if (tracker->current_size_allocated > tracker->peak_size_allocated) {
        tracker->peak_size_allocated = tracker->current_size_allocated;
    }

    update_system_stats(tracker);
}

static void tracker_remove_alloc(Tracker* tracker, size_t size) {
    if (tracker->current_number_of_allocations > 0) {
        tracker->current_number_of_allocations--;
    }
    if (tracker->current_size_allocated >= size) {
        tracker->current_size_allocated -= size;
    }
    tracker->freed_allocation_size += size;
    
    update_system_stats(tracker);
}

void tracker_write_header(FILE *file) {
    if (file) {
        fprintf(file, "peak_size_allocated,total_size_allocated,total_number_"
                      "of_allocations,"
                      "current_size_allocated,current_number_of_allocations,"
                      "freed_allocation_size,"
                      "vm_peak_bytes,vm_size_bytes,vm_rss_bytes,vm_hwm_bytes,"
                      "vm_data_bytes,"
                      "vm_stk_bytes,vm_exe_bytes,vm_lib_bytes\n");
    }
}

void tracker_write(const Tracker *tracker, FILE *file) {
    if (file) {
        fprintf(file,
                "%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu\n",
                tracker->peak_size_allocated, tracker->total_size_allocated,
                tracker->total_number_of_allocations,
                tracker->current_size_allocated,
                tracker->current_number_of_allocations,
                tracker->freed_allocation_size,
                tracker->system_stats.vm_peak * 1024, // Convert KB to bytes
                tracker->system_stats.vm_size * 1024,
                tracker->system_stats.vm_rss * 1024,
                tracker->system_stats.vm_hwm * 1024,
                tracker->system_stats.vm_data * 1024,
                tracker->system_stats.vm_stk * 1024,
                tracker->system_stats.vm_exe * 1024,
                tracker->system_stats.vm_lib * 1024);
    }
}

void tracker_print_debug(const Tracker *tracker) {
    printf("============================================\n");
    printf("CUSTOM ALLOCATOR TRACKING:\n");
    printf("Peak size allocated:            %zu\n",
           tracker->peak_size_allocated);
    printf("Total size allocated:           %zu\n",
           tracker->total_size_allocated);
    printf("Total allocations:              %zu\n",
           tracker->total_number_of_allocations);
    printf("Current size allocated:         %zu\n",
           tracker->current_size_allocated);
    printf("Current allocations:            %zu\n",
           tracker->current_number_of_allocations);
    printf("Freed allocation size:          %zu\n",
           tracker->freed_allocation_size);
    printf("--------------------------------------------\n");
    printf("LINUX SYSTEM MEMORY (/proc/self/status):\n");
    printf("Peak Virtual Memory:            %zu bytes\n",
           tracker->system_stats.vm_peak * 1024);
    printf("Virtual Memory Size:            %zu bytes\n",
           tracker->system_stats.vm_size * 1024);
    printf("Resident Set Size:              %zu bytes\n",
           tracker->system_stats.vm_rss * 1024);
    printf("Peak Resident Size:             %zu bytes\n",
           tracker->system_stats.vm_hwm * 1024);
    printf("Data Segment:                   %zu bytes\n",
           tracker->system_stats.vm_data * 1024);
    printf("Stack Size:                     %zu bytes\n",
           tracker->system_stats.vm_stk * 1024);
    printf("============================================\n");

    // Analysis
    double efficiency = 0.0;
    if (tracker->system_stats.vm_rss > 0) {
        efficiency = ((double)tracker->current_size_allocated) /
                     ((double)(tracker->system_stats.vm_rss * 1024)) * 100.0;
    }
    printf("Memory efficiency:              %.2f%%\n", efficiency);

    size_t overhead = (tracker->system_stats.vm_rss * 1024);
    if (overhead >= tracker->current_size_allocated) {
        overhead -= tracker->current_size_allocated;
    } else {
        overhead = 0;
    }
    printf("Memory overhead:                %zu bytes\n", overhead);
    printf("============================================\n");
}

void *tracker_alloc(Tracker *tracker, size_t size) {
    void *ptr = calloc(1, size);
    if (ptr != NULL) {
        tracker_add_alloc(tracker, size);
    }
    return ptr;
}

void tracker_free(Tracker *tracker, void *ptr, size_t size) {
    if (ptr) {
        tracker_remove_alloc(tracker, size);
        free(ptr);
    }
}
