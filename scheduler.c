#include "headers.h"
int msgq_id, chosen;
key_t key_id;

int Recv_Signal()
{
    key_t process_key = ftok("pidfile", 75);               // create unique key
    int process_msgq_id = msgget(process_key, 0644 | IPC_CREAT); // create message queue and return id
    Config to_recv;
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
    *quantum = c.Schedule[1];
    *numOfProcess = c.Schedule[2];
    return c.Schedule[0];
}

Process Recived_Process(int *priority)
{
    msgbuff m;
    int recv_val = msgrcv(msgq_id, &m, sizeof(m.p), 4, IPC_NOWAIT);
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

void intToStrArray(int num1, int num2, int num3, int num4,char strArr [4][10]) {
    sprintf(strArr[0], "%d", num1);
    sprintf(strArr[1], "%d", num2);
    sprintf(strArr[2], "%d", num3);
    sprintf(strArr[3], "%d", num4);

}
void Non_preemptive_Highest_Priority_First(Node **Process_queue)
{
    int pid, status, sid;
    char Args [4][10];
    while (!isEmpty(&(*Process_queue)))
    {
        Process running = dequeue(&(*Process_queue));
        intToStrArray(running.Remaining_Time,running.Id,chosen,10000,Args);
        pid = fork();
        if (pid == 0)
        {
            execl("./process.out", "process.out",Args[0], Args[1], Args[2], Args[3], NULL);
            perror("erorr");
            return;
        }
        if ((sid = wait(&status)) > 0);
    }
}

Process Shortest_Remaining_time_Next(Node **Process_queue)
{
    int pid, quantum = 1;
    char Args [4][10];
    int current_clk = getClk();
    if (!isEmpty(&(*Process_queue)))
    {
        Process running = dequeue(&(*Process_queue));
        intToStrArray(running.Remaining_Time,running.Id,chosen,quantum,Args);
        int remaining = running.Remaining_Time;
        pid = fork();
        if (pid == 0)
        {
            execl("./process.out", "process.out", Args[0], Args[1], Args[2], Args[3], NULL);
            perror("Erorr");
        }

        running.Remaining_Time = Recv_Signal();
      
        if (running.Remaining_Time > 0)
            return running;
        printQueue(&(*Process_queue));
          
    }
    Process dummy;
    dummy.Id=-1;
    dummy.Arrive_Time=-1;
    return dummy;
}

Process Round_Robin(Node **Process_queue, int quantum)
{
    int pid;
    char Args [4][10];
    int current_clk = getClk();
    if (!isEmpty(&(*Process_queue)))
    {
        Process running = dequeue(&(*Process_queue));
        intToStrArray(running.Remaining_Time,running.Id,chosen,quantum,Args);
        int remaining = running.Remaining_Time;
        pid = fork();
        if (pid == 0)
        {
            execl("./process.out", "process.out", Args[0], Args[1], Args[2], Args[3], NULL);
            perror("Erorr");
            //exit(-1);
        }
        running.Remaining_Time = Recv_Signal();
      
        if (running.Remaining_Time > 0)
            return running;
    }
    Process dummy;
    dummy.Id=-1;
    dummy.Arrive_Time=-1;
    return dummy;
}
int main(int argc, char *argv[])
{
    initClk();
    int quantum, numOfProcess,priority = 0;
    chosen = Recived_Config(&quantum, &numOfProcess);
    Node *Process_queue = NULL;

    //  TODO implement the scheduler :)
    // upon termination release the clock resources.
    Process non_finished_process,Currunt_process;
    non_finished_process.Id = -1;
    while (numOfProcess > 0 || !isEmpty(&Process_queue) || non_finished_process.Id != -1)
    { // handle when multi process came in the same time
        Currunt_process = Recived_Process(&priority);
        while (Currunt_process.Arrive_Time != -1)
        {
            if(chosen == 2){ // SRTN priority
                priority = Currunt_process.Remaining_Time;
            }
            printf("Id = %d\n", Currunt_process.Id);
            enqueue(&Process_queue, Currunt_process, priority);
            numOfProcess--;
            Currunt_process = Recived_Process(&priority);
        }
        if(non_finished_process.Id != -1){
            if(chosen == 2){ // SRTN priority
                priority = non_finished_process.Remaining_Time;
            }
            else priority=0;
            enqueue(&Process_queue, non_finished_process, priority);
            non_finished_process.Id = -1;
        }
        if (chosen == 1)
            Non_preemptive_Highest_Priority_First(&Process_queue);
        else if (chosen == 2)
            non_finished_process = Shortest_Remaining_time_Next(&Process_queue);
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