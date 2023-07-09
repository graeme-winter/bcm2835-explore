// registers from §6.1 of data sheet
volatile unsigned int *GPIO = (unsigned int *)0x20200000;
#define GPFSEL0 (0x0 / 4)
#define GPFSEL4 (0x10 / 4)
#define GPSET1 (0x20 / 4)
#define GPCLR1 (0x2C / 4)

volatile unsigned int *CM_GP = (unsigned int *)0x20101000;
#define CM_GP0CTL (0x70 / 4)
#define CM_GP0DIV (0x74 / 4)
#define GP_PASS (0x5a << 24)

// GPIO4 on bank 0
#define CLK (4)

// GPIO47 on bank #1
#define LED (47 - 32)

extern int nop(int);

int main(void) {
  unsigned int gpio_reg;

  // clock
  gpio_reg = GPIO[GPFSEL0];
  gpio_reg &= ~(7 << 12);
  gpio_reg |= 4 << 12;
  GPIO[GPFSEL0] = gpio_reg;

  CM_GP[CM_GP0CTL] = 0;
  CM_GP[CM_GP0DIV] = GP_PASS | (1920 << 12);
  CM_GP[CM_GP0CTL] = GP_PASS | 0x10 | 0x1;

  // blinkenlight
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
