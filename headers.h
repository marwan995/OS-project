#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "PQueue.h"
#include "tree.h"
#include <string.h>
#include <math.h>
typedef short bool;
#define true 1
#define false 0

#define SHKEY 300
#define msgbuff struct msgbuff
#define ProcessSignal  struct ProcessSignal 
#define PCB  struct PCB

///==============================
//don't mess with this variable//
int * shmaddr;                 //
//===============================

msgbuff
{
    long mtype;
    Process p;
};
ProcessSignal
{
    long mtype;
    int Remaining_Time;
};
PCB{
    char state[10];
    int Execution_time,
        Remaining_time,Arrival_Time
        ,Waiting_Time,PID,Finish_Time,
        Start_Time, Mem_Size;
};

int getClk()
{
    return *shmaddr;
}

int next_power_of_two(int n) {
    int p = ceil(log2(n));
    return pow(2, p);
}
/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}


/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}
