#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#define GPCLK_ADDR 0x20101070

dev_t dev = 0;

static struct class *clk_class;
static struct cdev clk_cdev;

// module load and unload functions
static int __init clk_driver_init(void);
static void __exit clk_driver_exit(void);

// open, close, read, write
static int clk_open(struct inode *i, struct file *f);
static int clk_release(struct inode *i, struct file *f);
static ssize_t clk_read(struct file *f, char __user *buf, size_t len,
                        loff_t *off);
static ssize_t clk_write(struct file *f, const char *buf, size_t len,
                         loff_t *off);

static void *clk_addr;

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = clk_read,
    .write = clk_write,
    .open = clk_open,
    .release = clk_release,
};

// local memory
int clkdiv = 0;
int clksrc = 0;

// open and close are set up to map / free a region of the pi address space
static int clk_open(struct inode *i, struct file *f) {
  printk("gpclk open\n");
  clk_addr = ioremap(GPCLK_ADDR, 2 * sizeof(unsigned int));
  return 0;
}

static int clk_release(struct inode *i, struct file *f) {
  printk("gpclk close\n");
  iounmap(clk_addr);
  return 0;
}

static ssize_t clk_read(struct file *f, char __user *buf, size_t len,
                        loff_t *off) {
  char msg[80];
  int div, n, error;

  printk("gpclk read\n");

  // generate EOF for graceful reading
  if (*off) {
    *off = 0;
    return 0;
  }

  // write as a string the current value
  div = (readl(&clk_addr[1]) >> 12) & 0xfff;
  sprintf(msg, "%d", div);
  n = strlen(msg);

  error = copy_to_user(buf, msg, n);

  printk("gpclk read status: %d\n", error);

  // transmit that data has been written
  *off += n;

  return n;
}

static ssize_t clk_write(struct file *f, const char __user *buf, size_t len,
                         loff_t *off) {
  char msg[80];
  int state, pass, div, error = 0;

  pass = 0x5a << 24;

  printk("gpclk write\n");

  if (len > 80) {
    len = 79;
  }

  error = copy_from_user(msg, buf, len);

  printk("gpclk write status: %d\n", error);

  msg[len] = 0;

  sscanf(msg, "%d", &div);

  // check is < 4096 etc.

  // disable, update, reenable
  state = readl(&clk_addr[0]);
  writel(0, &clk_addr[0]);
  writel(pass | (div << 12), &clk_addr[1]);
  writel(pass | state, &clk_addr[0]);

  *off = len;
  return len;
}

static int __init clk_driver_init(void) {

  if ((alloc_chrdev_region(&dev, 0, 1, "gpclk")) < 0) {
    goto r_unreg;
  }

  cdev_init(&clk_cdev, &fops);

  if ((cdev_add(&clk_cdev, dev, 1)) < 0) {
    goto r_del;
  }

  if (IS_ERR(clk_class = class_create(THIS_MODULE, "gpclk"))) {
    goto r_class;
  }

  if (IS_ERR(device_create(clk_class, NULL, dev, NULL, "gpclk"))) {
    goto r_device;
  }

  return 0;

r_device:
  device_destroy(clk_class, dev);
r_class:
  class_destroy(clk_class);
r_del:
  cdev_del(&clk_cdev);
r_unreg:
  unregister_chrdev_region(dev, 1);

  return -1;
}

static void __exit clk_driver_exit(void) {
  device_destroy(clk_class, dev);
  class_destroy(clk_class);
  cdev_del(&clk_cdev);
  unregister_chrdev_region(dev, 1);
}

module_init(clk_driver_init);
module_exit(clk_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Graeme Winter <graeme.winter@gmail.com>");
MODULE_DESCRIPTION("Simple wrapper around BCM2835 GPCLK");
MODULE_VERSION("0.1");
