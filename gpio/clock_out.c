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

static volatile uint32_t reg = NULL;
