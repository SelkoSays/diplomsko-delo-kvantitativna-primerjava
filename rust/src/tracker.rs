use std::{fs, io::Write};

#[derive(Debug, Clone)]
pub struct SystemMemoryStats {
    pub vm_peak: usize, // Peak virtual memory size (KB)
    pub vm_size: usize, // Virtual memory size (KB)
    pub vm_rss: usize,  // Resident set size (KB)
    pub vm_hwm: usize,  // Peak resident set size (KB)
    pub vm_data: usize, // Size of data segment (KB)
    pub vm_stk: usize,  // Size of stack (KB)
    pub vm_exe: usize,  // Size of text segment (KB)
    pub vm_lib: usize,  // Shared library code size (KB)
}

impl SystemMemoryStats {
    pub fn new() -> Result<Self, std::io::Error> {
        Self::read_from_proc()
    }

    fn read_from_proc() -> Result<Self, std::io::Error> {
        let status_content = fs::read_to_string("/proc/self/status")?;
        let mut stats = SystemMemoryStats {
            vm_peak: 0,
            vm_size: 0,
            vm_rss: 0,
            vm_hwm: 0,
            vm_data: 0,
            vm_stk: 0,
            vm_exe: 0,
            vm_lib: 0,
        };

        for line in status_content.lines() {
            if let Some((key, value)) = line.split_once(':') {
                let key = key.trim();
                let value_kb = Self::parse_kb_value(value);

                match key {
                    "VmPeak" => stats.vm_peak = value_kb,
                    "VmSize" => stats.vm_size = value_kb,
                    "VmRSS" => stats.vm_rss = value_kb,
                    "VmHWM" => stats.vm_hwm = value_kb,
                    "VmData" => stats.vm_data = value_kb,
                    "VmStk" => stats.vm_stk = value_kb,
                    "VmExe" => stats.vm_exe = value_kb,
                    "VmLib" => stats.vm_lib = value_kb,
                    _ => {}
                }
            }
        }

        Ok(stats)
    }

    fn parse_kb_value(value_str: &str) -> usize {
        value_str
            .trim()
            .split_whitespace()
            .next()
            .and_then(|v| v.parse().ok())
            .unwrap_or(0)
    }

    // Convert KB to bytes for consistency
    pub fn vm_rss_bytes(&self) -> usize {
        self.vm_rss * 1024
    }
    pub fn vm_size_bytes(&self) -> usize {
        self.vm_size * 1024
    }
    pub fn vm_peak_bytes(&self) -> usize {
        self.vm_peak * 1024
    }
    pub fn vm_hwm_bytes(&self) -> usize {
        self.vm_hwm * 1024
    }
}

impl std::fmt::Display for SystemMemoryStats {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        writeln!(f, "System Memory Stats (Linux /proc/self/status):")?;
        writeln!(
            f,
            "  Peak Virtual Memory (VmPeak):  {} KB ({} bytes)",
            self.vm_peak,
            self.vm_peak_bytes()
        )?;
        writeln!(
            f,
            "  Virtual Memory Size (VmSize):  {} KB ({} bytes)",
            self.vm_size,
            self.vm_size_bytes()
        )?;
        writeln!(
            f,
            "  Resident Set Size (VmRSS):     {} KB ({} bytes)",
            self.vm_rss,
            self.vm_rss_bytes()
        )?;
        writeln!(
            f,
            "  Peak Resident Size (VmHWM):    {} KB ({} bytes)",
            self.vm_hwm,
            self.vm_hwm_bytes()
        )?;
        writeln!(f, "  Data Segment (VmData):         {} KB", self.vm_data)?;
        writeln!(f, "  Stack Size (VmStk):            {} KB", self.vm_stk)?;
        writeln!(f, "  Text Segment (VmExe):          {} KB", self.vm_exe)?;
        writeln!(f, "  Shared Libraries (VmLib):      {} KB", self.vm_lib)?;
        Ok(())
    }
}

#[derive(Debug)]
pub struct Tracker {
    pub peak_size_allocated: usize,
    pub total_size_allocated: usize,
    pub total_number_of_allocations: usize,
    pub current_size_allocated: usize,
    pub current_number_of_allocations: usize,
    pub freed_allocation_size: usize,

    pub system_stats: SystemMemoryStats,
}

impl Tracker {
    pub const fn new() -> Self {
        Self {
            peak_size_allocated: 0,
            total_size_allocated: 0,
            total_number_of_allocations: 0,
            current_size_allocated: 0,
            current_number_of_allocations: 0,
            freed_allocation_size: 0,
            system_stats: SystemMemoryStats {
                vm_peak: 0,
                vm_size: 0,
                vm_rss: 0,
                vm_hwm: 0,
                vm_data: 0,
                vm_stk: 0,
                vm_exe: 0,
                vm_lib: 0,
            },
        }
    }

    pub fn init(&mut self) {
        self.peak_size_allocated = 0;
        self.total_size_allocated = 0;
        self.total_number_of_allocations = 0;
        self.current_size_allocated = 0;
        self.current_number_of_allocations = 0;
        self.freed_allocation_size = 0;

        self.update_system_stats();
    }

    fn update_system_stats(&mut self) {
        if let Ok(stats) = SystemMemoryStats::new() {
            self.system_stats = stats;
        }
    }

    pub fn add_alloc(&mut self, size: usize) {
        self.current_number_of_allocations += 1;
        self.total_number_of_allocations += 1;
        self.current_size_allocated += size;
        self.total_size_allocated += size;

        if self.current_size_allocated > self.peak_size_allocated {
            self.peak_size_allocated = self.current_size_allocated;
        }

        self.update_system_stats();
    }

    pub fn remove_alloc(&mut self, size: usize) {
        self.current_number_of_allocations = self.current_number_of_allocations.saturating_sub(1);
        self.current_size_allocated = self.current_size_allocated.saturating_sub(size);
        self.freed_allocation_size += size;

        self.update_system_stats();
    }

    pub fn write(&mut self, file: Option<&mut std::fs::File>) {
        // self.update_system_stats();
        if let Some(f) = file {
            writeln!(
                f,
                "{},{},{},{},{},{},{},{},{},{},{},{},{},{}",
                self.peak_size_allocated,
                self.total_size_allocated,
                self.total_number_of_allocations,
                self.current_size_allocated,
                self.current_number_of_allocations,
                self.freed_allocation_size,
                self.system_stats.vm_peak_bytes(),
                self.system_stats.vm_size_bytes(),
                self.system_stats.vm_rss_bytes(),
                self.system_stats.vm_hwm_bytes(),
                self.system_stats.vm_data * 1024,
                self.system_stats.vm_stk * 1024,
                self.system_stats.vm_exe * 1024,
                self.system_stats.vm_lib * 1024,
            )
            .unwrap_or_else(|_| {
                eprintln!("Could not write tracker to file");
            });
        }
    }

    pub fn write_header(file: Option<&mut std::fs::File>) {
        if let Some(f) = file {
            writeln!(
                f,
                "peak_size_allocated,total_size_allocated,total_number_of_allocations,\
                current_size_allocated,current_number_of_allocations,freed_allocation_size,\
                vm_peak_bytes,vm_size_bytes,vm_rss_bytes,vm_hwm_bytes,vm_data_bytes,\
                vm_stk_bytes,vm_exe_bytes,vm_lib_bytes"
            )
            .unwrap_or_else(|_| {
                eprintln!("Could not write header to file");
            });
        }
    }

    pub fn get_stats(&self) -> TrackerStats {
        TrackerStats {
            peak_size_allocated: self.peak_size_allocated,
            total_size_allocated: self.total_size_allocated,
            total_number_of_allocations: self.total_number_of_allocations,
            current_size_allocated: self.current_size_allocated,
            current_number_of_allocations: self.current_number_of_allocations,
            freed_allocation_size: self.freed_allocation_size,
            system_stats: self.system_stats.clone(),
        }
    }

    pub fn print_debug(&self) {
        eprintln!("============================================");
        eprintln!("CUSTOM ALLOCATOR TRACKING:");
        eprintln!(
            "Peak size allocated:            {}",
            self.peak_size_allocated
        );
        eprintln!(
            "Total size allocated:           {}",
            self.total_size_allocated
        );
        eprintln!(
            "Total allocations:              {}",
            self.total_number_of_allocations
        );
        eprintln!(
            "Current size allocated:         {}",
            self.current_size_allocated
        );
        eprintln!(
            "Current allocations:            {}",
            self.current_number_of_allocations
        );
        eprintln!(
            "Freed allocation size:          {}",
            self.freed_allocation_size
        );
        eprintln!("--------------------------------------------");
        eprintln!("LINUX SYSTEM MEMORY (/proc/self/status):");
        eprintln!(
            "Peak Virtual Memory:            {} bytes",
            self.system_stats.vm_peak_bytes()
        );
        eprintln!(
            "Virtual Memory Size:            {} bytes",
            self.system_stats.vm_size_bytes()
        );
        eprintln!(
            "Resident Set Size:              {} bytes",
            self.system_stats.vm_rss_bytes()
        );
        eprintln!(
            "Peak Resident Size:             {} bytes",
            self.system_stats.vm_hwm_bytes()
        );
        eprintln!(
            "Data Segment:                   {} bytes",
            self.system_stats.vm_data * 1024
        );
        eprintln!(
            "Stack Size:                     {} bytes",
            self.system_stats.vm_stk * 1024
        );
        eprintln!("============================================");
    }

    pub fn memory_efficiency(&self) -> f64 {
        if self.system_stats.vm_rss == 0 {
            return 0.0;
        }
        (self.current_size_allocated as f64) / (self.system_stats.vm_rss_bytes() as f64)
    }

    pub fn memory_overhead_bytes(&self) -> usize {
        self.system_stats
            .vm_rss_bytes()
            .saturating_sub(self.current_size_allocated)
    }
}

#[derive(Debug, Clone)]
pub struct TrackerStats {
    pub peak_size_allocated: usize,
    pub total_size_allocated: usize,
    pub total_number_of_allocations: usize,
    pub current_size_allocated: usize,
    pub current_number_of_allocations: usize,
    pub freed_allocation_size: usize,
    pub system_stats: SystemMemoryStats,
}

impl std::fmt::Display for TrackerStats {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        writeln!(f, "============================================")?;
        writeln!(f, "ALLOCATOR TRACKING:")?;
        writeln!(
            f,
            "Peak size allocated:            {}",
            self.peak_size_allocated
        )?;
        writeln!(
            f,
            "Total size allocated:           {}",
            self.total_size_allocated
        )?;
        writeln!(
            f,
            "Total allocations:              {}",
            self.total_number_of_allocations
        )?;
        writeln!(
            f,
            "Current size allocated:         {}",
            self.current_size_allocated
        )?;
        writeln!(
            f,
            "Current allocations:            {}",
            self.current_number_of_allocations
        )?;
        writeln!(
            f,
            "Freed allocation size:          {}",
            self.freed_allocation_size
        )?;
        writeln!(f, "--------------------------------------------")?;
        write!(f, "{}", self.system_stats)?;
        writeln!(f, "--------------------------------------------")?;
        writeln!(f, "ANALYSIS:")?;
        let efficiency = if self.system_stats.vm_rss > 0 {
            (self.current_size_allocated as f64) / (self.system_stats.vm_rss_bytes() as f64) * 100.0
        } else {
            0.0
        };
        writeln!(f, "Memory efficiency:              {:.2}%", efficiency)?;
        writeln!(
            f,
            "Memory overhead:                {} bytes",
            self.system_stats
                .vm_rss_bytes()
                .saturating_sub(self.current_size_allocated)
        )?;
        writeln!(f, "============================================")?;
        Ok(())
    }
}

// Global tracker instance
pub static mut GLOBAL_TRACKER: Tracker = Tracker::new();
