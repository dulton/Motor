/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

static const char *device = "/dev/spidev0.0";
static uint32_t mode;
static uint8_t bits = 8;
static uint32_t speed = 2000000;
static uint16_t delay;

#define SPI_READ 	(0x7F)

int g_SPI_Fd = 0;
#define SPI_DEBUG


uint8_t reverse_byte(uint8_t c)
{
	uint8_t s = 0;
        int i;
        for (i = 0; i < 8; ++i) {
                s <<= 1;
                s |= c & 1;
                c >>= 1;
        }
        return s;
}


int spi_read(uint8_t addr, uint16_t *value)
{
	int ret;
	struct spi_ioc_transfer tr[2];
	uint16_t result;
	uint8_t cmd = addr | SPI_READ;

	cmd = reverse_byte(cmd) ;

	memset(tr, 0, sizeof(tr));
	tr[0].tx_buf = (unsigned long)&cmd;
	tr[0].rx_buf = 0;
	tr[0].len = 1;
	tr[0].delay_usecs = delay;
	tr[0].speed_hz = speed;
	tr[0].bits_per_word = bits;

	tr[1].tx_buf = 0;
	tr[1].rx_buf = (unsigned long)&result;
	tr[1].len = 2;
	tr[1].delay_usecs = delay;
	tr[1].speed_hz = speed;
	tr[1].bits_per_word = bits;

	ret = ioctl(fd, SPI_IOC_MESSAGE(2), tr);
	if (ret < 1) {
		printf("can't send spi message");
		ret = -1;
	} else {
		ret = 0;
	}

	printf("%.4X ", result);
	*value = result;
	return ret;
}

int spi_write(uint8_t addr, uint16_t value)
{
	int ret = 0;
	uint8_t tx[3] = {0};

	tx[0] = reverse_byte(addr);
	tx[1] = value >> 8;
	tx[2] = value;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = 0,
		.len = 3,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		printf("can't send spi message");
		ret = -1;
	} else {
		ret = 0;
	}
	return ret;
}

static void cmd_send(int fd)
{
	int ret;
	uint8_t tx_1[] = {
		0x04, 0x0F, 0x78,
	};
	uint8_t rx = 0;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx_1,
		.rx_buf = 0,
		.len = 3,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		printf("aaaacan't send spi message");
}

static void transfer_recv(int fd)
{
	int ret;
	uint8_t cmd = 0x06;
	uint8_t result = { 0 };

	struct spi_ioc_transfer tr[2];
	uint8_t buf[32] = {0};

	memset(tr, 0, sizeof(tr));
	memset(buf, 0, sizeof(buf));

	tr[0].tx_buf = (unsigned long)&cmd;
	tr[0].rx_buf = (unsigned long)0;
	tr[0].len = 1;
	tr[0].delay_usecs = delay;
	tr[0].speed_hz = speed;
	tr[0].bits_per_word = bits;

	tr[1].tx_buf = (unsigned long)0;
	tr[1].rx_buf = (unsigned long)buf;
	tr[1].len = 2;
	tr[1].delay_usecs = delay;
	tr[1].speed_hz = speed;
	tr[1].bits_per_word = bits;

	ret = ioctl(fd, SPI_IOC_MESSAGE(2), tr);
	if (ret < 1)
		printf("recv can't send spi message");

	for (ret = 0; ret < 2; ret++) {
		printf("%.2X ", buf[ret]);
	}
	printf("\n");
}


static void print_usage(const char *prog)
{
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev1.1)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word \n"
	     "  -l --loop     loopback\n"
	     "  -H --cpha     clock phase\n"
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
	     "  -C --cs-high  chip select active high\n"
	     "  -3 --3wire    SI/SO signals shared\n"
	     "  -N --no-cs    no chip select\n"
	     "  -R --ready    slave pulls low to pause\n"
	     "  -2 --dual     dual transfer\n" "  -4 --quad     quad transfer\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{"device", 1, 0, 'D'},
			{"speed", 1, 0, 's'},
			{"delay", 1, 0, 'd'},
			{"bpw", 1, 0, 'b'},
			{"loop", 0, 0, 'l'},
			{"cpha", 0, 0, 'H'},
			{"cpol", 0, 0, 'O'},
			{"lsb", 0, 0, 'L'},
			{"cs-high", 0, 0, 'C'},
			{"3wire", 0, 0, '3'},
			{"no-cs", 0, 0, 'N'},
			{"ready", 0, 0, 'R'},
			{"dual", 0, 0, '2'},
			{"quad", 0, 0, '4'},
			{NULL, 0, 0, 0},
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR24", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		case '2':
			mode |= SPI_TX_DUAL;
			break;
		case '4':
			mode |= SPI_TX_QUAD;
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
	if (mode & SPI_LOOP) {
		if (mode & SPI_TX_DUAL)
			mode |= SPI_RX_DUAL;
		if (mode & SPI_TX_QUAD)
			mode |= SPI_RX_QUAD;
	}
}

/*
int SPI_Write(uint8_t * TxBuf, int len)
{
	int ret;
	int fd = g_SPI_Fd;

	ret = write(fd, TxBuf, len);
	if (ret < 0)
		printf("SPI Write errorn\n");
	else {
#ifdef SPI_DEBUG
		int i;
		printf("SPI Write [Len:%d]: \n", len);
		for (i = 0; i < len; i++) {
			if (i % 8 == 0)
				printf("\n\t");
			printf("0x%02X ", TxBuf[i]);
		}
		printf("\n");

#endif
	}

	return ret;
}

int SPI_Read(uint8_t * RxBuf, int len)
{
	int ret;
	int fd = g_SPI_Fd;
	ret = read(fd, RxBuf, len);
	if (ret < 0)
		printf("SPI Read errorn\n");
	else {
#ifdef SPI_DEBUG
		int i;
		printf("\nSPI Read [len:%d]:\n", len);
		for (i = 0; i < len; i++) {
			if (i % 8 == 0)
				printf("\n\t");
			printf("0x%02X ", RxBuf[i]);
		}
		printf("\n");
#endif
	}

	return ret;
}
*/
int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;

	parse_opts(argc, argv);

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	g_SPI_Fd = fd;
	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE32, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: 0x%x\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed / 1000);

	//transfer(fd);

	cmd_send(fd);

	while (1) {
		//cmd_send(fd);

		transfer_recv(fd);

		sleep(1);
		//transfer_send(fd);
		//transfer_recv(fd);
		//usleep(1000);
		//printf("----------------1---------------\n");
	}
	printf("----------------1---------------\n");

/*	uint8_t data[] = {
		0x20, 0x03, 0x1e
	};

	printf("----------------2---------------\n");
	uint8_t send_cmd[3]= {0x20, 0x06, 0x7e};
	uint8_t recv[4] = {0};

	SPI_Write((uint8_t *)&send_cmd, 3);
	sleep(1);

	printf("----------------3---------------\n");

	uint8_t read_cmd = 0x22;

	SPI_Write((uint8_t *)&read_cmd, 1);
	SPI_Read(recv, 2);*/


	sleep(1);


	close(fd);

	return ret;
}
