#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <string.h>

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

int main(int argc, char *argv[]) {
  char path[64];
  int fd;
  uint64_t va, pfn;

  snprintf(path, sizeof(path), "/proc/%d/pagemap", getpid());
  fd = open(path, O_RDONLY);
  if (fd == -1) {
    perror("open");
    return 1;
  }

  printf("Virtual address\tPFN\n");
  for (va = (uint64_t)&va & ~(PAGE_SIZE - 1); ; va += PAGE_SIZE) {
    lseek(fd, (va / PAGE_SIZE) * sizeof(uint64_t), SEEK_SET);
    read(fd, &pfn, sizeof(uint64_t));
    pfn &= 0x7fffffffffffff;
    if (pfn == 0)
      break;
    printf("0x%lx\t\t%lu\n", va, pfn);
  }

  close(fd);
  return 0;
}
