#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// Function to read the PFN for a given virtual address
uint64_t read_pfn(uint64_t virtual_address) {
    // Open the /proc/self/pagemap file
    int pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
    if (pagemap_fd == -1) {
        perror("Error opening /proc/self/pagemap");
        return 0;
    }

    // Get the system page size
    size_t page_size = getpagesize();

    // Calculate the index and offset of the virtual address in the pagemap file
    size_t page_index = virtual_address / page_size;
    off_t offset = page_index * sizeof(uint64_t);

    // Read the pagemap entry at the calculated offset
    uint64_t pfn_entry;
    if (pread(pagemap_fd, &pfn_entry, sizeof(pfn_entry), offset) != sizeof(pfn_entry)) {
        perror("Error reading pfn entry from /proc/self/pagemap");
        close(pagemap_fd);
        return 0;
    }

    // Close the pagemap file
    close(pagemap_fd);

    // Extract the PFN from the pfn_entry (bits 0-54)
    uint64_t pfn = pfn_entry & ((1ULL << 55) - 1);
    return pfn;
}

int main() {
    // Open the /proc/self/maps file to find the stack memory region
    FILE *maps_file = fopen("/proc/self/maps", "r");
    if (!maps_file) {
        perror("Error opening /proc/self/maps");
        return 1;
    }

    // Variables to store the start and end addresses, and permissions of each memory region
    uint64_t start, end;
    char perms[5];

    // Read the memory regions from the /proc/self/maps file
    while (fscanf(maps_file, "%lx-%lx %s", &start, &end, perms) == 3) {
        // Check if the region has the 's' flag (stack region)
        if (perms[3] == 's') {
            // Get the system page size
            size_t page_size = getpagesize();

            // Iterate through each page in the stack region
            for (uint64_t addr = start; addr < end; addr += page_size) {
                // Call the read_pfn function to get the PFN for the current virtual address
                uint64_t pfn = read_pfn(addr);

                // Print the virtual address, PFN, and permissions
                printf("VADDR: 0x%016lx, PFN: 0x%016lx, PERMS: %s\n", addr, pfn, perms);
            }
        }

        // Ignore the rest of the line
        fscanf(maps_file, "%*[^\n]\n");
    }

    // Close the /proc/self/maps file
    fclose(maps_file);

    return 0;
}
