#include "headers.h"
int msgq_id, chosen;
key_t key_id;

int Recv_Signal()
{
    key_t process_key = ftok("pidfile", 75);               // create unique key
    int process_msgq_id = msgget(process_key, 0644 | IPC_CREAT); // create message queue and return id
    Config to_recv;
    //sleep(1);
    int rec_val = msgrcv(process_msgq_id, &to_recv, sizeof(to_recv.Schedule), 65, !IPC_NOWAIT); // 0
    
    if (rec_val == -1)
        perror("reciving");
    return to_recv.Schedule[0];
}
int Recived_Config(int *quantum, int *numOfProcess)
{
    key_id = ftok("keyfile", 65);               // create unique key
    msgq_id = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id
    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    Config c;
    int rec_val = msgrcv(msgq_id, &c, sizeof(c.Schedule), 1, !IPC_NOWAIT); // 0
    if (rec_val == -1)
        perror("Error in receive");
    // else
    //     printf("\nMessage received: %d %d\n", c.Schedule[0], c.Schedule[1]);
    *quantum = c.Schedule[1];
    *numOfProcess = c.Schedule[2];
    return c.Schedule[0];
}
int dbg_cnt = 0;
Process Recived_Process(int *priority)
{
    //  printf("\nreciving\n");
    msgbuff m;
    int recv_val = msgrcv(msgq_id, &m, sizeof(m.p), 4, IPC_NOWAIT);
    //printf("Recived_Process number : %d\n", dbg_cnt);
    // dbgcnt++;
    if (chosen != 3)
        *priority = m.p.Priority;
    else
        *priority = 0;
    if (recv_val == -1)
        m.p.Arrive_Time = -1;
    else
        printf("Recived process with id %d time %d\n", m.p.Id, getClk());
    return m.p;
}
void Non_preemptive_Highest_Priority_First(Node **Process_queue)
{
    int pid, status, sid;
    while (!isEmpty(&(*Process_queue)))
    {
        Process running = dequeue(&(*Process_queue));
        char argString1[10];
        sprintf(argString1, " %d", running.Remaining_Time);
        char argString2[10];
        sprintf(argString2, " %d", running.Id);
        char argString3[10];
        sprintf(argString3, " %d", chosen);    
        char argString4[10]; 
        sprintf(argString4, " %d", 10000000);
        pid = fork();
        if (pid == 0)
        {
            execl("./process.out", "process.out", argString1, argString2, argString3, argString4, NULL);
            perror("erorr");
            return;
        }
        if ((sid = wait(&status)) > 0)
        {
            // printf("\nexit code %d\n", status >> 8);
        } // must be there as they will work in parell if not
    }
}
void Shortest_Remaining_time_Next(Node **Process_queue)
{
}
Process Round_Robin(Node **Process_queue, int quantum)
{
    int pid, status, sid;
    int current_clk = getClk();
    if (!isEmpty(&(*Process_queue)))
    {
        Process running = dequeue(&(*Process_queue));
        char argString1[10];
        sprintf(argString1, " %d", running.Remaining_Time);
        char argString2[10];
        sprintf(argString2, " %d", running.Id);
        char argString3[10];
        sprintf(argString3, " %d", chosen);    
        char argString4[10]; 
        sprintf(argString4, " %d", quantum);
       // printf("before send RT: %d id: %d chosen:%d  quantum:%d\n", running.Remaining_Time, running.Id, chosen, quantum);

        int remaining = running.Remaining_Time;
        pid = fork();
        if (pid == 0)
        {
            execl("./process.out", "process.out", argString1, argString2, argString3, argString4, NULL);
            perror("Erorr");
            exit(-1);
        }

        running.Remaining_Time = Recv_Signal();
      
        if (running.Remaining_Time > 0)
            return running;
        printQueue(&(*Process_queue));
        
        if(getClk()>=30)
            kill(getpgrp(),SIGINT);
        // printf("Hello in RR (if) : %d with queue size :%d\n", running.Id, size);
    }
}
int main(int argc, char *argv[])
{
    initClk();
    int quantum, numOfProcess;
    chosen = Recived_Config(&quantum, &numOfProcess);
    Node *Process_queue = NULL;
    int priority;
    //  TODO implement the scheduler :)
    // upon termination release the clock resources.
    printf("\n%d\n", chosen);
    Process Currunt_process;
    printf("numOfProcess = %d\n", numOfProcess);

    Process non_finished_process;
    non_finished_process.Id = -1;
    while (numOfProcess > 0 || !isEmpty(&Process_queue) || non_finished_process.Id != -1)
    { // handle when multi process came in the same time
        Currunt_process = Recived_Process(&priority);
        while (Currunt_process.Arrive_Time != -1)
        {
            printf("Id = %d\n", Currunt_process.Id);
            enqueue(&Process_queue, Currunt_process, priority);
            numOfProcess--;
            Currunt_process = Recived_Process(&priority);
        }
        if(non_finished_process.Id != -1){
            enqueue(&Process_queue, non_finished_process, priority);
            non_finished_process.Id = -1;
        }
        if (chosen == 1)
            Non_preemptive_Highest_Priority_First(&Process_queue);
        else if (chosen == 2)
            Shortest_Remaining_time_Next(&Process_queue);
        else if (chosen == 3)
            non_finished_process = Round_Robin(&Process_queue, quantum);
    }
    while (wait(&quantum) > 0);        ;

    destroyClk(true);
    return 0;
}
/*
 all process have same arrive time differnet priority
*/