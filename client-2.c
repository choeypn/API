//Nat Choeypant
//Assignment 3

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "shm.h"

#define handle_error_en(en,msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char* argv[]){
  int retVal = 0;
  int length = sizeof(ShmData);
  off_t offset = 0;
  /*
  <Use the POSIX "shm_open" API to open file descriptor with
    "O_RDWR" options and the "0666" permissions>
  */
  int fd = shm_open(SHARED_MEM_NAME, O_RDWR, 0666);
	if(fd == -1){
		handle_error_en(fd,"shm_open");
	}

  //<Use the "mmap" API to memory map the file descriptor>
  ShmData * addr = mmap(NULL,length,PROT_READ | PROT_WRITE,MAP_SHARED,fd,offset);
  if (addr == MAP_FAILED){
    perror("mmap");
    close(fd);
    exit(1);
  }
  printf("[Client]: Waiting for valid data ...\n");

  while(addr->status != VALID)
    {
      sleep(1);
    }

  printf("[Client]: Received %d\n",addr->data);

  addr->status = CONSUMED;

  //<use the "munmap" API to unmap the pointer>
  munmap(addr,length);

  printf("[Client]: Client exiting...\n");

  return(retVal);

}
