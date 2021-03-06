// gcc -o read_usb -g  read_usb1.c -lusb-1.0 -lpthread -L/work/libusb1_pc/lib/ -I/work/libusb1_pc/include/libusb-1.0/
#include <stdio.h>
#include <unistd.h>
#include <libusb.h>

#define MY_VID 0x2222
#define MY_PID 0x5555
#define EP_IN 0x82
#define BUF_SIZE (1024*1024*512)
#define BLK_SIZE (1024*1024)
char buf[BUF_SIZE];
  //max2769 spi config
char spi1[10][64]={
	{0xA2,0x91,0x9A,0x30},//CONF1//{0xA2,0x91,0x9A,0x30},//CONF1
	{0x05,0x50,0x28,0xC1},//CONF2
	{0xEA,0xFE,0x1D,0xC2},//CONF3
	{0x9E,0xC0,0x00,0x83},//PLLCONF,{0x9E,0xC0,0x00,0x83},//PLLCONF
	{0x0C,0x00,0x08,0x04},//DIV,GPS:{0x0C,0x00,0x08,0x04},
	{0x00,0x00,0x07,0x05},//FDIV 
	{0x80,0x00,0x00,0x06},//STRM
	{0x10,0x06,0x1B,0x27},//CLK
	{0x1E,0x0F,0x40,0x18},//TEST1
	{0x14,0xC0,0x40,0x29}};//TEST2

  //max2769 spi config
char spi2[10][64]={
	{0xA2,0x91,0x9A,0x30},//CONF1
	{0x05,0x50,0x28,0xC1},//CONF2
	{0xEA,0xFE,0x1D,0xC2},//CONF3
	{0x9E,0xC0,0x00,0x83},//PLLCONF
	{0x0B,0xE4,0x08,0x04},//DIV,BD1:{0x0B,0xE4,0x08,0x04}
	{0x00,0x00,0x07,0x05},//FDIV,L1oíBD1:{0x00,0x00,0x07,0x05}
	{0x80,0x00,0x00,0x06},//STRM
	{0x10,0x06,0x1B,0x27},//CLK
	{0x1E,0x0F,0x40,0x18},//TEST1
	{0x14,0xC0,0x40,0x29}};//TEST2

int main() 
{
	int i;
	int size;
	libusb_device **devs;
	struct libusb_device_handle *devh = NULL;
	FILE *fp;
	time_t t1, t2;
	pid_t pid;

	
	libusb_init(NULL);
	libusb_get_device_list(NULL, &devs);
	devh = libusb_open_device_with_vid_pid(NULL, MY_VID, MY_PID);
	libusb_claim_interface(devh, 0);
	libusb_set_configuration(devh,1);
	
	for(i=0;i<10;i++)
	
	{
		libusb_control_transfer(devh,0x40, 0xeb, 0, 0, spi1[i], 4, 200);
		usleep(50000); 
	}
	
	for(i=0;i<10;i++)
	{
		libusb_control_transfer(devh,0x40, 0xec, 0, 0, spi2[i], 4, 200);
		usleep(50000); 
	}
	

	int offset = 0;
	while(1)
	{
		libusb_bulk_transfer(devh, EP_IN, buf+offset, BLK_SIZE, &size, 1000); 
		if (size < BLK_SIZE)
			printf("read size:%d\n", size);
		
		offset += size;
		printf("%d %d\n", size, offset/BLK_SIZE);
		if (offset >= BUF_SIZE)
			break;
	}
	printf("start writing....\n");
	int offset1 = 0;
	fp = fopen("usbdata.dat", "w+");
	while(1)
	{
		int size = fwrite(buf+offset1, 1, BLK_SIZE, fp);
		if (size < BLK_SIZE)
		{
			printf("fwrite error\n");
		}
		
		offset1 += size;
		printf("%d %d\n", size, offset1/BLK_SIZE);
		if (offset1 >= offset)
			break;
		
	}
	fclose(fp); 

	return 0;
}




