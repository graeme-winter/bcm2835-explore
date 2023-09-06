#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#define GPIO_21 (21)

dev_t dev = 0;
static struct class *dev_class;
static struct cdev clk_cdev;

static int __init clk_driver_init(void);
static void __exit clk_driver_exit(void);

static int clk_open(struct inode *inode, struct file *file);
static int clk_release(struct inode *inode, struct file *file);
static ssize_t clk_read(struct file *filp, char __user *buf, size_t len,
                        loff_t *off);
static ssize_t clk_write(struct file *filp, const char *buf, size_t len,
                         loff_t *off);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = clk_read,
    .write = clk_write,
    .open = clk_open,
    .release = clk_release,
};

static int clk_open(struct inode *inode, struct file *file) { return 0; }

static int clk_release(struct inode *inode, struct file *file) { return 0; }

static ssize_t clk_read(struct file *filp, char __user *buf, size_t len,
                        loff_t *off) {
  uint8_t gpio_state = 0;

  gpio_state = gpio_get_value(GPIO_21);

  len = 1;
  if (copy_to_user(buf, &gpio_state, len) > 0) {
  }

  return 0;
}

static ssize_t clk_write(struct file *filp, const char __user *buf, size_t len,
                         loff_t *off) {
  uint8_t rec_buf[10] = {0};

  if (copy_from_user(rec_buf, buf, len) > 0) {
    // act on this error
  }

  if (rec_buf[0] == '1') {
    gpio_set_value(GPIO_21, 1);
  } else if (rec_buf[0] == '0') {
    gpio_set_value(GPIO_21, 0);
  }

  return len;
}

static int __init clk_driver_init(void) {
  if ((alloc_chrdev_region(&dev, 0, 1, "clk_Dev")) < 0) {
    goto r_unreg;
  }

  cdev_init(&clk_cdev, &fops);

  if ((cdev_add(&clk_cdev, dev, 1)) < 0) {
    goto r_del;
  }

  if (IS_ERR(dev_class = class_create(THIS_MODULE, "clk_class"))) {
    goto r_class;
  }

  if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "clk_device"))) {
    goto r_device;
  }

  if (gpio_is_valid(GPIO_21) == false) {
    goto r_device;
  }

  if (gpio_request(GPIO_21, "GPIO_21") < 0) {
    goto r_gpio;
  }

  gpio_direction_output(GPIO_21, 0);

  gpio_export(GPIO_21, false);

  return 0;

r_gpio:
  gpio_free(GPIO_21);
r_device:
  device_destroy(dev_class, dev);
r_class:
  class_destroy(dev_class);
r_del:
  cdev_del(&clk_cdev);
r_unreg:
  unregister_chrdev_region(dev, 1);

  return -1;
}

/*
** Module exit function
*/
static void __exit clk_driver_exit(void) {
  gpio_unexport(GPIO_21);
  gpio_free(GPIO_21);
  device_destroy(dev_class, dev);
  class_destroy(dev_class);
  cdev_del(&clk_cdev);
  unregister_chrdev_region(dev, 1);
}

module_init(clk_driver_init);
module_exit(clk_driver_exit);

MODULE_LICENSE("BSD");
MODULE_AUTHOR("Graeme Winter <graeme.winter@gmail.com>");
MODULE_DESCRIPTION("Simple wrapper around BCM2835 GPCLK");
MODULE_VERSION("0.1");
