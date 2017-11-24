#include<linux/kernel.h>  
#include<linux/version.h>  
#include<linux/module.h>  
#include<linux/types.h>  
#include<linux/errno.h>  
#include<linux/fcntl.h>  
#include<linux/mm.h>  
#include<linux/proc_fs.h>  
#include<linux/fs.h>  
#include<linux/slab.h>  
#include<linux/init.h>  
#include<asm/uaccess.h>  
#include<asm/gpio.h>  
#include<asm/system.h>  
#include<linux/miscdevice.h>  
#include<linux/delay.h>  
#include<linux/sched.h>  
   
#include<linux/proc_fs.h>  
#include<linux/poll.h>  
   
#include<asm/bitops.h>  
#include<asm/uaccess.h>  
#include<asm/irq.h>  
   
#include<linux/moduleparam.h>  
#include<linux/ioport.h>  
#include<linux/interrupt.h>  
#include<linux/cdev.h>  
#include<linux/semaphore.h>  
#include<linux/wait.h>

  
#include "motor.h"

#define H_MAX 	31
#define H_MIN 	0
#define V_MAX 	15
#define V_MIN 	0



struct motor_drive_t 
{

	int	dir;		//drv dir
	int	dist;		//distance to move 
	int	curr_coord;
	int motor_status;
	int speed;
	spinlock_t   motor_lock;

};

struct	motor_drive_t  	motor_h_drv;
struct	motor_drive_t	motor_v_drv;

/****************************************************************************** 
**函数名称：Set_nCS 
**函数功能：禁用片选 
**输入参数：无 
**输出参数：无 
**注意：   高电平为禁用片选，低电平为使能片选 
******************************************************************************/  
void Set_nCS(void)  
{  
	__gpio_set_value(MOTOR_CS, LEVEL_HIGH);
}  
   
   
/****************************************************************************** 
**函数名称：Clr_nCS 
**函数功能：使能片选 
**输入参数：无 
**输出参数：无 
**注意：   高电平为禁用片选，低电平为使能片选 
******************************************************************************/  
void Clr_nCS(void)  
{  
	__gpio_set_value(MOTOR_CS, LEVEL_LOW);
}  
   
   
/****************************************************************************** 
**函数名称：Set_SCK 
**函数功能：SCK为高电平 
**输入参数：无 
**输出参数：无 
**注意：     
******************************************************************************/  
void Set_SCK(void)  
{  
	__gpio_set_value(MOTOR_SCLK, LEVEL_HIGH);
	//udelay(1);

}  
   
   
/****************************************************************************** 
**函数名称：Clr_SCK 
**函数功能：SCK为低电平 
**输入参数：无 
**输出参数：无 
**注意：     
******************************************************************************/  
void Clr_SCK(void)  
{  
	__gpio_set_value(MOTOR_SCLK, LEVEL_LOW);
	//udelay(1);
}  
   
   
/****************************************************************************** 
**函数名称：Set_MOSI 
**函数功能：MOSI为高电平 
**输入参数：无 
**输出参数：无 
**注意：     
******************************************************************************/  
void Set_MOSI(void)  
{  
	__gpio_set_value(MOTOR_SOUT, LEVEL_HIGH);
	//udelay(1);
}  
   
   
/****************************************************************************** 
**函数名称：Clr_MOSI 
**函数功能：MOSI为低电平 
**输入参数：无 
**输出参数：无 
**注意：     
******************************************************************************/  
void Clr_MOSI(void)  
{  
	__gpio_set_value(MOTOR_SOUT, LEVEL_LOW);
	//udelay(1);

}  
   
/****************************************************************************** 
**函数名称：MISO_H 
**函数功能：读取MISO_H的值 
**输入参数：无 
**输出参数：无 
**注意：     
******************************************************************************/  
u16 MISO_H(void)  
{  
	u16 regvalue;  

	regvalue = __gpio_get_value(MOTOR_SIN);
	//printk("miso data: %d\n", regvalue);

	return regvalue;  
}


/****************************************************************************** 
**函数名称：SPIBurstRead 
**函数功能：SPI连续读取模式 
**输入参数：adr——读取地址 
**          ptr——存储数据指针 
**          length 读取长度 
**输出参数：无，数据存在ptr中 
******************************************************************************/  
u16 SPIBurstRead(u8 addr)  
{  
	int i = 0;

	u16 value = 0;
	u32 data_temp = 0;
	u32 data_temp2 = 0;
	
	Clr_nCS();
	Set_SCK();
	udelay(20);
	
	Set_nCS();
	udelay(5);
	
	for(i = 0;i < 6;i++) {
		Clr_SCK();
		if(addr & 0x1) {  
			Set_MOSI();
		} else {  
			Clr_MOSI();
		}
		addr >>= 1;
		udelay(12);
		Set_SCK();
		udelay(15);
	}
	
	Clr_SCK();
	Set_MOSI(); //c0 = 1
	udelay(12);
	Set_SCK();
	udelay(15);
	
	Clr_SCK();
	Clr_MOSI(); //c1 = 0
	udelay(15);
	Set_SCK();
	udelay(15);
	
	for (i = 0;i < 16;i++){
		Clr_SCK();
		value = MISO_H();
		if (value == 0)
			data_temp2 = 0;
		else
			data_temp2 = 1;
			
		data_temp |= data_temp2<< i;
		udelay(12);
		Set_SCK();
		udelay(15);
	}
	
	udelay(15);
	Clr_nCS();
	
	//printk("data1=%#x\n", data_temp);

	return data_temp;
}  
   
/****************************************************************************** 
**函数名称：SPIBurstWrite 
**函数功能：SPI连续写入模式 
**输入参数：adr——写入地址 
**          ptr——存储数据指针 
**          length 写入长度 
**输出参数：无 
******************************************************************************/  
void SPIBurstWrite(u8 addr, u16 value)  
{  
	u8 i = 0;
	
	Clr_nCS();
	Set_SCK();
	udelay(10);
	
	Set_nCS();
	udelay(5);
	
	for(i = 0;i < 6;i++) {
		Clr_SCK();
		if(addr & 0x1) {  
			Set_MOSI();
		} else {  
			Clr_MOSI();
		}
		addr >>= 1;
		udelay(3);
		Set_SCK();
		udelay(5);
	}
	
	Clr_SCK();
	Clr_MOSI(); //c0 = 0
	udelay(5);
	Set_SCK();
	udelay(5);
	
	Clr_SCK();
	Clr_MOSI(); //c1 = 0
	udelay(5);
	Set_SCK();
	udelay(5);
	
	for (i = 0;i < 16; ++i) {
		Clr_SCK();
		if(value & 0x1) {  
			Set_MOSI();
		} else {  
			Clr_MOSI();
		}
		value >>= 1;
		udelay(2);
		Set_SCK();
		udelay(5);
	}
	
	udelay(5);
	Clr_nCS();
	
}  
void ms41909_param_init(void)
{
	u16 val = 0;
	
	val = 0x0080;
	SPIBurstWrite(0x0b, val);	 //设定频率为	   DT1延时设为 3ms   0x1e0a
	msleep(1);
	val = 0x0E03;
	SPIBurstWrite(0x20, val);	 //设定频率为	   DT1延时设为 3ms   0x1e0a
	msleep(1);
	
	//垂直电机
	val = 0x0002;
	SPIBurstWrite(0x22, val);	 //相位校正0度，DT2延时设为 0
	msleep(1);
	val = 0xd8d8;
	SPIBurstWrite(0x23, val);	 //设置AB占空比为 90%  0xd8d8		设定占空比0xf0f0 100%	  , 0x7878 50%
	msleep(1);
	val = 0x0900;
	SPIBurstWrite(0x24, val);	 //AB 256细分	设定电流方向反，计数0 
	msleep(1);
	val = 0x03ff;
	SPIBurstWrite(0x25, val);  //设置STEP为3.64ms  0x07ff
	msleep(1);
	
	//水平电机
	val = 0x0002;
	SPIBurstWrite(0x27, val);	 //相位校正0度，DT2延时设为 0
	msleep(1);
	val = 0xd8d8;
	SPIBurstWrite(0x28, val);	 //设置CD占空比为 90%  0xd8d8		设定占空比0xf0f0 100%	  , 0x7878 50%
	msleep(1);
	val = 0x0900;
	SPIBurstWrite(0x29, val);	 //CD 256细分	设定电流方向反，计数0 
	msleep(1);
	val = 0x00ff;
	SPIBurstWrite(0x2a, val);  //设置STEP为3.64ms  0x07ff
	
	msleep(1);
	val = 0x0087;
	SPIBurstWrite(0x21, val);  //设置STEP为3.64ms  0x07ff
	
}


void motor_h_param_init(void)
{
	motor_h_drv.dir							= DIR_FORWARD;
	motor_h_drv.dist						= 0;
	motor_h_drv.motor_status				= MOTOR_STILL;
	
	spin_lock_init(&motor_h_drv.motor_lock);
}

void motor_v_param_init(void)
{
	motor_v_drv.dir							= DIR_FORWARD;
	motor_v_drv.dist						= 0;
	motor_v_drv.motor_status				= MOTOR_STILL;
	
	spin_lock_init(&motor_v_drv.motor_lock);
}

static int m41909_reg_proc_write( struct file *filp,
                                        const char __user *buff,
                                        unsigned long len,
                                        void *data )
{
   
	return 0;
}


static int m41909_reg_proc_read(char *buf, char **start, off_t offset,
                        int count, int *eof, void *data)
{
	int i;
    int len = 0;
    u16 value = 0;
    u8 addr = 0;
    addr = 0x0b;
   	value = SPIBurstRead(addr);
    printk("m41909 [%#x]=>%#x\n", addr, value);
    
    for(i = 0x20; i < 0x2B; i++ ) {
    	addr = i;
    	if (addr == 0x26)
    		continue;
	   	value = SPIBurstRead(addr);
    	printk("m41909 [%#x]=>%#x\n", addr, value);
    }
    return len;
}


void motor41909_proc_init(void)
{
    struct proc_dir_entry *motor_proc_entry = NULL;
    char name[strlen("m41909_reg")];

    sprintf(name, "m41909_reg");

	motor_proc_entry = create_proc_entry(name, 0644, NULL);

	if(motor_proc_entry == NULL)
		printk(KERN_INFO "proc entry creat failed for motor_proc_entry!\n");
	else {
	  	motor_proc_entry->read_proc = m41909_reg_proc_read;
	  	motor_proc_entry->write_proc = m41909_reg_proc_write;
	  }
}

/*
**************************************************************************************
***************************Hortional Motor Param Config API*******************************
**************************************************************************************
*/

void VDFZ(void) 
{
	__gpio_set_value(MOTOR_VDFZ, LEVEL_LOW);
	mdelay(1);
	__gpio_set_value(MOTOR_VDFZ, LEVEL_HIGH);
	mdelay(1);
	__gpio_set_value(MOTOR_VDFZ, LEVEL_LOW);
}

void motor_h_dir_set(int h_dir)
{
	u8 addr;
	u16 val, read_val;
	
	addr = CCWCWCD_D8;
	val = (u16)h_dir << 8;

	/*  set spi core to set dir */
	read_val = SPIBurstRead(addr);
	
	read_val &= 0xfeff;/*clear CCWCWCD*/
	val = read_val | val;
	
	SPIBurstWrite(addr, val);
	
	VDFZ();
	
	motor_h_drv.dir = h_dir;

}

void motor_h_dist_set(int dist)
{
	u8 addr;
	u16 val;
	u16 read_val;
	//int h_dir = motor_h_drv.dir;
	
	int i = dist * 4;
	
	/* 单位步数*/
	int h_dist = 0xff;

	addr = PSUMCD_D7_D0;
	
	/* set spi core to set dist */
	read_val = SPIBurstRead(addr);
	
	read_val &= 0xfd00; /*clear PSUMCD*/
	val = read_val | h_dist | MOTOR_ENABLE; 

	SPIBurstWrite(addr, val); 
	
	for (; i > 0; i--){
			/*可以用定时器再尝试看看*/
			VDFZ();
			msleep(45);
	}
	
	/* disable motor */
	val = read_val & 0xfb00; 
	SPIBurstWrite(addr, val); 
	
	VDFZ();

	if (motor_h_drv.dir == MOTOR_DIR_LEFT)
		motor_h_drv.dist -= dist;
	else if (motor_h_drv.dir == MOTOR_DIR_RIGHT)
		motor_h_drv.dist += dist;
	else 
		;
		
	if (motor_h_drv.dist >= H_MAX)
		motor_h_drv.dist = H_MAX;
	else if (motor_h_drv.dist <= H_MIN)
		motor_h_drv.dist = H_MIN;
	
	//printk("hdist = %d\n", motor_h_drv.dist);
	 
}



int motor_h_curr_coord_get(void)
{
	int	h_dist;

	h_dist = motor_h_drv.dist;

	return	h_dist;
}

void motor_h_curr_coord_set(int loca)
{
	motor_h_drv.dist = loca;
}



int motor_h_status_get(void)
{
	int	motor_h_status;

	motor_h_status = motor_h_drv.motor_status;

	return	motor_h_status;			
}

int motor_h_speed_set(int speed)
{
	int status = 0;

	speed = motor_h_drv.speed;	
	
	return status;
}


/*
**************************************************************************************
***************************Vertical Motor Param Config API********************************
**************************************************************************************
*/

void motor_v_dir_set(int v_dir)
{
	u8 addr;
	u16 val, read_val;
	
	addr = CCWCWAB_D8;
	val = (u16)v_dir << 8;

	/*  set spi core to set dir */
	read_val = SPIBurstRead(addr);

	read_val &= 0xfeff;/*clear CCWCWAB*/
	val = read_val | val;
	
	SPIBurstWrite(addr, val);
	VDFZ();
	
	motor_v_drv.dir = v_dir;
	
	

}

void motor_v_dist_set(int dist)
{
	u8 addr;
	u16 val;
	u16 read_val;
	//int v_dir = motor_v_drv.dir;
	
	int i = dist * 4;
	
	/* 单位步数*/
	int v_dist = 0xff;

	addr = PSUMAB_D7_D0;

	
	/* set spi core to set dist */
	read_val = SPIBurstRead(addr);
	
	read_val &= 0xfd00; /*clear PSUMCD*/
	val = read_val | v_dist | MOTOR_ENABLE; 

	SPIBurstWrite(addr, val); 
	
	for (; i > 0; i--){
			/*可以用定时器再尝试看看*/
			VDFZ();
			msleep(30);
	}
	
	/* disable motor */
	val = read_val & 0xfb00; 
	SPIBurstWrite(addr, val); 
	
	VDFZ();
	
	if (motor_v_drv.dir == MOTOR_DIR_UP)
		motor_v_drv.dist += dist;
	else if (motor_v_drv.dir == MOTOR_DIR_DOWN)
		motor_v_drv.dist -= dist;
	else
		;
		
	if (motor_v_drv.dist >= V_MAX)
		motor_v_drv.dist = V_MAX;
	else if (motor_v_drv.dist <= V_MIN)
		motor_v_drv.dist = V_MIN;
		
	//printk("vdist = %d\n", motor_v_drv.dist);

}


int motor_v_curr_coord_get(void)
{
	int	v_dist;

	v_dist = motor_v_drv.dist;

	return	v_dist;
}

void motor_v_curr_coord_set(int loca)
{
	motor_v_drv.dist = loca;
}


int motor_v_status_get(void)
{
	int	motor_v_status;

	motor_v_status = motor_v_drv.motor_status;

	return	motor_v_status;		
}


int motor_v_speed_set(int speed)
{
	int status = 0;

	speed = motor_v_drv.speed;
	
	return status;
}


