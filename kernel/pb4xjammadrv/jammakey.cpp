


#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <linux/fb.h>
#include <SDL/SDL.h>
//#include "games.h"


#define PAGE_SIZE (sysconf( _SC_PAGESIZE))

#define LOG_INFO(fmt, args...) do {  fprintf(stdout, "[%s:%d]" fmt "\n", __FILE__, __LINE__, ##args ); fflush(stdout);} while (0)

//int WINDOW_WIDTH = 800;
//int WINDOW_HEIGHT = 600;
//const char* WINDOW_TITLE = "game select menu";

unsigned int gpio_reg =0xffffffff; 
void* mem = NULL;

int initmem()
{
  //The O_SYNC option prevents Linux from caching the contents of /dev/mem
  int fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (fd<0) { fprintf (stderr, "open /dev/mem faild : %s \n", strerror(errno)); exit(-2);  }

  unsigned int reg = 0x01c20800; 

  unsigned int dev_base = reg & (~(PAGE_SIZE-1));
  unsigned int offset   = reg & ( (PAGE_SIZE-1));


  // Map one page of memory into user space such that the device is in that page, but  
  //it    may not
  // be at the start of the page.
  mem = mmap(NULL,
      PAGE_SIZE,
      PROT_READ,
      MAP_SHARED,  //File may not be updated until msync or munmap is // called.
      fd,
      dev_base);  //How to know the offset?
  if (mem == MAP_FAILED) {  fprintf (stderr, "map failed %08x %s \n", dev_base, strerror(errno)); close(fd); exit(-2); }

  gpio_reg = ((unsigned int)mem)+offset;
  return 0;
}

int uninitmem()
{
  munmap(mem, PAGE_SIZE);
  return 0;
}

int readKey(int key_code)
{
  int port = (key_code>>5) & 0xf;
  int bit = (key_code>>0) & 0x1f;
  unsigned int regData =  *((volatile unsigned int *) (gpio_reg+0x10+0x24*port));
  
  return   (regData>>bit)&0x1;
}


struct timeval last_key_tv;
long long timeval_diff(struct timeval *difference,
    struct timeval *end_time,
    struct timeval *start_time
    )
{
  struct timeval temp_diff;

  if(difference==NULL)
  {
    difference=&temp_diff;
  }

  difference->tv_sec =end_time->tv_sec -start_time->tv_sec ;
  difference->tv_usec=end_time->tv_usec-start_time->tv_usec;

  /* Using while instead of if below makes the code slightly more robust. */

  while(difference->tv_usec<0)
  {
    difference->tv_usec+=1000000;
    difference->tv_sec -=1;
  }

  return 1000000LL*difference->tv_sec+
    difference->tv_usec;

} /* timeval_diff() */

bool checkTimeSleep(int i)
{
  if(i<=0) 
  {
    gettimeofday(&last_key_tv, NULL);
    return false;
  }
  else
  {
    struct timeval current_tv;
    gettimeofday(&current_tv, NULL);
    if(timeval_diff(NULL, &current_tv, &last_key_tv)>i*1000l) return true;
    else return false;
  }
}


