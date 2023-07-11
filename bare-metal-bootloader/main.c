// registers from §6.1 of data sheet
volatile unsigned int *GPIO = (unsigned int *)0x20200000;
#define GPFSEL1 (0x4 / 4)
#define GPFSEL4 (0x10 / 4)
#define GPSET0 (0x1C / 4)
#define GPCLR0 (0x28 / 4)
#define GPSET1 (0x20 / 4)
#define GPCLR1 (0x2C / 4)
#define GPPUD (0x94 / 4)
#define GPPUDCLK0 (0x98 / 4)

// will use GPIO14/15 for TX / RX respectively
// on UART1 -> ALT5 GPIO function

// GPIO47 on bank #1
#define LED (47 - 32)

// §2.1 of data sheet
volatile unsigned int *AUX = (unsigned int *)0x20215000;
#define AUX_ENABLES (0x4 / 4)
#define AUX_MU_IO_REG (0x40 / 4)
#define AUX_MU_IER_REG (0x44 / 4)
#define AUX_MU_IIR_REG (0x48 / 4)
#define AUX_MU_LCR_REG (0x4C / 4)
#define AUX_MU_MCR_REG (0x50 / 4)
#define AUX_MU_LSR_REG (0x54 / 4)
#define AUX_MU_MSR_REG (0x58 / 4)
#define AUX_MU_SCRATCH (0x5C / 4)
#define AUX_MU_CNTL_REG (0x60 / 4)
#define AUX_MU_STAT_REG (0x64 / 4)
#define AUX_MU_BAUD_REG (0x68 / 4)

int main(void) {
  unsigned int gpio_reg;

  /// set up UART1
  AUX[AUX_ENABLES] = 1;
  AUX[AUX_MU_IER_REG] = 0;
  AUX[AUX_MU_CNTL_REG] = 0;
  AUX[AUX_MU_LCR_REG] = 3;
  AUX[AUX_MU_MCR_REG] = 0;
  AUX[AUX_MU_IER_REG] = 0;
  AUX[AUX_MU_IIR_REG] = 0xC6;
  AUX[AUX_MU_BAUD_REG] = 270;

  // GPIO14, 15
  gpio_reg = GPIO[GPFSEL1];
  gpio_reg &= ~(7 << 12);
  gpio_reg |= (2 << 12);
  gpio_reg &= ~(7 << 15);
  gpio_reg |= (2 << 15);
  GPIO[GPFSEL1] = gpio_reg;

  AUX[AUX_MU_CNTL_REG] = 3;

  // GPIO47
  gpio_reg = GPIO[GPFSEL4];
  gpio_reg &= ~(7 << 21);
  gpio_reg |= 1 << 21;
  GPIO[GPFSEL4] = gpio_reg;

  while (1) {
    // read 4 bytes
    // 15 == strlen(message)
    unsigned int size = 0, sum = 0;
    unsigned char * csize = (unsigned char *) &size;
    unsigned char * csum = (unsigned char *) &sum;

    GPIO[GPSET1] = 1 << LED;
    for (int j = 0; j < 4; j++) {
      // wait for byte to be ready
      while (!(AUX[AUX_MU_LSR_REG] & (1 << 0)))
        ;
      csize[j] = AUX[AUX_MU_IO_REG] & 0xff;
    }
    GPIO[GPCLR1] = 1 << LED;
    for (int j = 0; j < size; j++) {
      // wait for byte to be ready
      while (!(AUX[AUX_MU_LSR_REG] & (1 << 0)))
        ;
      sum += AUX[AUX_MU_IO_REG] & 0xff;
    }
    // write sum back up pipe
    for (int j = 0; j < 4; j++) {
      // wait for transmit
      while (!(AUX[AUX_MU_LSR_REG] & (1 << 5)))
        ;
      AUX[AUX_MU_IO_REG] = csum[j];
    }
  }

  return 0;
}
