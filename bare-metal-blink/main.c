// registers from §6.1 of data sheet
volatile unsigned int *GPIO = (unsigned int *)0x20200000;
#define GPFSEL4 (0x10 / 4)
#define GPSET1 (0x20 / 4)
#define GPCLR1 (0x2C / 4)

// GPIO47 on bank #1
#define LED (47 - 32)

extern int nop(int);

int main(void) {
  unsigned int gpio_reg;

  gpio_reg = GPIO[GPFSEL4];
  gpio_reg &= ~(7 << 21);
  gpio_reg |= 1 << 21;
  GPIO[GPFSEL4] = gpio_reg;

  while (1) {
    GPIO[GPSET1] = 1 << LED;
    for (int j = 0; j < 0x100000; j++)
      nop(j);
    GPIO[GPCLR1] = 1 << LED;
    for (int j = 0; j < 0x100000; j++)
      nop(j);
  }

  return 0;
}
