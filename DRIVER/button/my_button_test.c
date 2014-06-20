#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/input.h>
MODULE_AUTHOR("wjb");
MODULE_LICENSE("GPL");
struct input_dev* button_devp;
struct button_irq_desc{
 int irq;
 int pin;
 int number;
 char *name;
};
static struct button_irq_desc button_irqs[] = {
 {IRQ_EINT8, S3C2410_GPG(0), 0, "KEY0"},
 {IRQ_EINT11, S3C2410_GPG(3), 1, "KEY1"},
 {IRQ_EINT13, S3C2410_GPG(5), 2, "KEY2"},
 {IRQ_EINT15, S3C2410_GPG(6), 3, "KEY3"},
 {IRQ_EINT14, S3C2410_GPG(7), 4, "KEY4"},
 {IRQ_EINT19, S3C2410_GPG(11), 5, "KEY5"},
};
static irqreturn_t  button_interrupt(int irq, void* dev_id)
{
 struct button_irq_desc *button_irqs = (struct button_irq_desc*)dev_id;
 int down; 
 
 down = !s3c2410_gpio_getpin(button_irqs->pin);
 switch(button_irqs->irq)
 {
  case IRQ_EINT8:
   input_report_key(button_devp, BTN_0, down);
   input_sync(button_devp);
   break;
  case IRQ_EINT11:
   input_report_key(button_devp, BTN_1, down);
   input_sync(button_devp);  
   break;
  case IRQ_EINT13:
   input_report_key(button_devp, BTN_2, down);
   input_sync(button_devp);      
   break;
  case IRQ_EINT15:
   input_report_key(button_devp, BTN_3, down);
   input_sync(button_devp);      
   break;
  case IRQ_EINT14:
   input_report_key(button_devp, BTN_4, down);
   input_sync(button_devp);      
   break;
  case IRQ_EINT19:
   input_report_key(button_devp, BTN_5, down);
   input_sync(button_devp);       
   break;
  default:
   break;
 }    
 return IRQ_RETVAL(IRQ_HANDLED);
}
static int __init button_init(void)
{
    int i;
 int err;
 
 button_devp = input_allocate_device();
 for(i = 0; i < sizeof(button_irqs)/sizeof(button_irqs[0]); i++)
 {
  if(button_irqs[i].irq < 0)
  {
   continue;
  }
  err = request_irq(button_irqs[i].irq, button_interrupt, IRQ_TYPE_EDGE_BOTH, button_irqs[i].name, (void *)&button_irqs[i]);
  if(err)
  {
   break;
  }
 }
 
 if(err)
 {
  i--;
  for(; i >= 0; i--)
  {
   if(button_irqs[i].irq < 0)
   {
    continue;
   }
   disable_irq(button_irqs[i].irq);
   free_irq(button_irqs[i].irq, (void*)&button_irqs[i]);
  }
  return -EBUSY;
 }
        
  set_bit(EV_KEY, button_devp->evbit);
  set_bit(BTN_0, button_devp->keybit);
  set_bit(BTN_1, button_devp->keybit);
  set_bit(BTN_2, button_devp->keybit);
  set_bit(BTN_3, button_devp->keybit);
  set_bit(BTN_4, button_devp->keybit);
  set_bit(BTN_5, button_devp->keybit);
  input_register_device(button_devp);
  
  return 0;
}
static void __exit button_exit(void)
{
 int i = 5;
    input_unregister_device(button_devp);
    for(i = 5; i >= 0; i--)
 {
  if(button_irqs[i].irq < 0)
  {
   continue;
  }
  disable_irq(button_irqs[i].irq);
  free_irq(button_irqs[i].irq, (void*)&button_irqs[i]);
 }
}
module_init(button_init);
module_exit(button_exit);