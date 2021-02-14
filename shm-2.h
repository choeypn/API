//Nat Choeypant
//Assignment 3

#ifndef SHARED_MEM_NAME
#define SHARED_MEM_NAME "SharedMem1"


//<Define an enum called StatusEnus with the enumerations "INVALID", "VALID" and "CONSUMED">
enum StatusEnus{INVALID,VALID,CONSUMED};

//<Define a typedef structure with the enum above and an "int" variable called "data">
typedef struct {
  enum StatusEnus status;
  int data;
} ShmData;

#endif
/*
Analysis:

After observing the IPC method from running server and client.
On this assignment, it didn't specify to use any timer, so it seems that
the client received with the minimum time. In this assignment,
I use StatusEnus status as a flag for each client to ping the server to receive
server's data,then it should be similar to assignment 2 where I created a time period
for each thread to run or wait with signal, and use multithread as clients that
would wait for signal from the server.
Hence, I think maybe I could use StatusEnus status as a flag for each thread (similar to client)
instead of using signaling and timer in assignment 2 to receive or wait for server's
data with the given time intervals because I think it is more efficient with IPC method.

*/
