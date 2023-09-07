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

#define GPCLK0_DIV 0x20101074

dev_t dev = 0;

static struct class *clk_class;
static struct cdev clk_cdev;

// module load and unload functions
static int __init clk_driver_init(void);
static void __exit clk_driver_exit(void);

// open, close, read, write
static int clk_open(struct inode *i, struct file *f);
static int clk_release(struct inode *i, struct file *f);
static ssize_t clk_read(struct file *f, char __user *buf, size_t len, loff_t *off);
static ssize_t clk_write(struct file *f, const char *buf, size_t len, loff_t *off);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = clk_read,
    .write = clk_write,
    .open = clk_open,
    .release = clk_release,
};

// open and close are no-ops - we don't need to do anything in particular
static int clk_open(struct inode *i, struct file *f) { return 0; }
static int clk_release(struct inode *i, struct file *f) { return 0; }

static ssize_t clk_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
  uint32_t clkdiv = 0;

  // copy out the register value
  clkdiv = *((uint32_t *)GPCLK0_DIV);

  // should probably handle failure to write here
  copy_to_user(buf, &clkdiv, sizeof(uint32_t));

  return sizeof(uint32_t);
}

static ssize_t clk_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
  uint8_t clkbuf[10] = {0};

  // should have error handling here
  copy_from_user(clkbuf, buf, sizeof(uint32_t));
  *((uint32_t *)GPCLK0_DIV) = *(uint32_t *)clkbuf;

  return sizeof(uint32_t);
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

  if (IS_ERR(device_create(clk_class, NULL, dev, NULL, "gpclk0"))) {
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
