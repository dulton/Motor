#ifndef	_MOTOR_H_
#define	_MOTOR_H_



/*
**************************************************************************************
********************************Motor Dev Num*****************************************
**************************************************************************************
*/

#define	MOTOR_DEV_NO_MAJOR				888
#define	MOTOR_DEV_NO_MINOR				0


/*
**************************************************************************************
********************************Motor Postion Int*************************************
**************************************************************************************
*/
#define	MOTOR_LEFT_GPIO_NUM				34	
#define	MOTOR_RIGHT_GPIO_NUM			35
#define	MOTOR_UP_GPIO_NUM				36	
#define	MOTOR_DOWN_GPIO_NUM				37


/*
**************************************************************************************
********************************MS41909 regs        *************************************
**************************************************************************************
*/
#define MODESEL_FZ_D9	0x0b
#define TESTEN1_D7		0x0b
	
#define PWMRES_D14_D13	0x20
#define PWMMODE_D12_D8	0x20
#define DT1_D7_D0		0x20
	
#define TESTEN2_D7		0x21
#define ZTEST_D4_D0		0x21
	
#define HMODAB_D13_D8	0x22
#define T2A_D7_D0		0x22
	
#define PPWB_D15_D8		0x23
#define PPWA_D7_D0		0x23
	
#define MICROAB_D13_D12	0x24
#define LEDB_D11		0x24
#define ENDISAB_D10		0x24
#define BRAKEAB_D9		0x24
#define CCWCWAB_D8		0x24
#define PSUMAB_D7_D0	0x24
	
#define INTCTAB_D15_D0	0x25
	
#define HMODCD_D13_D8	0x27
#define DT2B_D7_D0		0x27
	
#define PPWD_D15_D8		0x28
#define PPWC_D7_D0		0x28
	
#define MICROCD_D13_D12	0x29
#define LEDA_D11		0x29
#define ENDISCD_D10		0x29
#define BRAKECD_D9		0x29
#define CCWCWCD_D8		0x29
#define PSUMCD_D7_D0	0x29
	
#define INTCTCD_D15_D0	0x2A

#define MOTOR_ENABLE    0x0400


/*
**************************************************************************************
********************************Motor Dev Ioctl***************************************
**************************************************************************************
*/

#define	MOTOR_DEV_MAGIC					'M'

/*
**************************************************************************************
********************************Motor Ctrl Cmd****************************************
**************************************************************************************
*/
//HMOTOR
#define	MOTOR_IOC_H_START				_IOW(MOTOR_DEV_MAGIC, 1, int)
#define	MOTOR_IOC_H_STOP				_IOW(MOTOR_DEV_MAGIC, 2, int)
#define	MOTOR_IOC_H_DIR_SET				_IOW(MOTOR_DEV_MAGIC, 3, int)
#define	MOTOR_IOC_H_DIST_SET			_IOW(MOTOR_DEV_MAGIC, 4, int)
#define	MOTOR_IOC_H_COORD_CURR_GET		_IOW(MOTOR_DEV_MAGIC, 5, int)
#define	MOTOR_IOC_H_COORD_CURR_SET		_IOW(MOTOR_DEV_MAGIC, 6, int)
#define	MOTOR_IOC_H_SPEED_SET			_IOW(MOTOR_DEV_MAGIC, 7, int)

	
	
//VMOTOR
#define	MOTOR_IOC_V_START				_IOW(MOTOR_DEV_MAGIC, 21, int)
#define	MOTOR_IOC_V_STOP				_IOW(MOTOR_DEV_MAGIC, 22, int)
#define	MOTOR_IOC_V_DIR_SET				_IOW(MOTOR_DEV_MAGIC, 23, int)
#define	MOTOR_IOC_V_DIST_SET			_IOW(MOTOR_DEV_MAGIC, 24, int)
#define	MOTOR_IOC_V_COORD_CURR_GET		_IOW(MOTOR_DEV_MAGIC, 25, int)
#define	MOTOR_IOC_V_COORD_CURR_SET		_IOW(MOTOR_DEV_MAGIC, 26, int)
#define	MOTOR_IOC_V_SPEED_SET			_IOW(MOTOR_DEV_MAGIC, 27, int)



void ms41909_param_init(void);
void motor_h_param_init(void);
void motor_v_param_init(void);

void motor_h_dir_set(int h_dir);
void motor_h_dist_set(int dist);
int motor_h_curr_coord_get(void);
void motor_h_curr_coord_set(int loca);

int motor_h_status_get(void);
int motor_h_speed_set(int speed);

void motor_v_dir_set(int v_dir);
void motor_v_dist_set(int dist);
int motor_v_curr_coord_get(void);
void motor_v_curr_coord_set(int loca);

int motor_v_status_get(void);
int motor_v_speed_set(int speed);






/*
**************************************************************************************
********************************Motor GPIO Interface*************************************
**************************************************************************************
*/
#define	MOTOR_SCLK						64  
#define	MOTOR_SIN						65  
#define	MOTOR_SOUT						66  
#define	MOTOR_CS						63  
#define	MOTOR_VDFZ						39  

#define	MOTOR_RESET						47     



#define	MOTOR_RUN				0x10		
#define	MOTOR_STOP				0x11
#define MOTOR_STILL				0x12


/*
**************************************************************************************
********************************Interface State***************************************
**************************************************************************************
*/
#define	LEVEL_LOW						0
#define	LEVEL_HIGH						1


/*
**************************************************************************************
********************************Motor Drv Dir*****************************************
**************************************************************************************
*/
#define	DIR_FORWARD						1	
#define	DIR_BACKWARD					0

#define	MOTOR_DIR_LEFT					DIR_BACKWARD
#define	MOTOR_DIR_RIGHT					DIR_FORWARD
#define	MOTOR_DIR_UP					DIR_FORWARD
#define	MOTOR_DIR_DOWN					DIR_BACKWARD



#define MOTOR_KERR(fmt, args...) \
    {printk(KERN_EMERG "[MOTOR_KERR]: %s:%d->" fmt, __FUNCTION__, __LINE__, ## args);}

#define MOTOR_KPRT(fmt, args...) \
    {printk(KERN_DEBUG "[MOTOR_KPRT]: " fmt, ## args);}

#define MOTOR_KMSG(fmt, args...) \
    {printk(KERN_DEBUG "[MOTOR_KMSG]: " fmt, ## args);}

#define MOTOR_KDBG_ON

#ifdef MOTOR_KDBG_ON
#define MOTOR_KDBG(fmt, args...) \
    {   printk("[MOTOR_KDBG]:"); \
        printk(fmt, ## args);  }
#else
#define MOTOR_KDBG(fmt, args...)
#endif


#endif 	/*_MOTOR_H_*/

