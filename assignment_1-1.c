//Nat Choeypant
//CSCI 247 Assignment1

/* Analysis :
I used Linux OS X64 bits to test my code.
compile line: gcc assignment_1 assignment_1.c -lpthread
execute line: sudo taskset 0x8 ./assignment_1

After all threads had been run, it seems that thre first thread that gets to run
is SCHED_FIFO with the lowest priority within SCHED_FIFO,
then the second thread SCHED_RR with the lowest priority within SCHED_RR,
then the third thread with SCHED_FIFO with the second lowest priority gets to run...
This goes on until threads with SCHED_FIFO and SCHED_RR are done, then threads
with SCHED_OTHER gets to run last.

I print out the display of task end TimeStamp after each task is done, then after
all 3 tasks are done, it output the endtime.

The maximum number of threads that can run simultaneously is one because
I didn't make all threads run simultaneously.
I lock all threads but one with the first priority then unlock all when it is done.

All threads have different addresses since only one thread gets to run at a time
Each task takes around 6 seconds, all three tasks runtime total of 17 seconds.

*/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>

#define NUM_THREADS 9 //number of threads

//struct a structure to store thread id, schdule policy and priority
typedef struct {
  pthread_t threadID;
  int policy;
  struct sched_param param;

} ThreadInfo;

ThreadInfo myThreadInfo[NUM_THREADS];

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER; // set the mutex


int threadNumber = 1; //variable to display thread number

//display Thread attributes function
void DisplayThreadSchedAttributes(void){
  int policy;
  struct sched_param param;
  time_t currentTime; //get the current time
  time(&currentTime);
  pthread_getschedparam(pthread_self(),&policy,&param);
  //display the attributes
  printf("DisplayThreadSchedAttributes:\n threadID = 0x%lx\n policy = %s\n priority = %d\n StartTime = %s",
  pthread_self(),
  (policy == SCHED_FIFO)?"SCHED_FIFO" :
  (policy == SCHED_RR)?"SCHED_RR" :
  (policy == SCHED_OTHER)?"SCHED_OTHER" :
  "???",
  param.sched_priority,ctime(&currentTime));
}

//function for thread runner
void * countFunction(){
  unsigned long limit = 2500000000,count = 0; // variables for task
  unsigned long long microStart, microEnd, delta; // variables for TimeStamp
  pthread_mutex_lock(&count_mutex); // lock akk the threads except the running thread
  printf("Thread# %i\n",threadNumber); // print thread number
  threadNumber++;
  DisplayThreadSchedAttributes(); // display the attributes
  struct timespec tms;
  clock_gettime(CLOCK_REALTIME,&tms);
  microStart = tms.tv_sec * 1000000;
  microStart += tms.tv_nsec / 1000;
  printf(" Task start TimeStamp in micro seconds %lld\n",microStart); // print the time stamp
  for(int k = 0; k < 3;k++){ //run the thread 3 times
    for (long i = 0; i <= limit;i++){
      count +=i;
    }
    clock_gettime(CLOCK_REALTIME,&tms);
    microEnd = tms.tv_sec * 1000000;
    microEnd += tms.tv_nsec / 1000;
    delta = microEnd - microStart; // calculate delta
    microStart = microEnd; // update micro start for the next task
    printf(" Task %i end TimeStamp in micro seconds %lld  Delta %lld\n",k+1,microEnd,delta); // print time stamp each time it is completed
  }
  time_t endTime;
  time(&endTime);
  printf(" EndTime = %s",ctime(&endTime)); //print the endtime

  printf("\n");
  pthread_mutex_unlock(&count_mutex); // unlock the threads
  pthread_exit(0); //exit the thread after it is done
}

int main(){
  int fifoPri = 60; //variables for FIFO and RR priority
  int rrPri = 30;
  printf("running...\n");
  //create Threads, run, and store threadinfo into ThreadInfo
  for(int i = 0; i < NUM_THREADS;i++){
    pthread_create(&myThreadInfo[i].threadID,NULL,countFunction,&myThreadInfo[i]);

    if(i % 3 == 0){ //set 3 threads to FIFO with different priority
      myThreadInfo[i].policy = SCHED_FIFO;
      myThreadInfo[i].param.sched_priority = fifoPri + i;
    } else if ( i % 3 == 1){ //set 3 threads to RR with different priority
      myThreadInfo[i].policy = SCHED_RR;
      myThreadInfo[i].param.sched_priority = rrPri + i;
    } else { //set 3 threads to OTHER
      myThreadInfo[i].policy = SCHED_OTHER;
      myThreadInfo[i].param.sched_priority = 0;
    }
    //set threads to the given policy and priority
    pthread_setschedparam(myThreadInfo[i].threadID, myThreadInfo[i].policy, &myThreadInfo[i].param);
  }

  for(int i = 0; i < NUM_THREADS;i++){ //join all the threads back
     pthread_join(myThreadInfo[i].threadID,NULL);

  }
  return 0;
}
