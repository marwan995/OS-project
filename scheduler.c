#include "headers.h"
int msgq_id;
// seif atef samy emad 
//marwan samy
int Recived_Config(int *quantum,int*numOfProcess)
{
    key_t key_id;
    key_id = ftok("keyfile", 65);               // create unique key
    msgq_id = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id
    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    
    Config c;
    int rec_val = msgrcv(msgq_id, &c, sizeof(c.Schedule),0, !IPC_NOWAIT); // 0

    if (rec_val == -1)
        perror("Error in receive");
    // else
    //     printf("\nMessage received: %d %d\n", c.Schedule[0], c.Schedule[1]);
    *quantum = c.Schedule[1];
    *numOfProcess=c.Schedule[2];
    return c.Schedule[0];
}
Process Recived_Process(int *priority)
{
  //  printf("\nreciving\n");
    msgbuff m;
    int send_val = msgrcv(msgq_id, &m, sizeof(m.p), 4, !IPC_NOWAIT);
   
    *priority = m.p.Priority;
    printf("Recived process with id %d time %d\n",m.p.Id,getClk());

    return m.p;
}
void Non_preemptive_Highest_Priority_First(Node **Process_queue)
{
    int pid, status, sid;
    while (!isEmpty(&(*Process_queue)))
    {
        Process running = dequeue(&(*Process_queue));

        char argString1[10];
        sprintf(argString1, " %d", running.Run_Time);
        char argString2[10];
        sprintf(argString2, " %d", running.Id);
        pid = fork();
        if (pid == 0)
        {
            execl("./process.out", "process.out", argString1, argString2, NULL);
            perror("erorr");
            return;
        }
        if ((sid = wait(&status)) > 0)
        {
           // printf("\nexit code %d\n", status >> 8);
        }//must be there as they will work in parell if not
       
    }
}
int main(int argc, char *argv[])
{
    initClk();
    int quantum,numOfProcess;
    Recived_Config(&quantum,&numOfProcess);
    Node *Process_queue = NULL;
    int priority;
    //  TODO implement the scheduler :)
    // upon termination release the clock resources.

     Process Currunt_process=Recived_Process(&priority);
     enqueue(&Process_queue, Currunt_process, priority);
     int prev =Currunt_process.Arrive_Time;
     numOfProcess--;

    
    while (numOfProcess)
    { //handle when multi process came in the same time 
        if(prev==Currunt_process.Arrive_Time){
          while(prev==Currunt_process.Arrive_Time&&numOfProcess){
             prev= Currunt_process.Arrive_Time;
             Currunt_process = Recived_Process(&priority);
             enqueue(&Process_queue, Currunt_process, priority);
              numOfProcess--;
       }
        }
        else{
              prev= Currunt_process.Arrive_Time;
              Currunt_process = Recived_Process(&priority);
             enqueue(&Process_queue, Currunt_process, priority);
              numOfProcess--;
        }
            Non_preemptive_Highest_Priority_First(&Process_queue);
       
    }
    while (wait(&prev) > 0);
    
    destroyClk(true);
    return 0;
}
/*
 all process have same arrive time differnet priority
*/