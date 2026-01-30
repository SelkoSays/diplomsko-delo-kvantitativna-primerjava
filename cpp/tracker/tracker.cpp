#include "tracker.hpp"

namespace tracker
{
    bool SystemMemoryStats::readFromProc() {
        std::ifstream status_file("/proc/self/status");
        if (!status_file.is_open()) {
            return false;
        }

        std::string line;
        while (std::getline(status_file, line)) {
            std::istringstream iss(line);
            std::string key;
            size_t value;
            std::string kb;

            if (iss >> key >> value >> kb) {
                if (key == "VmPeak:")
                    vm_peak = value;
                else if (key == "VmSize:")
                    vm_size = value;
                else if (key == "VmRSS:")
                    vm_rss = value;
                else if (key == "VmHWM:")
                    vm_hwm = value;
                else if (key == "VmData:")
                    vm_data = value;
                else if (key == "VmStk:")
                    vm_stk = value;
                else if (key == "VmExe:")
                    vm_exe = value;
                else if (key == "VmLib:")
                    vm_lib = value;
            }
        }

        return true;
    }
} // namespace tracker

namespace tracker
{
    Tracker& Tracker::instance() {
        static Tracker tracker = Tracker();
        return tracker;
    }

    void Tracker::init() {
        peak_size_allocated_ = 0;
        total_size_allocated_ = 0;
        total_number_of_allocations_ = 0;
        current_size_allocated_ = 0;
        current_number_of_allocations_ = 0;
        freed_allocation_size_ = 0;
        updateSystemStats();
    }

    void Tracker::addAlloc(size_t size) {
        current_number_of_allocations_++;
        total_number_of_allocations_++;
        current_size_allocated_ += size;
        total_size_allocated_ += size;
        
        if (current_size_allocated_ > peak_size_allocated_) {
            peak_size_allocated_ = current_size_allocated_;
        }
        
        updateSystemStats();
    }

    void Tracker::removeAlloc(size_t size) {
        if (current_number_of_allocations_ > 0) {
            current_number_of_allocations_--;
        }
        if (current_size_allocated_ >= size) {
            current_size_allocated_ -= size;
        }
        freed_allocation_size_ += size;
        
        updateSystemStats();
    }

    void Tracker::writeHeader(std::ostream& os) const {
        os << "peak_size_allocated,total_size_allocated,total_number_of_allocations,"
           << "current_size_allocated,current_number_of_allocations,freed_allocation_size,"
           << "vm_peak_bytes,vm_size_bytes,vm_rss_bytes,vm_hwm_bytes,vm_data_bytes,"
           << "vm_stk_bytes,vm_exe_bytes,vm_lib_bytes\n";
    }
    
    void Tracker::write(std::ostream& os) const {
        os << peak_size_allocated_ << ","
           << total_size_allocated_ << ","
           << total_number_of_allocations_ << ","
           << current_size_allocated_ << ","
           << current_number_of_allocations_ << ","
           << freed_allocation_size_ << ","
           << system_stats_.vmPeakBytes() << ","
           << system_stats_.vmSizeBytes() << ","
           << system_stats_.vmRssBytes() << ","
           << system_stats_.vmHwmBytes() << ","
           << (system_stats_.vm_data * 1024) << ","
           << (system_stats_.vm_stk * 1024) << ","
           << (system_stats_.vm_exe * 1024) << ","
           << (system_stats_.vm_lib * 1024) << "\n";
    }

    void Tracker::printDebug() const {
        std::cout << "============================================\n"
                  << "CUSTOM ALLOCATOR TRACKING:\n"
                  << "Peak size allocated:            " << peak_size_allocated_ << "\n"
                  << "Total size allocated:           " << total_size_allocated_ << "\n"
                  << "Total allocations:              " << total_number_of_allocations_ << "\n"
                  << "Current size allocated:         " << current_size_allocated_ << "\n"
                  << "Current allocations:            " << current_number_of_allocations_ << "\n"
                  << "Freed allocation size:          " << freed_allocation_size_ << "\n"
                  << "--------------------------------------------\n"
                  << "LINUX SYSTEM MEMORY (/proc/self/status):\n"
                  << "Peak Virtual Memory:            " << system_stats_.vmPeakBytes() << " bytes\n"
                  << "Virtual Memory Size:            " << system_stats_.vmSizeBytes() << " bytes\n"
                  << "Resident Set Size:              " << system_stats_.vmRssBytes() << " bytes\n"
                  << "Peak Resident Size:             " << system_stats_.vmHwmBytes() << " bytes\n"
                  << "Data Segment:                   " << (system_stats_.vm_data * 1024) << " bytes\n"
                  << "Stack Size:                     " << (system_stats_.vm_stk * 1024) << " bytes\n"
                  << "============================================\n";
        
        double efficiency = 0.0;
        if (system_stats_.vm_rss > 0) {
            efficiency = (static_cast<double>(current_size_allocated_) / 
                         static_cast<double>(system_stats_.vmRssBytes())) * 100.0;
        }
        std::cout << "Memory efficiency:              " << efficiency << "%\n";
        
        size_t overhead = system_stats_.vmRssBytes();
        if (overhead >= current_size_allocated_) {
            overhead -= current_size_allocated_;
        } else {
            overhead = 0;
        }
        std::cout << "Memory overhead:                " << overhead << " bytes\n"
                  << "============================================\n";
    }

    double Tracker::memoryEfficiency() const {
        if (system_stats_.vm_rss == 0) return 0.0;
        return static_cast<double>(current_size_allocated_) / 
               static_cast<double>(system_stats_.vmRssBytes());
    }

    size_t Tracker::memoryOverheadBytes() const {
        size_t rss_bytes = system_stats_.vmRssBytes();
        return (rss_bytes >= current_size_allocated_) ? 
               (rss_bytes - current_size_allocated_) : 0;
    }
} // namespace tracker
