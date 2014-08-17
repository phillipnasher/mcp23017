#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>

#define I2C_DEV_0 "/dev/i2c-0"
#define I2C_ADDR 0x43

#define SIZE 2

/*LEDs are connected to port B*/

#define PORT_A 0x00
#define PORT_B 0x01
#define PORT_A_LATCH 0x14
#define PORT_B_LATCH 0x15

#define PORT_DIR 0x00

/*
	Initialize the direction of the ports.

*/
int init_port(const int fd,const char port, const char dir, char *latch)
{
	char data[2];

	data[0] = port;
	data[1] = dir;

	if (port == PORT_A) {
		*latch = PORT_A_LATCH;
	} else {
		*latch = PORT_B_LATCH;
	}

	return write(fd,data,2);
}

int init_device(int *fd)
{
	if ((*fd = open(I2C_DEV_0,O_RDWR)) == -1) {
		printf("Unable to open device\n");
		return EXIT_FAILURE;
	}

	if (ioctl(*fd,I2C_SLAVE,I2C_ADDR) < 0) {
		printf("Unable to obtain address.\n");
		close(*fd);
	}
}


void cycle_ports(const int fd, const int delay, const int latch)
{
	char data[1];

	data[1] = 0x01;

	while (!(data[1] & 0b10000000)) {
		data[0] = latch;
		write(fd,data,SIZE);
		data[1] <<= 0x01;
		usleep(delay);
	}

	while (!(data[1] & 0b00000001)) {
		data[0] = latch;
		write(fd,data,SIZE);
		data[1] >>= 0x01;
		usleep(delay);
	}

}

int main(void)
{
	int fd;

	init_device(&fd);
	char latch;

	char data[2];
	data[1] = 0x01;

	while (1) {

		if (init_port(fd,PORT_A,PORT_DIR,&latch) != 2) {
			printf("Write error: Unable to configure device\n");
		}

		cycle_ports(fd, 10000, latch);

		if (init_port(fd,PORT_B,PORT_DIR,&latch) != 2) {
			printf("Write error: Unable to configure device\n");
		}

		cycle_ports(fd, 10000, latch);

	}
	close(fd);
	return EXIT_SUCCESS;
}
