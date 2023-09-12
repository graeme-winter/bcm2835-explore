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
#define MAXSIZE 1024
static char words[MAXSIZE] = {0};
static int size = 0;

// open and close are no-ops - we don't need to do anything in particular
static int clk_open(struct inode *i, struct file *f) {
  printk("kB open\n");
  return 0;
}

static int clk_release(struct inode *i, struct file *f) {
  printk("kB close\n");
  return 0;
}

static ssize_t clk_read(struct file *f, char __user *buf, size_t len,
                        loff_t *off) {
  int remains, error;
  char *msg;

  printk("kB read\n");

  remains = size - *off;

  if (remains <= 0) {
    *off = 0;
    return 0;
  }

  msg = words + *off;

  if (remains > len) {
    error = copy_to_user(buf, msg, len);
    *off += len;
    return len;
  } else {
    error = copy_to_user(buf, msg, remains);
    *off += remains;
    return remains;
  }

  // cannot reach here anyway
  return 0;
}

static ssize_t clk_write(struct file *f, const char __user *buf, size_t len,
                         loff_t *off) {
  char *msg;

  int error = 0;
  printk("kB write\n");
  if (len > MAXSIZE) {
    len = MAXSIZE;
  }
  error = copy_from_user(words, buf, len);
  *off = len;
  size = len;
  return len;
}

static int __init clk_driver_init(void) {
  int j;

  if ((alloc_chrdev_region(&dev, 0, 1, "kB")) < 0) {
    goto r_unreg;
  }

  cdev_init(&clk_cdev, &fops);

  if ((cdev_add(&clk_cdev, dev, 1)) < 0) {
    goto r_del;
  }

  if (IS_ERR(clk_class = class_create(THIS_MODULE, "kB"))) {
    goto r_class;
  }

  if (IS_ERR(device_create(clk_class, NULL, dev, NULL, "kB0"))) {
    goto r_device;
  }

  // initialise memory space
  for (j = 0; j < MAXSIZE; j++)
    words[j] = 0x41 + (j % 26);
  size = MAXSIZE;

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
