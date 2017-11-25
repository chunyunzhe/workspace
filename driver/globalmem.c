#include "globalmem.h"

static int globalmem_open(struct inode *inode, struct file *filp)
{/*{{{*/
	filp->private_data = globalmem_devp;
	return 0;
}/*}}}*/

static int globalmem_release(struct inode *inode, struct file *filp)
{/*{{{*/
	return 0;
}/*}}}*/

static int globalmem_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{/*{{{*/
	struct globalmem_dev *dev = filp->private_data;
	switch(cmd){
		case MEM_CLEAR:

			if(down_interruptible(&dev->sem))
				return -ERESTARTSYS;

			memset(dev->mem, 0, GLOBALMEM_SIZE);

			up(&dev->sem);

			printk(KERN_INFO "globalmem is set to zero\n");
			break;
		default:
			return -EINVAL;
	}
	return 0;
}/*}}}*/

static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{/*{{{*/
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct globalmem_dev *dev = filp->private_data;

	//分析和获取有效的写长度
	if(p >= GLOBALMEM_SIZE)
		return 0;
	if(count > GLOBALMEM_SIZE - p)
		count = GLOBALMEM_SIZE - p;

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	//内核空间->用户空间
	if(copy_to_user(buf, (void *)(dev->mem + p), count))	//prototype:copy_to_user(void __user *to, const void *from, unsigned long n);
		ret = -EFAULT;
	else{
		*ppos += count;
		ret = count;
		printk(KERN_INFO "read %u bytes(s) from %lu\n", count, p);
	}

	up(&dev->sem);

	return ret;
}/*}}}*/

static ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{/*{{{*/
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct globalmem_dev *dev = filp->private_data;		//获得设备结构体指针!

	//分析和获取有效的写长度
	if(p >= GLOBALMEM_SIZE)
		return 0;
	if(count > GLOBALMEM_SIZE - p)
		count = GLOBALMEM_SIZE - p;

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	//用户空间->内核空间
	if(copy_from_user(dev->mem + p, buf, count))		//prototype:copy_from_user(void *to, const void __user *from, unsigned long n);
		ret = -EFAULT;
	else{
		*ppos += count;
		ret = count;
		printk(KERN_INFO "written %u bytes(s) from %lu\n", count, p);
	}

	up(&dev->sem);

	return ret;
}/*}}}*/

static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)	//para1:偏移量，para2:相对位置!
{/*{{{*/
	loff_t ret = 0;
	switch(orig){
	case 0:
		if(offset < 0){
			ret = -EINVAL;
			break;
		}
		if((unsigned int)offset > GLOBALMEM_SIZE){
			ret = -EINVAL;
			break;
		}
		filp->f_pos = (unsigned int)offset;
		ret = filp->f_pos;
		break;
	case 1:
		if((filp->f_pos + offset) > GLOBALMEM_SIZE){
			ret = -EINVAL;
			break;
		}
		if((filp->f_pos + offset) < 0){
			ret = -EINVAL;
			break;
		}
		filp->f_pos += offset;
		ret = filp->f_pos;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}/*}}}*/

static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{/*{{{*/
	int err, devno = MKDEV(globalmem_major, index);
	//初始化cdev
	cdev_init(&dev->cdev, &globalmem_fops);				//建立了驱动函数和cdev的联系!
	dev->cdev.owner = THIS_MODULE;
	//向内核添加cdev
	err = cdev_add(&dev->cdev, devno, 1);
	if(err)
		printk(KERN_NOTICE "Error %d adding globalmem %d", err, index);
}/*}}}*/

int globalmem_init(void)
{/*{{{*/
	int result;
	dev_t devno = MKDEV(globalmem_major, 0);		//MKDEV:将主设备号和次设备号转换为一个dev_t格式设备编号!
	//申请设备号
	if(globalmem_major)
		result = register_chrdev_region(devno, 1, "globalmem");//将起始设备编号为para0的设备连续注册进内核，注册数量为para1，设备相关联的设备名称为para2!
	else{
		result = alloc_chrdev_region(&devno, 0, 1, "globalmem");	//比起register_chrdev_region，para1是多出的,意取请求的最小此编号!
		globalmem_major = MAJOR(devno);
	}
	if(result < 0)
		return result;
	globalmem_devp = kzalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
	if(!globalmem_devp){
		result = -ENOMEM;
		goto fail_zalloc;
	}
	//注册cdev
	globalmem_setup_cdev(globalmem_devp, 1);	//为什么次设备号是1?

	init_MUTEX(&globalmem_devp->sem);
	return 0;
fail_zalloc:
	unregister_chrdev_region(devno, 1);
	return result;
}/*}}}*/

void globalmem_exit(void)
{/*{{{*/
	cdev_del(&globalmem_devp->cdev);
	kfree(globalmem_devp);
	unregister_chrdev_region(MKDEV(globalmem_major, 0), 1);
}/*}}}*/
