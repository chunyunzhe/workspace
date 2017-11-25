#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>

#include <asm/io.h>
//#include <asm/system.h>
#include <asm/uaccess.h>

#define GLOBALMEM_SIZE 0x1000
#define MEM_CLEAR 0x01
#define GLOBALMEM_MAJOR 250
#define init_MUTEX(sem) sema_init(sem, 1)
#define init_MUTEX_LOCKED(sem) sema_init(sem, 0)

static int globalmem_major = GLOBALMEM_MAJOR;
struct globalmem_dev{
	struct cdev cdev;
	unsigned char mem[GLOBALMEM_SIZE];

	struct semaphore sem;
};
struct globalmem_dev *globalmem_devp;
static int globalmem_open(struct inode *, struct file *);
static int globalmem_release(struct inode *, struct file *);
//static long globalmem_ioctl(struct inode *, struct file *, unsigned int, unsigned long);
static long globalmem_ioctl(struct file *, unsigned int, unsigned long);
static ssize_t globalmem_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t globalmem_write(struct file *, const char __user *, size_t, loff_t *);
static loff_t globalmem_llseek(struct file *, loff_t, int);
static void globalmem_setup_cdev(struct globalmem_dev *, int);
static const struct file_operations globalmem_fops = {
	.owner = THIS_MODULE,
	.llseek = globalmem_llseek,
	.read = globalmem_read,
	.write = globalmem_write,
	.unlocked_ioctl = globalmem_ioctl,
	.open = globalmem_open,
	.release = globalmem_release
};

int globalmem_init(void);
void globalmem_exit(void);

MODULE_AUTHOR("春云者!");
MODULE_LICENSE("Dual BSD/GPL");
module_param(globalmem_major, int, S_IRUGO);

module_init(globalmem_init);
module_exit(globalmem_exit);
