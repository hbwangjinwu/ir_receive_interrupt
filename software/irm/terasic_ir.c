#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#define DEVICE_NAME "irm"
struct irm_res {
	void __iomem *reg_base;
	int irq;
	unsigned int ir_receive_value;
	int ir_receive_state;//set to 1 when receive value
};

static struct irm_res *irm;
/* define and initial wait quene head */  
static DECLARE_WAIT_QUEUE_HEAD(irm_waitq); 

static DEFINE_SPINLOCK(interrupt_flag_lock);
static irqreturn_t irm_interrupt(int irq, void *dev_id)  
{    	
	if(irq!=irm->irq)	
		return IRQ_NONE;
	spin_lock(&interrupt_flag_lock);
	irm->ir_receive_state = 1;  
	irm->ir_receive_value=ioread32(irm->reg_base);
	spin_unlock(&interrupt_flag_lock);
	iowrite8(0,irm->reg_base);//write anything to clear interrupt
    	wake_up_interruptible(&irm_waitq);   
    	return IRQ_HANDLED;  
}  

static int irm_open(struct inode *inode, struct file *filp)
{
	int error;
	spin_lock(&interrupt_flag_lock);
	irm->ir_receive_state=0;
	spin_unlock(&interrupt_flag_lock);
	error = request_irq(irm->irq, irm_interrupt,IRQF_TRIGGER_RISING,"irm_interrupt", irm);
	if (error) {
		printk(KERN_INFO "failed requesting interrupt\n");
	}
	iowrite8(0,irm->reg_base);
        return 0;
}
static ssize_t irm_read(struct file *file, char __user *user, size_t size,loff_t *ppos)
{
	wait_event_interruptible(irm_waitq,irm->ir_receive_state);
	spin_lock(&interrupt_flag_lock);
	irm->ir_receive_state=0;
	spin_unlock(&interrupt_flag_lock);
	unsigned char value= irm->ir_receive_value>>16;
	copy_to_user(user, &value, min(sizeof(value),size)); 
	return 1;
	
}
static int irm_release(struct inode *inode, struct file *filp)
{
        free_irq(irm->irq,irm);
	return 0;
}


static struct file_operations irm_fops = {
    .owner = THIS_MODULE,
    .open = irm_open,
    .read =irm_read,
    .release = irm_release,
};

static struct miscdevice irm_misdev = 
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &irm_fops,
};


static int irm_probe(struct platform_device *pdev)
{
	struct resource *res;
	resource_size_t size;
	int irq;
	int error;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Can't get memory resource\n");
		return -ENOENT;
	}
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "Can't get interrupt resource\n");
		return irq;
	}
	irm = kzalloc(sizeof(*irm), GFP_KERNEL);
	
	size = resource_size(res);
	if (!request_mem_region(res->start, size, pdev->name)) {
		dev_err(&pdev->dev, "IRM registers are not free\n");
		error = -EBUSY;
		goto err_free_mem;
	}
	irm->reg_base = ioremap(res->start, size);
	if (!irm->reg_base) {
		dev_err(&pdev->dev, "Can't map memory\n");
		error = -ENOMEM;
		goto err_release_mem;
	}
	irm->irq=irq;
	 error = misc_register(&irm_misdev);
    if (error < 0 ) {
        printk("misc register fail! ret = %d\n",error);
        goto err_regmisc;
    }
	platform_set_drvdata(pdev, irm);
	device_init_wakeup(&pdev->dev, 1);
	return 0;
	err_regmisc:
		iounmap(irm->reg_base);
	err_release_mem:
		release_mem_region(res->start,size);
	err_free_mem:
		kfree(irm);
		return error;
}

static int irm_remove(struct platform_device *pdev)
{
	struct irm_res *irm_temp = platform_get_drvdata(pdev);
	struct resource *res;
	misc_deregister(&irm_misdev);
	device_init_wakeup(&pdev->dev, 0);
	iounmap(irm_temp->reg_base);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, resource_size(res));
	kfree(irm_temp);
	return 0;
}


static struct of_device_id terasic_ir_match[] = {
	{ .compatible = "terasic,irm",  .data = NULL},
	{},
};
MODULE_DEVICE_TABLE(of, terasic_ir_match);


static struct platform_driver irm_diver={
		.probe= irm_probe,
		.remove = irm_remove,
		.driver= {
			.owner=THIS_MODULE,
			.name="terasic_irm",
			.of_match_table=terasic_ir_match,
		},
};		


module_platform_driver(irm_diver);

MODULE_DESCRIPTION("ir  softip test driver");
MODULE_AUTHOR("matthew wang--terasic");
MODULE_LICENSE("GPL");
