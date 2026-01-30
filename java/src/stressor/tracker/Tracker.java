package stressor.tracker;

import java.io.*;
import java.lang.management.*;
import java.nio.file.*;
import java.util.*;

public class Tracker {

    public static class SystemMemoryStats {
        public long vmPeak = 0; // Peak virtual memory size (KB)
        public long vmSize = 0; // Virtual memory size (KB)
        public long vmRss = 0; // Resident set size (KB)
        public long vmHwm = 0; // Peak resident set size (KB)
        public long vmData = 0; // Size of data segment (KB)
        public long vmStk = 0; // Size of stack (KB)
        public long vmExe = 0; // Size of text segment (KB)
        public long vmLib = 0; // Shared library code size (KB)

        public SystemMemoryStats() {
        }

        public boolean readFromProc() {
            try {
                List<String> lines = Files.readAllLines(Paths.get("/proc/self/status"));

                for (String line : lines) {
                    String[] parts = line.split("\\s+");
                    if (parts.length >= 2) {
                        String key = parts[0];
                        try {
                            long value = Long.parseLong(parts[1]);

                            switch (key) {
                                case "VmPeak:":
                                    vmPeak = value;
                                    break;
                                case "VmSize:":
                                    vmSize = value;
                                    break;
                                case "VmRSS:":
                                    vmRss = value;
                                    break;
                                case "VmHWM:":
                                    vmHwm = value;
                                    break;
                                case "VmData:":
                                    vmData = value;
                                    break;
                                case "VmStk:":
                                    vmStk = value;
                                    break;
                                case "VmExe:":
                                    vmExe = value;
                                    break;
                                case "VmLib:":
                                    vmLib = value;
                                    break;
                            }
                        } catch (NumberFormatException e) {
                            // Ignore lines that don't have numeric values
                        }
                    }
                }
                return true;
            } catch (IOException e) {
                System.err.println("Failed to read /proc/self/status: " + e.getMessage());
                return false;
            }
        }

        // Convert KB to bytes for consistency
        public long vmRssBytes() {
            return vmRss * 1024;
        }

        public long vmSizeBytes() {
            return vmSize * 1024;
        }

        public long vmPeakBytes() {
            return vmPeak * 1024;
        }

        public long vmHwmBytes() {
            return vmHwm * 1024;
        }

        @Override
        public String toString() {
            return String.format(
                    "System Memory Stats (Linux /proc/self/status):\n" +
                            "  Peak Virtual Memory (VmPeak):  %d KB (%d bytes)\n" +
                            "  Virtual Memory Size (VmSize):  %d KB (%d bytes)\n" +
                            "  Resident Set Size (VmRSS):     %d KB (%d bytes)\n" +
                            "  Peak Resident Size (VmHWM):    %d KB (%d bytes)\n" +
                            "  Data Segment (VmData):         %d KB\n" +
                            "  Stack Size (VmStk):            %d KB\n" +
                            "  Text Segment (VmExe):          %d KB\n" +
                            "  Shared Libraries (VmLib):      %d KB",
                    vmPeak, vmPeakBytes(), vmSize, vmSizeBytes(),
                    vmRss, vmRssBytes(), vmHwm, vmHwmBytes(),
                    vmData, vmStk, vmExe, vmLib);
        }
    }

    public static class JavaMemoryStats {
        public long heapUsed = 0;
        public long heapMax = 0;
        public long heapCommitted = 0;
        public long nonHeapUsed = 0;
        public long nonHeapMax = 0;
        public long nonHeapCommitted = 0;

        public JavaMemoryStats() {
        }

        public void update() {
            MemoryMXBean memoryBean = ManagementFactory.getMemoryMXBean();
            MemoryUsage heapUsage = memoryBean.getHeapMemoryUsage();
            MemoryUsage nonHeapUsage = memoryBean.getNonHeapMemoryUsage();

            heapUsed = heapUsage.getUsed();
            heapMax = heapUsage.getMax();
            heapCommitted = heapUsage.getCommitted();

            nonHeapUsed = nonHeapUsage.getUsed();
            nonHeapMax = nonHeapUsage.getMax();
            nonHeapCommitted = nonHeapUsage.getCommitted();
        }

        @Override
        public String toString() {
            return String.format(
                    "Java Memory Stats (MemoryMXBean):\n" +
                            "  Heap Used:                     %d bytes\n" +
                            "  Heap Max:                      %d bytes\n" +
                            "  Heap Committed:                %d bytes\n" +
                            "  Non-Heap Used:                 %d bytes\n" +
                            "  Non-Heap Max:                  %d bytes\n" +
                            "  Non-Heap Committed:            %d bytes",
                    heapUsed, heapMax, heapCommitted,
                    nonHeapUsed, nonHeapMax, nonHeapCommitted);
        }
    }

    public long peakSizeAllocated = 0;
    public long totalSizeAllocated = 0;
    public long totalNumberOfAllocations = 0;
    public long currentSizeAllocated = 0;
    public long currentNumberOfAllocations = 0;
    public long freedAllocationSize = 0;

    private SystemMemoryStats systemStats = new SystemMemoryStats();
    private JavaMemoryStats javaStats = new JavaMemoryStats();

    private static final Tracker INSTANCE = new Tracker();

    private Tracker() {
    }

    public static Tracker get() {
        return INSTANCE;
    }

    public void init() {
        peakSizeAllocated = 0;
        totalSizeAllocated = 0;
        totalNumberOfAllocations = 0;
        currentSizeAllocated = 0;
        currentNumberOfAllocations = 0;
        freedAllocationSize = 0;
        updateStats();
    }

    private void updateStats() {
        systemStats.readFromProc();
        javaStats.update();
    }

    public void addAlloc(long size) {
        currentNumberOfAllocations++;
        totalNumberOfAllocations++;
        currentSizeAllocated += size;
        totalSizeAllocated += size;

        if (currentSizeAllocated > peakSizeAllocated) {
            peakSizeAllocated = currentSizeAllocated;
        }

        updateStats();
    }

    public void removeAlloc(long size) {
        if (currentNumberOfAllocations > 0) {
            currentNumberOfAllocations--;
        }
        if (currentSizeAllocated >= size) {
            currentSizeAllocated -= size;
        }
        freedAllocationSize += size;
        updateStats();
    }

    public void writeHeader(PrintWriter writer) {
        if (writer == null) {
            return;
        }
        writer.println("peak_size_allocated,total_size_allocated,total_number_of_allocations," +
                "current_size_allocated,current_number_of_allocations,freed_allocation_size," +
                "vm_peak_bytes,vm_size_bytes,vm_rss_bytes,vm_hwm_bytes,vm_data_bytes," +
                "vm_stk_bytes,vm_exe_bytes,vm_lib_bytes"
        );
    }

    public void write(PrintWriter writer) {
        if (writer == null) {
            return;
        }
        writer.printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                peakSizeAllocated,
                totalSizeAllocated,
                totalNumberOfAllocations,
                currentSizeAllocated,
                currentNumberOfAllocations,
                freedAllocationSize,
                systemStats.vmPeakBytes(),
                systemStats.vmSizeBytes(),
                systemStats.vmRssBytes(),
                systemStats.vmHwmBytes(),
                systemStats.vmData * 1024,
                systemStats.vmStk * 1024,
                systemStats.vmExe * 1024,
                systemStats.vmLib * 1024);
        writer.println();
    }

    public void printDebug() {
        System.out.println("============================================");
        System.out.println("CUSTOM ALLOCATION TRACKING:");
        System.out.println("Peak size allocated:            " + peakSizeAllocated);
        System.out.println("Total size allocated:           " + totalSizeAllocated);
        System.out.println("Total allocations:              " + totalNumberOfAllocations);
        System.out.println("Current size allocated:         " + currentSizeAllocated);
        System.out.println("Current allocations:            " + currentNumberOfAllocations);
        System.out.println("Freed allocation size:          " + freedAllocationSize);
        System.out.println("--------------------------------------------");
        System.out.println("LINUX SYSTEM MEMORY (/proc/self/status):");
        System.out.println("Peak Virtual Memory:            " + systemStats.vmPeakBytes() + " bytes");
        System.out.println("Virtual Memory Size:            " + systemStats.vmSizeBytes() + " bytes");
        System.out.println("Resident Set Size:              " + systemStats.vmRssBytes() + " bytes");
        System.out.println("Peak Resident Size:             " + systemStats.vmHwmBytes() + " bytes");
        System.out.println("Data Segment:                   " + (systemStats.vmData * 1024) + " bytes");
        System.out.println("Stack Size:                     " + (systemStats.vmStk * 1024) + " bytes");
        System.out.println("--------------------------------------------");

        // Analysis
        double efficiency = 0.0;
        if (systemStats.vmRss > 0) {
            efficiency = ((double) currentSizeAllocated) /
                    ((double) systemStats.vmRssBytes()) * 100.0;
        }
        System.out.printf("Memory efficiency:              %.2f%%\n", efficiency);

        long overhead = systemStats.vmRssBytes() - currentSizeAllocated;
        if (overhead < 0)
            overhead = 0;
        System.out.println("Memory overhead:                " + overhead + " bytes");
        System.out.println("============================================");
    }

    public double memoryEfficiency() {
        if (systemStats.vmRss == 0)
            return 0.0;
        return ((double) currentSizeAllocated) / ((double) systemStats.vmRssBytes());
    }

    public long memoryOverheadBytes() {
        long overhead = systemStats.vmRssBytes() - currentSizeAllocated;
        return Math.max(0, overhead);
    }

    public SystemMemoryStats getSystemStats() {
        return systemStats;
    }

    public JavaMemoryStats getJavaStats() {
        return javaStats;
    }
}