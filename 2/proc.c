#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define PROC_NAME "example_proc" // Имя файла в /proc
#define BUFFER_SIZE 128          // Размер буфера

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rostik");
MODULE_DESCRIPTION("Example Kernel Module with proc communication");

static char *proc_buffer;         
static size_t proc_buffer_len;    
static struct proc_dir_entry *proc_entry; 

static ssize_t proc_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) {
    size_t len = proc_buffer_len;
    if (*ppos > 0 || count < len)
        return 0;

    if (copy_to_user(user_buf, proc_buffer, len))
        return -EFAULT;

    *ppos = len;
    return len;
}


static ssize_t proc_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos) {
    if (count > BUFFER_SIZE)
        count = BUFFER_SIZE;

    if (copy_from_user(proc_buffer, user_buf, count))
        return -EFAULT;

    proc_buffer_len = count;
    proc_buffer[count] = '\0'; 
    return count;
}


static const struct proc_ops proc_fops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};


static int __init proc_module_init(void) {
    proc_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!proc_buffer)
        return -ENOMEM;

    proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_fops);
    if (!proc_entry) {
        kfree(proc_buffer);
        return -ENOMEM;
    }

    proc_buffer_len = 0;
    pr_info("Proc module loaded. Access via /proc/%s\n", PROC_NAME);
    return 0;
}


static void __exit proc_module_exit(void) {
    if (proc_entry)
        remove_proc_entry(PROC_NAME, NULL);

    kfree(proc_buffer);
    pr_info("Proc module unloaded.\n");
}

module_init(proc_module_init);
module_exit(proc_module_exit);

