#ifndef TRACKER_HPP
#define TRACKER_HPP

#include "../../c/utils/common.h"
#include "../../c/utils/logging.h"

#include "../utils/common.hpp"

#include <fstream>
#include <new>
#include <sstream>
#include <type_traits>

namespace tracker {

class SystemMemoryStats {
  public:
    size_t vm_peak = 0; // Peak virtual memory size (KB)
    size_t vm_size = 0; // Virtual memory size (KB)
    size_t vm_rss = 0;  // Resident set size (KB)
    size_t vm_hwm = 0;  // Peak resident set size (KB)
    size_t vm_data = 0; // Size of data segment (KB)
    size_t vm_stk = 0;  // Size of stack (KB)
    size_t vm_exe = 0;  // Size of text segment (KB)
    size_t vm_lib = 0;  // Shared library code size (KB)

    SystemMemoryStats() = default;

    bool readFromProc();

    // Convert KB to bytes for consistency
    size_t vmRssBytes() const { return vm_rss * 1024; }
    size_t vmSizeBytes() const { return vm_size * 1024; }
    size_t vmPeakBytes() const { return vm_peak * 1024; }
    size_t vmHwmBytes() const { return vm_hwm * 1024; }

    friend std::ostream &operator<<(std::ostream &os,
                                    const SystemMemoryStats &stats) {
        os << "System Memory Stats (Linux /proc/self/status):\n"
           << "  Peak Virtual Memory (VmPeak):  " << stats.vm_peak << " KB ("
           << stats.vmPeakBytes() << " bytes)\n"
           << "  Virtual Memory Size (VmSize):  " << stats.vm_size << " KB ("
           << stats.vmSizeBytes() << " bytes)\n"
           << "  Resident Set Size (VmRSS):     " << stats.vm_rss << " KB ("
           << stats.vmRssBytes() << " bytes)\n"
           << "  Peak Resident Size (VmHWM):    " << stats.vm_hwm << " KB ("
           << stats.vmHwmBytes() << " bytes)\n"
           << "  Data Segment (VmData):         " << stats.vm_data << " KB\n"
           << "  Stack Size (VmStk):            " << stats.vm_stk << " KB\n"
           << "  Text Segment (VmExe):          " << stats.vm_exe << " KB\n"
           << "  Shared Libraries (VmLib):      " << stats.vm_lib << " KB\n";
        return os;
    }
};

class Tracker {
private:
    size_t peak_size_allocated_ = 0;
    size_t total_size_allocated_ = 0;
    size_t total_number_of_allocations_ = 0;
    size_t current_size_allocated_ = 0;
    size_t current_number_of_allocations_ = 0;
    size_t freed_allocation_size_ = 0;
    
    SystemMemoryStats system_stats_;
    
    void updateSystemStats() {
        system_stats_.readFromProc();
    }

public:
    static Tracker &instance();
    Tracker() = default;
    
    void init();
    
    void addAlloc(size_t size);
    void removeAlloc(size_t size);
    
    void writeHeader(std::ostream& os) const;
    void write(std::ostream& os) const;
    
    void printDebug() const;
    
    double memoryEfficiency() const;
    size_t memoryOverheadBytes() const;
    
    size_t peakSizeAllocated() const { return peak_size_allocated_; }
    size_t totalSizeAllocated() const { return total_size_allocated_; }
    size_t totalNumberOfAllocations() const { return total_number_of_allocations_; }
    size_t currentSizeAllocated() const { return current_size_allocated_; }
    size_t currentNumberOfAllocations() const { return current_number_of_allocations_; }
    size_t freedAllocationSize() const { return freed_allocation_size_; }
    const SystemMemoryStats& systemStats() const { return system_stats_; }
};

template <typename T> class TrackingAllocator {
  public:
    using value_type = T;
    using pointer = T *;
    using size_type = std::size_t;
    using is_always_equal = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    TrackingAllocator() = default;

    template <typename U>
    TrackingAllocator(const TrackingAllocator<U> &) noexcept {}

    pointer allocate(size_type n) {
        std::size_t bytes = n * sizeof(T);
        Tracker::instance().addAlloc(bytes);
        return static_cast<T *>(
            ::operator new(bytes)); // match with ::operator delete
    }

    void deallocate(pointer p, size_type n) noexcept {
        Tracker::instance().removeAlloc(n * sizeof(T));
        ::operator delete(p); // DO NOT use delete[] or std::free here
    }

    template <typename U>
    bool operator==(const TrackingAllocator<U> &) const noexcept {
        return true;
    }

    template <typename U>
    bool operator!=(const TrackingAllocator<U> &) const noexcept {
        return false;
    }
};

} // namespace tracker

#endif // TRACKER_H
