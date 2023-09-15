#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// GPIO mode - datasheet table 6.2

#define GPIO_ALT0 4

// GPIO register
static volatile uint32_t *reg = NULL;

void setup(void) {
  int fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
  reg = (uint32_t *)mmap(NULL, 0xB4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);
}

void set_mode(int gpio, int mode) {
  int off = gpio / 10;
  int shift = 3 * (gpio % 10);
  reg[off] = (reg[off] & ~(0b111 << shift)) | (mode << shift);
}

int main() {
  setup();
  set_mode(4, GPIO_ALT0);
  return 0;
}
