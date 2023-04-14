#include "headers.h"

int msgq_id, chosen;
key_t key_id;
PCB *Process_control;
int Recv_Signal()
{
    key_t process_key = ftok("pidfile", 75);                     // create unique key
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
void New_File()
{

    FILE *fp;
    char str[] = "#At time x process y state arr w total z remain y wait k\n";
    fp = fopen("scheduler.log", "w");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return;
    }
    fputs(str, fp);
    fclose(fp);
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
void add_to_PCB(Process p)
{
    int index = p.Id - 1;
    Process_control[index].PID = p.Id;
    strcpy(Process_control[index].state, "started");
    Process_control[index].Arrival_Time = p.Arrive_Time;
    Process_control[index].Start_Time = getClk();
    Process_control[index].Waiting_Time = getClk() - p.Arrive_Time;
    Process_control[index].Remaining_time = p.Remaining_Time;
    Process_control[index].Execution_time = p.Run_Time;
    Process_control[index].Finish_Time = 0;
}

int Write_to_schedulerLog(int wait, int id, int remain)
{
    FILE *fp;
    char line[200];
    char state[50];
    strcpy(state,Process_control[id-1].state);
    if (strcmp(state,"finished")!=0)
        sprintf(line, "At time %d process %d %s arr %d total %d remain %d wait %d\n", getClk(), id, state, Process_control[id - 1].Arrival_Time, Process_control[id - 1].Execution_time, remain, wait);
    else
    {
        int TA = Process_control[id - 1].Finish_Time - Process_control[id - 1].Arrival_Time;
        float WTA = (float)TA / Process_control[id - 1].Execution_time;
        sprintf(line, "At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), id, state, Process_control[id - 1].Arrival_Time, Process_control[id - 1].Execution_time, remain, wait, TA, WTA);
    }
    fp = fopen("scheduler.log", "a");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return 0;
    }

    fprintf(fp, line);
    fclose(fp);
    return 1;
}
void intToStrArray(int num1, int num2, int num3, int num4, char strArr[4][10])
{
    sprintf(strArr[0], "%d", num1);
    sprintf(strArr[1], "%d", num2);
    sprintf(strArr[2], "%d", num3);
    sprintf(strArr[3], "%d", num4);
}
void Non_preemptive_Highest_Priority_First(Node **Process_queue)
{
    int pid, status, sid;
    char Args[4][10];
    while (!isEmpty(&(*Process_queue)))
    {
        Process running = dequeue(&(*Process_queue));
        add_to_PCB(running);

        intToStrArray(running.Remaining_Time, running.Id, chosen, 10000, Args);
        Write_to_schedulerLog(Process_control[running.Id - 1].Waiting_Time, running.Id, running.Remaining_Time);
        pid = fork();
        if (pid == 0)
        {
            execl("./process.out", "process.out", Args[0], Args[1], Args[2], Args[3], NULL);
            perror("erorr");
            return;
        }
        if ((sid = wait(&status)) > 0)
            ;
        Process_control[running.Id - 1].Finish_Time = getClk();
        strcpy(Process_control[running.Id-1].state,"finished");
        Write_to_schedulerLog( Process_control[running.Id - 1].Waiting_Time, running.Id, 0);
    }
}
void scheduler_perf(int size)
{
    int Worktime = 0, WaitTime = 0, TA = 0;
    float WTA = 0.0;
    for (int i = 0; i < size; i++)
    {
        Worktime += Process_control[i].Execution_time;

        WaitTime += Process_control[i].Finish_Time - Process_control[i].Arrival_Time - Process_control[i].Execution_time;
        printf("WaitTime=%d\n", WaitTime);
        WTA += (float)(Process_control[i].Finish_Time - Process_control[i].Arrival_Time) / Process_control[i].Execution_time;
    }

    FILE *fp;
    char line[200];
    fp = fopen("scheduler.pref", "w");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return;
    }
    float ut = ((float)Worktime / getClk()) * 100;
    printf("clk:%d worktime:%d ut=%f\n", getClk(), Worktime, ut);
    float sum = 0, AvgWTA = (float)WTA / size;
    for (int i = 0; i < size; i++)
    {
        sum += pow(AvgWTA - ((float)(Process_control[i].Finish_Time - Process_control[i].Arrival_Time) / Process_control[i].Execution_time), 2);
    }
    float std = sqrt(sum / size);

    sprintf(line, "CPU utilization = %.2f\%\nAvg WTA = %.2f\nAvg Waiting = %.2f\nStd WTA = %.2f\n", ut, WTA / size, (float)WaitTime / size, std);

    fputs(line, fp);
    fclose(fp);
}
Process Round_Robin(Node **Process_queue, int quantum)
{
    int pid;
    char Args[4][10];
    int current_clk = getClk();
    if (!isEmpty(&(*Process_queue)))
    {
        Process running = dequeue(&(*Process_queue));
        intToStrArray(running.Remaining_Time, running.Id, chosen, quantum, Args);
        if (running.Run_Time == running.Remaining_Time){
            add_to_PCB(running);
            strcpy(Process_control[running.Id-1].state,"started");
            Write_to_schedulerLog( Process_control[running.Id - 1].Waiting_Time, running.Id, running.Remaining_Time);
        }
        else{
            int arr_time = Process_control[running.Id - 1].Arrival_Time;
            int exec_time = Process_control[running.Id - 1].Execution_time;
            int rem_time = Process_control[running.Id - 1].Remaining_time;
            Process_control[running.Id - 1].Waiting_Time = getClk() - arr_time - (exec_time - rem_time);
            strcpy(Process_control[running.Id-1].state,"resumed");
            Write_to_schedulerLog(Process_control[running.Id - 1].Waiting_Time, running.Id, running.Remaining_Time);
        }

        pid = fork();
        if (pid == 0)
        {
            execl("./process.out", "process.out", Args[0], Args[1], Args[2], Args[3], NULL);
            perror("Erorr");
            // exit(-1);
        }
        running.Remaining_Time = Recv_Signal();
        Process_control[running.Id - 1].Remaining_time = running.Remaining_Time;

        if (running.Remaining_Time > 0)
        {
             strcpy(Process_control[running.Id-1].state,"stopped");
            Write_to_schedulerLog( Process_control[running.Id - 1].Waiting_Time, running.Id, running.Remaining_Time);
            return running;
        }
        if (running.Remaining_Time == 0)
        {
            Process_control[running.Id - 1].Finish_Time = getClk();
            Process_control[running.Id - 1].Waiting_Time=Process_control[running.Id - 1].Finish_Time-Process_control[running.Id - 1].Arrival_Time-Process_control[running.Id - 1].Execution_time;
            strcpy(Process_control[running.Id-1].state,"finished");
            Write_to_schedulerLog( Process_control[running.Id - 1].Waiting_Time, running.Id, 0);
        }
    }
    Process dummy;
    dummy.Id = -1;
    dummy.Arrive_Time = -1;
    return dummy;
}
Process Shortest_Remaining_time_Next(Node **Process_queue)
{
    return Round_Robin(Process_queue, 1);
}
int main(int argc, char *argv[])
{
    initClk();
    int quantum, numOfProcess, priority = 0;
    New_File();
    chosen = Recived_Config(&quantum, &numOfProcess);
    Process_control=malloc(numOfProcess * sizeof(PCB));
    int FixedSize=numOfProcess;
    Node *Process_queue = NULL;
    //  TODO implement the scheduler :)
    // upon termination release the clock resources.
    Process non_finished_process, Currunt_process;
    non_finished_process.Id = -1;

    while (numOfProcess > 0 || !isEmpty(&Process_queue) || non_finished_process.Id != -1)
    { // handle when multi process came in the same time
        Currunt_process = Recived_Process(&priority);
        while (Currunt_process.Arrive_Time != -1)
        {
            if (chosen == 2)
            { // SRTN priority
                priority = Currunt_process.Remaining_Time;
            }
            printf("Id = %d\n", Currunt_process.Id);
            enqueue(&Process_queue, Currunt_process, priority);
            numOfProcess--;
            Currunt_process = Recived_Process(&priority);
        }
        if (non_finished_process.Id != -1)
        {
            if (chosen == 2)
            { // SRTN priority
                priority = non_finished_process.Remaining_Time;
            }
            else
                priority = 0;
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
    while (wait(&quantum) > 0)
        ;
    // print_pcb();
    scheduler_perf(FixedSize);
    destroyClk(true);
    return 0;
}
/*
 all process have same arrive time differnet priority
*/