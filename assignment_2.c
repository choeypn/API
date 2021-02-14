//Nat Choeypant
//CSCI 247 Assignment2

/* Analysis :
All threads seem to run with some spike (outside range of 100), but still
in the range of thread's time period
All threads run 5 times concurrently and they all seem to depends on each
individual's time period
The maximum jitter was 283 micro seconds in Thread[1]
The minimum jitter was 25 micro seconds in Thread[2]

output:
Thread[0] Timer Delta [1000088]us       Jitter[88]us
Thread[0] Timer Delta [999845]us       Jitter[-155]us
Thread[1] Timer Delta [2000061]us       Jitter[61]us
Thread[0] Timer Delta [999875]us       Jitter[-125]us
Thread[0] Timer Delta [999974]us       Jitter[-26]us
Thread[2] Timer Delta [4000136]us       Jitter[136]us
Thread[1] Timer Delta [1999856]us       Jitter[-144]us
Thread[0] Timer Delta [999873]us       Jitter[-127]us
Thread[1] Timer Delta [1999717]us       Jitter[-283]us
Thread[1] Timer Delta [2000088]us       Jitter[88]us
Thread[2] Timer Delta [3999896]us       Jitter[-104]us
Thread[1] Timer Delta [1999891]us       Jitter[-109]us
Thread[2] Timer Delta [3999803]us       Jitter[-197]us
Thread[2] Timer Delta [4000025]us       Jitter[25]us
Thread[2] Timer Delta [3999848]us       Jitter[-152]us

*/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<signal.h>
#include<time.h>
#include<unistd.h>
#include<errno.h>

#define NUM_THREADS 3 //number of threads
#define handle_error_en(en,msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

//struct a structure to store thread id, schdule policy and priority
typedef struct {
  pthread_t threadID;
  int threadNum;
  int policy;
  struct sched_param param;
  int signal_number;
  sigset_t timer_signal;
  int missed_signal_count;
  timer_t timerId;
  unsigned long long timer_Period; // in micro seconds
  unsigned long long timeStamp;
  unsigned int jitter;
} ThreadInfo;

ThreadInfo myThreadInfo[NUM_THREADS];

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER; // set the mutex


//display Thread attributes function
void DisplayThreadSchedAttributes(ThreadInfo * ThreadInfo){
  //display the attributes
  printf("Thread[%d] Timer Delta [%lld]us       Jitter[%d]us\n",ThreadInfo->threadNum,
  ThreadInfo->timeStamp,ThreadInfo->jitter);
}

void * CreateAndArmTimer(int unsigned period, ThreadInfo* threadInfo){
  //Create a static int variable to keep track of the next available signal number
  static int currentSignal = 0;
  unsigned long long nanoseconds, seconds;
  int ret;
  //Initialize the thread structure elements
  pthread_mutex_lock(&count_mutex);
  if(currentSignal == 0){
    currentSignal = SIGRTMIN;
  }
  //Assign the next available Real-time signal to thread “signal_number”
  threadInfo->signal_number = currentSignal;
  threadInfo->timer_Period = period;
  currentSignal++;
  pthread_mutex_unlock(&count_mutex);

  //Create the signal mask corresponding to the chosen signal_number in “timer_signal”
  //Use “sigemptyset” and “sigaddset” for this
  sigemptyset(&threadInfo->timer_signal);
  sigaddset(&threadInfo->timer_signal,threadInfo->signal_number);

  //Use timer_Create to create a timer – See code in background section
  struct sigevent mySignalEvent;
  mySignalEvent.sigev_notify = SIGEV_SIGNAL;
  mySignalEvent.sigev_signo = threadInfo->signal_number; //A value b/t SIGRTMIN and SIGRTMAX
  mySignalEvent.sigev_value.sival_ptr = (void *)&(threadInfo->timerId);
  ret = timer_create (CLOCK_MONOTONIC, &mySignalEvent, &threadInfo ->timerId);
  if(ret != 0){
    handle_error_en(ret,"timer_create");
  }

  //Arm Timer – See code in background section
  struct itimerspec timerSpec;
  seconds = period/1000000;
  nanoseconds = (period - (seconds * 1000000)) * 1000;
  timerSpec.it_interval.tv_sec = seconds;
  timerSpec.it_interval.tv_nsec = nanoseconds;
  timerSpec.it_value.tv_sec = seconds;
  timerSpec.it_value.tv_nsec = nanoseconds;
  ret = timer_settime (threadInfo->timerId, 0, & timerSpec, NULL);
  if(ret != 0){
    handle_error_en(ret,"timer_settime");
  }
  return 0;
}
void * WaitForTimer(ThreadInfo* threadInfo){
  //Use sigwait function to wait on the “timer_signal”
  int waitCheck = sigwait(&threadInfo->timer_signal,&threadInfo->signal_number);
  if(waitCheck != 0){
    handle_error_en(waitCheck,"sigwait");
  }
  //update missed_signal_count by calling “timer_getoverrun”
  threadInfo->missed_signal_count = timer_getoverrun(&threadInfo->timerId);
  return 0;
}

void * threadFunction(void* param){
  ThreadInfo* threadInfo = (ThreadInfo*) param;
  unsigned long microStart,microEnd;
  struct timespec tms;
  //run CreateAndArmTimer
  CreateAndArmTimer(threadInfo->timer_Period,threadInfo);
  //run forloop for WaitForTimer 5 times
  //display TimeStamp
  for(int i = 0; i < 5;i++){
    //start the timer
    clock_gettime(CLOCK_MONOTONIC,&tms);
    microStart = tms.tv_sec * 1000000;
    microStart += tms.tv_nsec / 1000;
    //run timer
    WaitForTimer(threadInfo);
    //get timer
    clock_gettime(CLOCK_MONOTONIC,&tms);
    microEnd = tms.tv_sec * 1000000;
    microEnd += tms.tv_nsec / 1000;
    //calculate timeStamp
    threadInfo->timeStamp = microEnd - microStart;
    //calculate jitter
    threadInfo->jitter = threadInfo->timeStamp - threadInfo->timer_Period;
    //print out the output
    DisplayThreadSchedAttributes(threadInfo);
  }
  return 0;
}

int main(){
  int fifoPri = 1;
  //lock signal
  sigset_t alarm_sig;
  sigemptyset (&alarm_sig);
  for(int i = SIGRTMIN; i <= SIGRTMAX;i++){
    int sigCheck = sigaddset(&alarm_sig,i);
    if(sigCheck != 0){
      handle_error_en(sigCheck,"sigaddset");
    }
  }
  sigprocmask (SIG_BLOCK,&alarm_sig,NULL);
  //create Threads and run
  for(int i = 0; i < NUM_THREADS;i++){
    int createCheck = pthread_create(&myThreadInfo[i].threadID,NULL,
      threadFunction,&myThreadInfo[i]);
    if(createCheck != 0){
      handle_error_en(createCheck,"pthread_create");
    }
    //set each thread time period and thread number
    if(i == 0){
      myThreadInfo[i].timer_Period = 1000000;
      myThreadInfo[i].threadNum = i;
    } else if (i == 1){
      myThreadInfo[i].timer_Period = 2000000;
      myThreadInfo[i].threadNum = i;
    } else {
      myThreadInfo[i].timer_Period = 4000000;
      myThreadInfo[i].threadNum = i;
    }

    //set all threads to FIFO with same priority
    myThreadInfo[i].param.sched_priority = fifoPri;
    int t = pthread_setschedparam(myThreadInfo[i].threadID,
    SCHED_FIFO,&myThreadInfo[i].param);
    if(t != 0){
      handle_error_en(t,"pthread_setschedparam");
    }
  }
  //join all the threads back
  for(int i = 0; i < NUM_THREADS;i++){
    int joinCheck = pthread_join(myThreadInfo[i].threadID,NULL);
    if(joinCheck != 0){
      handle_error_en(joinCheck,"pthread_join");
    }


  }
  return 0;
}
