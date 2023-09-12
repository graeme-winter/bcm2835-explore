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

// open and close are no-ops - we don't need to do anything in particular
static int clk_open(struct inode *i, struct file *f) {
  printk("gpclk open\n");
  return 0;
}

static int clk_release(struct inode *i, struct file *f) {
  printk("gpclk close\n");
  return 0;
}

static ssize_t clk_read(struct file *f, char __user *buf, size_t len,
                        loff_t *off) {
  char msg[80];
  int div, n, error;

  printk("gpclk read\n");

  if (*off) {
    *off = 0;
    return 0;
  }

  div = ((*(unsigned int *)(0x20101074)) >> 12) & 0xfff;
  sprintf(msg, "%d", div);
  n = strlen(msg);

  // if n > len error - this is very unlikely

  error = copy_to_user(buf, msg, n);

  // if error; we have an error

  *off += n;
  return n;
}

static ssize_t clk_write(struct file *f, const char __user *buf, size_t len,
                         loff_t *off) {
  char msg[80];
  int error = 0;

  printk("gpclk write\n");

  if (len > 80) {
    len = 79;
  }

  error = copy_from_user(msg, buf, len);

  msg[len] = 0;

  // FIXME do something with this message once I know the device address

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
