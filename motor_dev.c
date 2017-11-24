#include <linux/module.h>	/* For module stuff/... */
#include <linux/types.h>	/* For standard types (like size_t) */
#include <linux/errno.h>	/* For the -ENODEV/... values */
#include <linux/kernel.h>	/* For printk/panic/... */
#include <linux/fs.h>		/* For file operations */
#include <linux/init.h>		/* For __init/__exit/... */
#include <linux/uaccess.h>	/* For copy_to_user/put_user/... */
#include <linux/interrupt.h>
#include <linux/io.h>
#include <asm/uaccess.h>


//device-driver module 
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/err.h>  

//hardware api
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/clk.h>

//platform related 
#include <mach/io.h>

#include "motor.h"


struct class* motor_class;
struct device* motor_device;

struct motor_timer_t 
{
	//register address
	unsigned  int	tm_ctrl_addr;
	unsigned  int	tm_sts_addr;
	unsigned  int	tm_reload_addr;
	unsigned  int	tm_match1_addr;
	unsigned  int	tm_match2_addr;
	
	//config param
	
	//apb clk
	unsigned  int 	tm_input_freq;
	//no usage
	unsigned  int 	tm_output_freq;
	//interrupt freq--->motor pps
	unsigned  int  	tm_int_freq;
	//next reload count
	unsigned  int  	tm_reload_count; 
	
};

struct motor_dev_t 
{
	struct cdev 			cdev;
		
}motor_dev;


struct motor_timer_t	motor_tm_h;
struct motor_timer_t	motor_tm_v;

static dev_t motor_dev_no;

extern void ms41909_param_init(void);
extern void	motor41909_proc_init(void);

/*
**************************************************************************************
********************************Motor Cmd API******************************************
**************************************************************************************
*/
static long motor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

	unsigned int m_arg;
	long ret = 0;

	//check cmd magic
	if (_IOC_TYPE(cmd) != MOTOR_DEV_MAGIC)         
	{
		return -EINVAL;
	}

	//parse cmd
	switch(cmd)
	{
		/******************H motor***********************/
		case	MOTOR_IOC_H_DIR_SET:
			if (get_user(m_arg, (unsigned int __user *)arg))
				return -EFAULT;
			motor_h_dir_set(m_arg); 			
			break;
			
		case	MOTOR_IOC_H_DIST_SET:
			if (get_user(m_arg, (unsigned int __user *)arg))
				return -EFAULT;
			motor_h_dist_set(m_arg);	
			break;

		case 	MOTOR_IOC_H_COORD_CURR_GET:
			m_arg = motor_h_curr_coord_get();
			if (put_user(m_arg, (unsigned int __user *)arg))
				return -EFAULT;
			break;

		case 	MOTOR_IOC_H_COORD_CURR_SET:
			if (get_user(m_arg, (unsigned int __user *)arg))
				return -EFAULT;	
			motor_h_curr_coord_set(m_arg);
			break;
			
		case	MOTOR_IOC_H_SPEED_SET:
			if (get_user(m_arg, (unsigned int __user *)arg))
				return -EFAULT;
				
			motor_h_speed_set(m_arg);
			break;
			
		/******************V motor***********************/	
		case 	MOTOR_IOC_V_DIR_SET:
			if (get_user(m_arg, (unsigned int __user *)arg))
				return -EFAULT;
			motor_v_dir_set(m_arg);				
			break;
			
		case 	MOTOR_IOC_V_DIST_SET:
			if (get_user(m_arg, (unsigned int __user *)arg))
				return -EFAULT;
				motor_v_dist_set(m_arg);	
			break;
			
		case 	MOTOR_IOC_V_COORD_CURR_GET:
			m_arg = motor_v_curr_coord_get();
			if (put_user(m_arg, (unsigned int __user *)arg))
				return -EFAULT;
			break;

		case 	MOTOR_IOC_V_COORD_CURR_SET:
			if (get_user(m_arg, (unsigned int __user *)arg))
				return -EFAULT;
			motor_v_curr_coord_set(m_arg);
			break;

		case	MOTOR_IOC_V_SPEED_SET:
			if (get_user(m_arg, (unsigned int __user *)arg))
				return -EFAULT;
			motor_v_speed_set(m_arg);
			break;

		default:
			break;
			
			
	}

	return ret;
}


static int motor_open(struct inode *inode, struct file *file)
{
	ms41909_param_init();

	return 0;
}


static int motor_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations motor_fops = 
{
	.owner			= THIS_MODULE,
	.unlocked_ioctl	= motor_ioctl,
	.open			= motor_open,
	.release		= motor_release,
};


static int motor_gpio_init(void)
{
	int ret = 0;

	if((ret=gpio_request(MOTOR_SCLK, "motor_sclk"))<0)
	{
		goto GPIO_ERR0;
	}

	if((ret=gpio_request(MOTOR_SIN, "motor_sin"))<0)
	{
		goto GPIO_ERR1;
	}

	if((ret=gpio_request(MOTOR_SOUT, "motor_sout"))<0)
	{
		goto  GPIO_ERR2;
	}

	if((ret=gpio_request(MOTOR_CS, "motor_cs"))<0)
	{
		goto  GPIO_ERR3;
	}
	
	if((ret=gpio_request(MOTOR_RESET, "motor_reset"))<0)
	{
		goto  GPIO_ERR3;
	}

	if((ret=gpio_request(MOTOR_VDFZ, "motor_vdfz"))<0)
	{
		goto  GPIO_ERR3;
	}
	
	if((ret=gpio_direction_output(MOTOR_SCLK, LEVEL_HIGH))<0)
	{
		goto 	GPIO_ERR4;
	}
	
	if((ret=gpio_direction_input(MOTOR_SIN))<0)
	{
		goto 	GPIO_ERR4;
	}

	if((ret=gpio_direction_output(MOTOR_SOUT, LEVEL_HIGH))<0)
	{
		goto 	GPIO_ERR4;
	}
	
	if((ret=gpio_direction_output(MOTOR_CS, LEVEL_HIGH))<0)
	{
		goto 	GPIO_ERR4;
	}
	
	if((ret=gpio_direction_output(MOTOR_RESET, LEVEL_LOW))<0)
	{
		goto 	GPIO_ERR4;
	}

	if((ret=gpio_direction_output(MOTOR_VDFZ, LEVEL_LOW))<0)
	{
		goto 	GPIO_ERR4;
	}

	
	return ret;

GPIO_ERR4:
	gpio_free(MOTOR_RESET);
GPIO_ERR3:
	gpio_free(MOTOR_CS);
GPIO_ERR2:
	gpio_free(MOTOR_SOUT);	
GPIO_ERR1:
	gpio_free(MOTOR_SIN);
GPIO_ERR0:
	gpio_free(MOTOR_SCLK);

	return ret;
}

static int motor_gpio_exit(void)
{	
	gpio_free(MOTOR_CS);
	gpio_free(MOTOR_SOUT);
	gpio_free(MOTOR_SIN);
	gpio_free(MOTOR_SCLK);	
	gpio_free(MOTOR_RESET);
	gpio_free(MOTOR_VDFZ);

	return	0;
		
}

static int motor_init(void)
{
	int ret = 0;
	
	if((ret = motor_gpio_init()) < 0) {
		MOTOR_KERR("!!!MOTOR GPIO INIT ERROR!!!\r\n");
		return 	ret;
	}
	msleep(50);
	
	gpio_direction_output(MOTOR_RESET, LEVEL_HIGH);
	
	msleep(500);
	
	motor41909_proc_init();
	
	motor_h_param_init();
	motor_v_param_init();
	

	
	return ret;

}

static void motor_exit(void)
{

	motor_gpio_exit();

}


int  motor_dev_init(void)
{
	int err;
	
	motor_dev_no = MKDEV(MOTOR_DEV_NO_MAJOR, MOTOR_DEV_NO_MINOR);

	err = register_chrdev_region(motor_dev_no, 0, "motor");
	if(err)
	{
		MOTOR_KERR("allocate motor dev num error!\r\n");
		goto 	MOTOR_INIT_FAIL;
	}


	cdev_init(&motor_dev.cdev, &motor_fops);
	motor_dev.cdev.owner = THIS_MODULE;

	/* Add the device */
	err  = cdev_add(&motor_dev.cdev, motor_dev_no, 1);
	if (err) 
	{
		//NEVER forget to release resource
		MOTOR_KERR("add motor dev error!\r\n");
		goto 	CDEV_ERR;
		
	}

	//for motor device installed automatically 
	motor_class = class_create(THIS_MODULE, "motor");
	if(IS_ERR(motor_class))
	{
		err=PTR_ERR(motor_class);
		MOTOR_KERR("create motor class error!!!\r\n");
		goto 	CLASS_ERR;
	}

	motor_device = device_create(motor_class, NULL, motor_dev_no, NULL, "motor");
	if(IS_ERR(motor_device))
	{
		err=PTR_ERR(motor_device);
		MOTOR_KERR("create motor device error!!!\r\n");
		goto 	DEVICE_ERR;	
	}

	if((err = motor_init())<0)
	{
		MOTOR_KERR("motor horizion init error!!!\r\n");
		goto	DEVICE_ERR;
	}
	return 	0;


DEVICE_ERR:
	class_destroy(motor_class);
	
CLASS_ERR:
	cdev_del(&motor_dev.cdev);
	
CDEV_ERR:
	unregister_chrdev_region(motor_dev_no, 0);
	
MOTOR_INIT_FAIL:

	return err;
	
}



void  motor_dev_exit(void)
{
	
	//we will disable timer first
	motor_exit();
	
	//release cdev
	cdev_del(&motor_dev.cdev);
	unregister_chrdev_region(motor_dev_no, 0);

	//release class 
	device_destroy(motor_class, motor_dev_no);
	class_destroy(motor_class);

}

module_init(motor_dev_init);
module_exit(motor_dev_exit);

MODULE_AUTHOR("Chunlei Gan, <ganchunlei@longcheer.net>");
MODULE_LICENSE("GPL");
