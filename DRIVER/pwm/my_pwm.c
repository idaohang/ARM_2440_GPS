#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/major.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <asm/uaccess.h>	/* for VERIFY_READ/VERIFY_WRITE/copy_from_user */

#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <plat/regs-timer.h>
#include <mach/regs-irq.h>
#include <asm/mach/time.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#include <mach/regs-gpio.h>
#include <linux/gpio.h>

#define BUF_RW_MAX_SIZE 128
#define mini2440_pwm_NAME "mini2440_pwm" //新起的名字，避免与原有的LED驱动名字冲突
#define mini2440_pwm_CMD_0 _IOW('l', 0, int)
#define mini2440_pwm_CMD_1 _IOW('l', 1, int)
#define mini2440_pwm_CMD_2 _IOW('l', 2, int)
#define mini2440_pwm_CMD_3 _IOW('l', 3, int)

#define PWM_IOCTL_SET_FREQ		1
#define PWM_IOCTL_STOP			0



static void PWM_Stop(void);


struct mini2440_pwm_dev{	
	struct cdev cdev;
};


static struct timer_list my_timer;




//定时器,回调函数
static void my_timer_func(unsigned long data){
	PWM_Stop();
	return;
	
}


static void PWM_Set_Freq( unsigned long freq )  //设置pwm频率，并输出
{
	unsigned long tcon;
	unsigned long tcnt;
	unsigned long tcfg1;
	unsigned long tcfg0;

	struct clk *clk_p;
	unsigned long pclk;

	//set GPB0 as tout0, pwm output
	s3c2410_gpio_cfgpin(S3C2410_GPB(0), S3C2410_GPB0_TOUT0);

	tcon = __raw_readl(S3C2410_TCON);
	tcfg1 = __raw_readl(S3C2410_TCFG1);
	tcfg0 = __raw_readl(S3C2410_TCFG0);

	//prescaler = 50
	tcfg0 &= ~S3C2410_TCFG_PRESCALER0_MASK;
	tcfg0 |= (50 - 1); 

	//mux = 1/16
	tcfg1 &= ~S3C2410_TCFG1_MUX0_MASK;
	tcfg1 |= S3C2410_TCFG1_MUX0_DIV16;

	__raw_writel(tcfg1, S3C2410_TCFG1);
	__raw_writel(tcfg0, S3C2410_TCFG0);

	clk_p = clk_get(NULL, "pclk");
	pclk  = clk_get_rate(clk_p);
	tcnt  = (pclk/50/16)/freq;
	
	__raw_writel(tcnt, S3C2410_TCNTB(0));
	__raw_writel(tcnt/2, S3C2410_TCMPB(0));
				
	tcon &= ~0x1f;
	tcon |= 0xb;		//disable deadzone, auto-reload, inv-off, update TCNTB0&TCMPB0, start timer 0
	__raw_writel(tcon, S3C2410_TCON);
	
	tcon &= ~2;			//clear manual update bit
	__raw_writel(tcon, S3C2410_TCON);
}

static void PWM_Stop(void)  //停止pwm输出
{
	s3c2410_gpio_cfgpin(S3C2410_GPB(0), S3C2410_GPIO_OUTPUT);
	s3c2410_gpio_setpin(S3C2410_GPB(0), 0);
}

static struct mini2440_pwm_dev *p_mini2440_pwm_dev;

static struct class * p_mini2440_pwm_class = NULL;			// 设备类 /sys/class
static struct device *p_mini2440_pwm_device = NULL;			// mini2440_pwm的设备结构

static int mini2440_pwm_major = 0;     						// 主设备号变量
static int mini2440_pwm_minor	  = 0;							// 次设备号变量
static char buf_rw[BUF_RW_MAX_SIZE] = "mini2440_pwm is ok";

/*
 * 模块参数
 */
module_param(mini2440_pwm_major, int, S_IRUGO);
module_param(mini2440_pwm_minor, int, S_IRUGO);

static int mini2440_pwm_open(struct inode *inode, struct file *file)
{
	printk("mini2440_pwm_open\n");
	return 0;
}
/*
 * @brief			读mini2440_pwm设备
 * @param[in]		filp						文件结构
 * @param[in]		count						读数据的字节数	
 * @param[out]		buf							输出数据的缓冲区
 * @param[in|out]	f_pos						文件指针的位置
 * @return			读取的数据量	
 *					@li >= 0					读取的字节数目
 *					@li < 0						错误码
 */
ssize_t mini2440_pwm_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	//printk(KERN_WARNING"current: pid= %d\n", current->pid);
	//printk(KERN_WARNING"current: comm= %s\n", current->comm);
	printk("mini2440_pwm_read= %d,%s\n", count,buf_rw);
  
  count %= BUF_RW_MAX_SIZE;
	copy_to_user(buf, buf_rw, count);
	
	return 0;
}

/*
 * @brief			写mini2440_pwm设备
 * @param[in]		filp						文件结构
 * @param[in]		count						读数据的字节数	
 * @param[in]		buf							输出数据的缓冲区
 * @param[in|out]	f_pos						文件指针的位置
 * @return			写出结果	
 *					@li >= 0					写入的字节数量
 *					@li < 0						错误码
 */
ssize_t mini2440_pwm_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
  printk("mini2440_pwm_write= %d,%s\n", count,buf);
  
  count %= BUF_RW_MAX_SIZE;
	copy_from_user(buf_rw, buf, count);
	
	return count;
}


static int mini2440_pwm_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int led_io_state;
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	
	init_timer(&my_timer);
  my_timer.function = my_timer_func;
  my_timer.data = 0;
	

	get_user(led_io_state, p);
	
	printk("mini2440_pwm_ioctl:cmd:%x,led_io_state=%d\n",cmd,led_io_state);

	switch(cmd)
	{
		case PWM_IOCTL_SET_FREQ:
		if (arg == 0)
			return -EINVAL;
		PWM_Set_Freq(arg);
		mod_timer(&my_timer, jiffies + HZ);
		//printk("pwm set.");
		break;

		case PWM_IOCTL_STOP:
		PWM_Stop();
		printk("pwm stop");
		break;
		default:
			return -ENOTTY;
	}
	
	return 0;
}

static int mini2440_pwm_release(struct inode *inode, struct file *file)
{
	printk("mini2440_pwm_release\n");
	return 0;
}

static const struct file_operations mini2440_pwm_fops = {
	.owner = THIS_MODULE,
	.open = mini2440_pwm_open,
	.read = mini2440_pwm_read,
	.write = mini2440_pwm_write,
	.release = mini2440_pwm_release,
	.ioctl = mini2440_pwm_ioctl
};

static int mini2440_pwm_setup_cdev(struct mini2440_pwm_dev *p_mini2440_pwm_dev, dev_t devno)
{
	int ret = 0;

	cdev_init(&p_mini2440_pwm_dev->cdev, &mini2440_pwm_fops);
	p_mini2440_pwm_dev->cdev.owner = THIS_MODULE;
	ret = cdev_add(&p_mini2440_pwm_dev->cdev, devno, 1);

	return ret;
}

static int __init mini2440_pwm_init(void)
{
	int ret;
	dev_t devno;

	printk("mini2440_pwm_init\n");
	
	if(mini2440_pwm_major)
	{
		devno = MKDEV(mini2440_pwm_major, mini2440_pwm_minor);
		ret = register_chrdev_region(devno, 1, mini2440_pwm_NAME);
	}
	else
	{
		ret = alloc_chrdev_region(&devno, mini2440_pwm_minor, 1, mini2440_pwm_NAME);
		mini2440_pwm_major = MAJOR(devno);
		
	}
	printk("devno:%x, mini2440_pwm_major:%d\n",devno, mini2440_pwm_major);
	
	if(ret < 0)
	{
		printk("get mini2440_pwm_major fail\n");
		return ret;
	}
	
	p_mini2440_pwm_dev = kmalloc(sizeof(struct mini2440_pwm_dev), GFP_KERNEL);
	if(!p_mini2440_pwm_dev)
	{
		printk("%s...%d fail \n", __FUNCTION__,__LINE__);
		ret = -ENOMEM;
		goto device_kmalloc_fail;
	}

	memset(p_mini2440_pwm_dev, 0, sizeof(struct mini2440_pwm_dev));
	
	ret = mini2440_pwm_setup_cdev(p_mini2440_pwm_dev, devno);
	if(ret)
	{
		printk("mini2440_pwm_setup_cdev fail = %d\n",ret);
		goto cdev_add_fail;
	}
	
	p_mini2440_pwm_class = class_create(THIS_MODULE, mini2440_pwm_NAME);
	ret = IS_ERR(p_mini2440_pwm_class);
	if(ret)
	{
		printk("mini2440_pwm class_create fail\n");
		goto class_create_fail;
	}

	//p_mini2440_pwm_device = class_device_create(p_mini2440_pwm_class, NULL, devno, NULL, mini2440_pwm_NAME); // 早期内核版本
	p_mini2440_pwm_device = device_create(p_mini2440_pwm_class, NULL, devno, NULL, mini2440_pwm_NAME);
	ret = IS_ERR(p_mini2440_pwm_device);
	if (ret)
	{
		printk(KERN_WARNING "mini2440_pwm device_create fail, error code %ld", PTR_ERR(p_mini2440_pwm_device));
		goto device_create_fail;
	}
	
	return 0;
	
device_create_fail:
	class_destroy(p_mini2440_pwm_class);
class_create_fail:
	cdev_del(&p_mini2440_pwm_dev->cdev);
cdev_add_fail:
	kfree(p_mini2440_pwm_dev);
device_kmalloc_fail:
	unregister_chrdev_region(devno, 1);

	return ret;
}

static void __exit mini2440_pwm_exit(void)
{
	dev_t devno;
	
	printk("mini2440_pwm_exit\n");
	devno = MKDEV(mini2440_pwm_major, mini2440_pwm_minor);
	//class_device_destroy(p_mini2440_pwm_class, devno); // 早期内核版本
	device_destroy(p_mini2440_pwm_class, devno);
	class_destroy(p_mini2440_pwm_class);	
	
	cdev_del(&p_mini2440_pwm_dev->cdev);
	
	kfree(p_mini2440_pwm_dev);
	
	unregister_chrdev_region(devno, 1);
}

module_init(mini2440_pwm_init);
module_exit(mini2440_pwm_exit);

MODULE_AUTHOR("shenrt");
MODULE_DESCRIPTION("mini2440_pwm Driver");
MODULE_LICENSE("GPL");
