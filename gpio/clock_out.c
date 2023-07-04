#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// GPIO modes - datasheet table 6.2

#define GPIO_IN 0
#define GPIO_OUT 1
#define GPIO_ALT0 4
#define GPIO_ALT1 5
#define GPIO_ALT2 6
#define GPIO_ALT3 7
#define GPIO_ALT4 3
#define GPIO_ALT5 2

// GPIO register; GPCLK register
static volatile uint32_t *reg = NULL;
static volatile uint32_t *mem = NULL;

void setup(void) {
  // technically these are redundant since I could use /dev/mem for both
  int fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
  reg = (uint32_t *)mmap(NULL, 0xB4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);
  fd = open("/dev/mem", O_RDWR | O_SYNC);
  mem = (uint32_t *)mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                         0x20101000);
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
  uint32_t pass = 0x5a << 24;
  uint32_t div = 1920 << 12;
  mem[0x70 / 4] = 0;
  mem[0x74 / 4] = pass | div;
  mem[0x70 / 4] = pass | 0x10 | 0x1;
  return 0;
}
