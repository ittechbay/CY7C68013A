// gcc -g -lusb main.c

#include <stdio.h>

#include <usb.h>

#include <unistd.h>

/* the device's vendor and product id */

#define MY_VID 0x2222

#define MY_PID 0x5555



/* the device's endpoints */

#define EP_IN 0x82



#define BUF_SIZE 0x1000

usb_dev_handle *open_dev(void);

usb_dev_handle *dev = NULL; /* the device handle */

struct BUFFER

{

 int n;

 int m;

 char c[BUF_SIZE];

};

struct BUFFER buf;

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



void usbinit()

{

	int i;

  usb_init(); /* initialize the library */

  usb_find_busses(); /* find all busses */

  usb_find_devices(); /* find all connected devices */

  dev = open_dev();

  usb_set_configuration(dev, 1);

  usb_claim_interface(dev, 0);



  

  for(i=0;i<10;i++)

  {

   usb_control_msg(dev, 0x40, 0xeb, 0, 0, spi1[i], 4, 200);

   usleep(50000); 

  }

  for(i=0;i<10;i++)

  {

   usb_control_msg(dev, 0x40, 0xec, 0, 0, spi2[i], 4, 200);

   usleep(50000); 

  }

}

void usbdeinit()

{

  usb_release_interface(dev, 0);

  usb_close(dev);  

}

usb_dev_handle *open_dev(void)

{

  struct usb_bus *bus;

  struct usb_device *dev;



  for(bus = usb_get_busses(); bus; bus = bus->next) 

    {

      for(dev = bus->devices; dev; dev = dev->next) 

        {

          if(dev->descriptor.idVendor == MY_VID

             && dev->descriptor.idProduct == MY_PID)

            {

              return usb_open(dev);

            }

        }

    }

  return NULL;

}

void getdata()

{

     struct BUFFER *pbuf = &buf;
     

     

     pbuf->n=usb_bulk_read(dev, EP_IN, pbuf->c, BUF_SIZE, 1000); 



	 printf("usb_bulk_read %d: %02x %02x %02x %02x %02x",pbuf->n,  pbuf->c[0], pbuf->c[1], pbuf->c[2], pbuf->c[3]);

}



int main() {

	usbinit(); 

	getdata();

 

	return 0;

}



