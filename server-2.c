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
	//<Confirm argc is 2 and if not print a usage string.>
	if(argc != 2){
		printf("error, please input 2 arguments");
	}
  //<Use the POSIX "shm_open" API to open file descriptor with
	//"O_CREAT | O_RDWR" options and the "0666" permissions>
  int fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666);
	if(fd == -1){
		handle_error_en(fd,"shm_open");
	}
  //<Use the "ftruncate" API to set the size to the size of your
  //structure shm.h>
	int ret = ftruncate(fd,length);
	if(ret == -1){
		handle_error_en(ret,"ftruncate");
	}
  //<Use the "mmap" API to memory map the file descriptor>
	ShmData * addr = mmap(NULL,length,PROT_READ | PROT_WRITE,MAP_SHARED,fd,offset);
  if (addr == MAP_FAILED){
    perror("mmap");
    close(fd);
    exit(1);
  }
  //<Set the "status" field to INVALID>
	addr->status = INVALID;
  //<Set the "data" field to atoi(argv[1])>
	addr->data = atoi(argv[1]);
  //<Set the "status" field to VALID>
	addr->status = VALID;

  printf("[Server]: Server data Valid... waiting for client\n");

  while(addr->status != CONSUMED)
    {
      sleep(1);
    }

  printf("[Server]: Server Data consumed!\n");

  //<use the "munmap" API to unmap the pointer>
  ret = munmap(addr,length);
  if(ret == -1){
    handle_error_en(ret,"munmap");
  }
  //<use the "close" API to close the file Descriptor>
  ret = close(fd);
  if(ret == -1){
    handle_error_en(ret,"close");
  }
  //<use the "shm_unlink" API to revert the shm_open call above>
  ret = shm_unlink(SHARED_MEM_NAME);
  if(ret == -1){
    handle_error_en(ret,"shm_unlink");
  }
  printf("[Server]: Server exiting...\n");


  return(retVal);

}
