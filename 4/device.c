#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define DEVICE_NAME "chardev_example"
#define CLASS_NAME "chardev_class"
#define BUFFER_SIZE 256

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rostik");
MODULE_DESCRIPTION("Simple Character Device Example");
MODULE_VERSION("1.0");

static dev_t dev_number; 
static struct cdev cdev; 
static struct class *chardev_class; 
static struct device *chardev_device; 
static char device_buffer[BUFFER_SIZE]; 
static size_t data_size = 0; 


static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);



static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
};


static int device_open(struct inode *inode, struct file *file) {
    pr_info("chardev: Device opened\n");
    return 0;
}


static int device_release(struct inode *inode, struct file *file) {
    pr_info("chardev: Device closed\n");
    return 0;
}


static ssize_t device_read(struct file *file, char __user *buffer, size_t len, loff_t *offset) {
    size_t bytes_read;

    if (*offset >= data_size) 
        return 0;

    bytes_read = min(len, data_size - *offset);

    if (copy_to_user(buffer, device_buffer + *offset, bytes_read)) // Копируем данные в userspace
        return -EFAULT;

    *offset += bytes_read; 
    pr_info("chardev: Read %zu bytes\n", bytes_read);
    return bytes_read;
}


static ssize_t device_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset) {
    size_t bytes_to_write = min(len, BUFFER_SIZE - 1);

    if (copy_from_user(device_buffer, buffer, bytes_to_write)) 
        return -EFAULT;

    device_buffer[bytes_to_write] = '\0'; 
    data_size = bytes_to_write; 
    pr_info("chardev: Written %zu bytes\n", bytes_to_write);
    return bytes_to_write;
}


static int __init chardev_init(void) {
    int result;

    
    result = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (result < 0) {
        pr_err("chardev: Failed to allocate device number\n");
        return result;
    }

  
    cdev_init(&cdev, &fops);
    result = cdev_add(&cdev, dev_number, 1);
    if (result < 0) {
        pr_err("chardev: Failed to add cdev\n");
        unregister_chrdev_region(dev_number, 1);
        return result;
    }

    
    chardev_class = class_create(CLASS_NAME);
    if (IS_ERR(chardev_class)) {
        pr_err("chardev: Failed to create class\n");
        cdev_del(&cdev);
        unregister_chrdev_region(dev_number, 1);
        return PTR_ERR(chardev_class);
    }

    
    chardev_device = device_create(chardev_class, NULL, dev_number, NULL, DEVICE_NAME);
    if (IS_ERR(chardev_device)) {
        pr_err("chardev: Failed to create device\n");
        class_destroy(chardev_class);
        cdev_del(&cdev);
        unregister_chrdev_region(dev_number, 1);
        return PTR_ERR(chardev_device);
    }

    pr_info("chardev: Module loaded with device /dev/%s\n", DEVICE_NAME);
    return 0;
}


static void __exit chardev_exit(void) {
    device_destroy(chardev_class, dev_number);
    class_destroy(chardev_class);
    cdev_del(&cdev);
    unregister_chrdev_region(dev_number, 1);
    pr_info("chardev: Module unloaded\n");
}

module_init(chardev_init);
module_exit(chardev_exit);

