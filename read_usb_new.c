// gcc -g -lusb main.c
#include <stdio.h>
#include <unistd.h>
#include <libusb.h>
#include <pthread.h>

#define NUM_THREADS 2

#define MY_VID 0x2222
#define MY_PID 0x5555
#define EP_IN 0x82
#define BUF_SIZE (1024*1024)
#define READ_TIME (60*1)

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

/* create thread argument struct for thr_func() */
typedef struct _thread_data_t {
  FILE *fp;
  struct libusb_device_handle *devh;
  int write_ready;
  int read_ready;
  unsigned char buf_write[BUF_SIZE];	
  unsigned char buf_read[BUF_SIZE];
  time_t time;
} thread_data_t;


 
thread_data_t thdata; 

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_write = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_read = PTHREAD_COND_INITIALIZER;

int write_index = 0;
void *thr_write(void *arg) 
{

	thread_data_t *td = (thread_data_t *)arg;

	while(1)
	{
		/* thread code blocks here until MAX_COUNT is reached */
		pthread_mutex_lock(&lock);
		while(1)
		{
			if (td->write_ready == 0) 
			{
				pthread_cond_wait(&cond_write, &lock);
	
			}
			else
				break;
		}
		memcpy(td->buf_write, td->buf_read, BUF_SIZE);
		td->read_ready = 1;
		pthread_cond_signal(&cond_read);
		pthread_mutex_unlock(&lock);
		int ret = fwrite(td->buf_write, 1, BUF_SIZE, td->fp);
		if (ret != BUF_SIZE)
		{
			printf("fwrite:%d\n", ret);
			return;
		}

		pthread_mutex_lock(&lock);
		printf("w:%d\n", ++write_index);
		td->write_ready = 0;
		pthread_mutex_unlock(&lock);
		/* proceed with thread execution */
		if (time(NULL)-td->time == READ_TIME)
		{
			fclose(td->fp);
			exit(0);
		}
	}
}
 
/* some other thread code that signals a waiting thread that MAX_COUNT has been reached */
int read_index = 0;
void *thr_read(void *arg) 
{
	thread_data_t *td = (thread_data_t *)arg;
	int read_size;


	while (1)
	{
			pthread_mutex_lock(&lock);
			/* some code here that does interesting stuff and modifies count */
			while(1)
			{
				if (td->read_ready == 0)
				{
				pthread_cond_wait(&cond_read, &lock);
				}
				else
					break;
			}
			pthread_mutex_unlock(&lock);
			
			libusb_bulk_transfer(td->devh, EP_IN, td->buf_read, BUF_SIZE, &read_size, 1000); 
			if (read_size != BUF_SIZE)
			{
				printf("read_size:%d\n", read_size);
				return;
			}
			
			pthread_mutex_lock(&lock);
			printf("r:%d\n", ++read_index);
			td->write_ready = 1;
			td->read_ready = 0;
			pthread_mutex_unlock(&lock);
			pthread_cond_signal(&cond_write);

			
			if (time(NULL)-td->time == READ_TIME)
			{
				exit(0);
			}
	}
} 



int main() 
{
	int i;
	int size = 10;
	libusb_device **devs;
	struct libusb_device_handle *devh = NULL;
	time_t t1, t2;
	pthread_t thr[2];
	int rc;

	
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

	thdata.time = time(NULL);
	thdata.write_ready = 0;
	thdata.read_ready = 1;
	thdata.devh = devh;
	thdata.fp = fopen("usbdada.data", "w+");
	
	rc = pthread_create(&thr[0], NULL, thr_write, &thdata);
	if (rc != 0)
	{
		fprintf(stderr, "error: pthread_create thr_func0, rc: %d\n", rc);
		return 1;
	}
	rc = pthread_create(&thr[1], NULL, thr_read, &thdata);
	if (rc != 0)
	{
		fprintf(stderr, "error: pthread_create thr_func1, rc: %d\n", rc);
		return 1;
	}


	
	pthread_join(thr[0], NULL);
	pthread_join(thr[1], NULL);
	 

	return 0;
}




