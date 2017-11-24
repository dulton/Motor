#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
//#include <linux/spi/spidev.h>
#include "spidev.h"




#define ARRAY_SIZE(array) sizeof(array)/sizeof(array[0])
#define ADT7310_READ_ID 0x58
#define ADT7310_READ_TEMPERATURE 0x50
#define ADT7310_READ_CONFIG 0x48
#define ADT7310_WRITE_CONFIG 0x08
#define ADT7310_READ_STATUS 0x40



int main(int argc, char **argv) 
{
	int i,fd;

 
	if (argc<2) {
		printf("Usage:\n%s [device]\n", argv[0]);
		exit(1);
	}
   	
	fd = open(argv[1], O_RDWR);

	if (fd<=0) { 
		printf("%s: Device %s not found\n", argv[0], argv[1]);
		exit(1);
	}

 #if 0
    char  cmd = 0;
    char  result = 0;

     

    while(1){

        #if 1          
        cmd  |= ADT7310_READ_ID;

        if (write(fd, &cmd/*wr_buf*/, sizeof(cmd)/*ARRAY_SIZE(wr_buf)*/) != sizeof(cmd)/*ARRAY_SIZE(wr_buf)*/)
            perror("Write Error");

       #endif

//        result |=  ADT7310_READ_ID;      
        #if 1
	if (read(fd, &result, sizeof(result)) != sizeof(result))
		perror("Read Error");
	else{
                printf("read ID : %x\n",result);
	}
        #endif
    }
#else

	int ret;
	char cmd = ADT7310_READ_ID;
        short cmd_s = 0;
	char result = 0;
        char result2 = 0;
        signed short result_s = 0;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)&cmd,//0
		.rx_buf = (unsigned long)&result,
		.len = sizeof(cmd)
	};
#if 0
        cmd =  ADT7310_READ_CONFIG;
        tr.tx_buf = (unsigned long )&cmd;
        tr.rx_buf = (unsigned long)&result2;
        tr.len = sizeof(cmd);
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

        if(ret < 0)
            printf("ioctl error\n");
        else{
            printf("Read Config register A:%x\n",result2);
            result2=0xFF;
        }
#endif


        cmd =ADT7310_READ_ID;
        tr.tx_buf = (unsigned long )&cmd;
        tr.rx_buf = (unsigned long)&result;
        tr.len = sizeof(cmd);
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

        if(ret < 0)
            printf("ioctl error\n");
        else{
            printf("ID:%x\n",result);
            result =0xFF;

        }

        sleep(1);




        cmd_s = ADT7310_WRITE_CONFIG;  
        cmd_s |= 0x80 << 8; 

        tr.tx_buf = (unsigned long )&cmd_s;
        tr.rx_buf = (unsigned long)NULL;
        tr.len = sizeof(cmd_s);
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

        if(ret < 0)
            printf("ioctl error\n");
        else
            printf("write Config register:0x80\n");


        sleep(1);


        cmd =ADT7310_READ_CONFIG;
        tr.tx_buf = (unsigned long )&cmd;
        tr.rx_buf = (unsigned long)&result;
        tr.len = sizeof(cmd);
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

        if(ret < 0)
            printf("ioctl error\n");
        else{
            printf("Config:%x\n",result);
            result =0xFF;

        }
/*
        cmd =ADT7310_READ_TEMPERATURE;
        tr.tx_buf = (unsigned long )&cmd;
        tr.rx_buf = (unsigned long)&result_s;
        tr.len = sizeof(cmd);
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

        if(ret < 0)
            printf("ioctl error\n");
        else{
            printf("tmperature:%x\n",result_s);
            result =0xFF;

        }
*/
    while(1){

        #if 1          
        cmd =ADT7310_READ_TEMPERATURE;

        if (write(fd, &cmd/*wr_buf*/, sizeof(cmd)/*ARRAY_SIZE(wr_buf)*/) != sizeof(cmd)/*ARRAY_SIZE(wr_buf)*/)
            perror("Write Error");

       #endif

//        result |=  ADT7310_READ_ID;      
        #if 1
	if (read(fd, &result_s, sizeof(result_s)) != sizeof(result_s))
		perror("Read Error");
	else{

                unsigned short tmp =0;

                tmp = result_s << 8;
                tmp |= (result_s >> 8) & 0xFF;
                printf("(((((((((((((((((((((((((((((read Temperature : %d (%x) )))))))))))\n\n",tmp/128,tmp);
	}
        #endif

        sleep(1);
    }



#if 0
        cmd =  ADT7310_READ_CONFIG;
        tr.tx_buf = (unsigned long )&cmd;
        tr.rx_buf = (unsigned long)&result;
        tr.len = sizeof(result);
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

        if(ret < 0)
            printf("ioctl error\n");
        else
            printf("Read Config register B:%x\n",result);

#endif
#endif
	close(fd);
	return 0;
}
