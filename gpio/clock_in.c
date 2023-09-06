#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static volatile uint32_t *mem = NULL;

void setup(void) {
  int fd = open("/dev/mem", O_RDWR | O_SYNC);
  mem = (uint32_t *)mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                         0x20101000);
  close(fd);
}

int main() {
  setup();
  printf("%x %x\n", mem[0x70 / 4], mem[0x74 / 4]);
  return 0;
}
