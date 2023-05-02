#include "headers.h"
#define EPS 0.001
#define memo_size 1024
PCB *Process_control;
int chosen, process_msgq_id = -1, msgq_id = -1;
int clearResources_flag = 0;
int memory[memo_size];
int mem_flag = 1;
TreeNode *root = NULL;
int prev_id = -1;
void clearResources();
void initialize_memory();
int Recv_Signal();
void New_File();
Process Recived_Process(int *priority);
void make_PCB(Process p);
void Write_to_schedulerLog(int id);
void Write_to_MemoryLog(int id, int size, int start, char *state, int actualSize);
void intToStrArray(int num1, int num2, int num3, int num4, char strArr[4][10]);
int Reserved_free_memory_FF(int size, int id);
int free_memory(int size, int id);
void Move_between_queues(Node **Process_queue, Node **has_no_mem_queue);
int checkMemo(int size);

void Non_preemptive_Highest_Priority_First(Node **Process_queue)
{
    int pid, status, sid;
    char Args[4][10];
    if (!isEmpty(&(*Process_queue)))
    {
        Process running = dequeue(&(*Process_queue));

        // if process has no free memory continue
        int memo_index = Reserved_free_memory_FF(running.Mem_Size, running.Id);

        Write_to_MemoryLog(running.Id, mem_flag == 2 ? pow(2, ceil(log2(running.Mem_Size))) : running.Mem_Size, memo_index, "allocated", running.Mem_Size);

        make_PCB(running);

        intToStrArray(running.Remaining_Time, running.Id, chosen, 10000, Args);
        Write_to_schedulerLog(running.Id);
        pid = fork();
        if (pid == 0)
        {
            execl("./process.out", "process.out", Args[0], Args[1], Args[2], Args[3], NULL);
            perror("erorr");
            return;
        }
        running.Remaining_Time = Recv_Signal();
        Process_control[running.Id - 1].Remaining_time = running.Remaining_Time;
        if ((sid = wait(&status)) > 0)
            ;
        Process_control[running.Id - 1].Finish_Time = getClk();
        int start = free_memory(running.Mem_Size, running.Id);
        Write_to_MemoryLog(running.Id, mem_flag == 2 ? pow(2, ceil(log2(running.Mem_Size))) : running.Mem_Size, start, "freed", running.Mem_Size);
        strcpy(Process_control[running.Id - 1].state, "finished");
        Write_to_schedulerLog(running.Id);
    }
}
void scheduler_pref(int size)
{
    int Worktime = 0, WaitTime = 0, TA = 0;
    float WTA = 0.0;
    FILE *fp;
    char line[200];
    fp = fopen("scheduler.pref", "w");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return;
    }
    for (int i = 0; i < size; i++)
    {
        Worktime += Process_control[i].Execution_time;
        WaitTime += Process_control[i].Finish_Time - Process_control[i].Arrival_Time - Process_control[i].Execution_time;
        TA = Process_control[i].Finish_Time - Process_control[i].Arrival_Time;
        WTA += Process_control[i].Execution_time > 0 ? (float)(TA) / Process_control[i].Execution_time : 0;
    }

    float ut = ((float)Worktime / getClk()) * 100 + EPS;
    // printf("clk:%d worktime:%d ut=%f\n", getClk(), Worktime, ut);
    float sum = 0, AvgWTA = (float)WTA / size + EPS;
    for (int i = 0; i < size; i++)
    {
        TA = Process_control[i].Finish_Time - Process_control[i].Arrival_Time;
        sum += pow(AvgWTA - (Process_control[i].Execution_time > 0 ? (float)TA / Process_control[i].Execution_time : 0), 2);
    }
    float std = sqrt(sum / size) + EPS;

    sprintf(line, "CPU utilization = %.2f%%\nAvg WTA = %.2f\nAvg Waiting = %.2f\nStd WTA = %.2f\n", ut, WTA / size + EPS, (float)WaitTime / size + EPS, std);
    fputs(line, fp);
    fclose(fp);
}

Process Round_Robin(Node **Process_queue, int quantum)
{
    int pid;
    char Args[4][10];
    Process dummy;
    dummy.Remaining_Time = 0;
    if (!isEmpty(&(*Process_queue)))
    {
        Process running = dequeue(&(*Process_queue));
        intToStrArray(running.Remaining_Time, running.Id, chosen, quantum, Args);
        if (running.Run_Time == running.Remaining_Time) // if first time to run add it to pcb and write it in schedulerLog
        {

            int memo_index = Reserved_free_memory_FF(running.Mem_Size, running.Id);

            if (memo_index == -1)
            {   
                
                return running;

            }
            printf("procees found memory:: id = %d, memo_indx = %d, memo_size = %d\n", running.Id, memo_index, running.Mem_Size);
            Write_to_MemoryLog(running.Id, mem_flag == 2 ? pow(2, ceil(log2(running.Mem_Size))) : running.Mem_Size, memo_index, "allocated", running.Mem_Size);
            make_PCB(running);

            Write_to_schedulerLog(running.Id);
        }
        else
        {
            // update just the wait time
            if (chosen == 3)
            {
                int arr_time = Process_control[running.Id - 1].Arrival_Time;
                int exec_time = Process_control[running.Id - 1].Execution_time;
                int rem_time = Process_control[running.Id - 1].Remaining_time;
                Process_control[running.Id - 1].Waiting_Time = getClk() - arr_time - (exec_time - rem_time);
                strcpy(Process_control[running.Id - 1].state, "resumed");
                Write_to_schedulerLog(running.Id);
            }
        }

        pid = fork();
        if (pid == 0)
        {
            execl("./process.out", "process.out", Args[0], Args[1], Args[2], Args[3], NULL);
            perror("Erorr");
            exit(-1);
        }
        running.Remaining_Time = Recv_Signal();
        Process_control[running.Id - 1].Remaining_time = running.Remaining_Time;

        // after the process finishes its quantum free its memory space

        if (running.Remaining_Time > 0)
        {
            if (chosen == 2)
                return running;
            strcpy(Process_control[running.Id - 1].state, "stopped");
            Write_to_schedulerLog(running.Id);
            return running;
        }
        else if (running.Remaining_Time == 0)
        {
            Process_control[running.Id - 1].Finish_Time = getClk();
            Process_control[running.Id - 1].Waiting_Time = Process_control[running.Id - 1].Finish_Time - Process_control[running.Id - 1].Arrival_Time - Process_control[running.Id - 1].Execution_time;
            int start = free_memory(running.Mem_Size, running.Id);
            Write_to_MemoryLog(running.Id, mem_flag == 2 ? pow(2, ceil(log2(running.Mem_Size))) : running.Mem_Size, start, "freed", running.Mem_Size);
            printf("memeory is freed \n");
            strcpy(Process_control[running.Id - 1].state, "finished");
            Write_to_schedulerLog(running.Id);
        }
    }
    else
    {
        dummy.Remaining_Time = -1;
    }
    dummy.Id = -1; // To know if the process is finished or not, -1 means it has finished
    return dummy;
}
Process Shortest_Remaining_time_Next(Node **Process_queue)
{
    return Round_Robin(Process_queue, 1);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    initClk();
    chosen = atoi(argv[1]);      // chosen algo
    int quantum = atoi(argv[2]); // q of Round Robin
    int numOfProcess = atoi(argv[3]);
    mem_flag = atoi(argv[4]);
    int priority = 0;
    Process_control = malloc(numOfProcess * sizeof(PCB)); // PCB
    for (int i = 0; i < numOfProcess; i++)
    {
        Process_control[i].PID = -1;
    }
    int FixedSize = numOfProcess;
    Node *Process_queue = NULL; // process priority queue
    Node *has_No_Memory_queue = NULL;
    Process non_finished_process, Currunt_process;
    non_finished_process.Id = -1;
    non_finished_process.Remaining_Time = -1;
    New_File();          // init the Schaduluer.log file
    initialize_memory(); // init the memorey
    while (numOfProcess > 0 || !isEmpty(&Process_queue) || non_finished_process.Id != -1 || !isEmpty(&has_No_Memory_queue))
    {
        Currunt_process = Recived_Process(&priority);
        int first_enqueue = 1;
        while (Currunt_process.Arrive_Time != -1) // while there is a process recive it
        {
            if (chosen == 2)// SRTN priority
                priority = Currunt_process.Remaining_Time;
            if(first_enqueue == 1 && chosen == 3) {
                Move_between_queues(&Process_queue, &has_No_Memory_queue);
            }
            first_enqueue = 2;
           if(chosen==2 && checkMemo(Currunt_process.Mem_Size) == 0){
                enqueue(&has_No_Memory_queue, Currunt_process, priority);
           }
           else        
               enqueue(&Process_queue, Currunt_process, priority);

            numOfProcess--;
            Currunt_process = Recived_Process(&priority);
        }
        if (non_finished_process.Id != -1) // check if the process is not finished in RR OR SRTN then enqueue it again
        {
            if (chosen == 2)
                // SRTN priority
                priority = non_finished_process.Remaining_Time;
            else
                priority = 0;

            if (non_finished_process.Remaining_Time == non_finished_process.Run_Time)
            { // this means that the process didn't find memory
                enqueue(&has_No_Memory_queue, non_finished_process, priority);
            }
            else
            {
                enqueue(&Process_queue, non_finished_process, priority);
            }
            Process headProcess = Peek(&Process_queue);
            if (headProcess.Id != non_finished_process.Id && chosen == 2)
            {
                strcpy(Process_control[non_finished_process.Id - 1].state, "stopped");
                Write_to_schedulerLog(non_finished_process.Id);              
            }
            non_finished_process.Id = -1;
        }
        else if (non_finished_process.Remaining_Time == -2)
        {           
            Move_between_queues(&Process_queue, &has_No_Memory_queue);
            Process headProcess = Peek(&Process_queue);//if null id=-5

              if (headProcess.Id!=-5&&headProcess.Remaining_Time != headProcess.Run_Time&&chosen==2)
                {
                    int arr_time = Process_control[headProcess.Id - 1].Arrival_Time;
                    int exec_time = Process_control[headProcess.Id - 1].Execution_time;
                    int rem_time = Process_control[headProcess.Id - 1].Remaining_time;
                    Process_control[headProcess.Id - 1].Waiting_Time = getClk() - arr_time - (exec_time - rem_time);
                    strcpy(Process_control[headProcess.Id - 1].state, "resumed");
                    Write_to_schedulerLog(headProcess.Id);
                }
        }
        if (chosen == 1)
            Non_preemptive_Highest_Priority_First(&Process_queue);
        else if (chosen == 2)
            non_finished_process = Shortest_Remaining_time_Next(&Process_queue);
        else if (chosen == 3)
            non_finished_process = Round_Robin(&Process_queue, quantum);

        if (non_finished_process.Remaining_Time == 0)
        {
            non_finished_process.Remaining_Time = -2;
        }
        else if (non_finished_process.Remaining_Time == -2)
            non_finished_process.Remaining_Time = -1;
    }
    while (wait(&quantum) > 0) // wait all process to finish
        ;
    scheduler_pref(FixedSize);
    destroyClk(true);
    return 0;
}

void Move_between_queues(Node **Process_queue, Node **has_no_mem_queue)
{
    while (!isEmpty(&(*has_no_mem_queue)))
    {
        Process cur_process = dequeue(&(*has_no_mem_queue));
        if(chosen==2)
        enqueue(&(*Process_queue), cur_process, cur_process.Remaining_Time);
        else
         enqueue(&(*Process_queue), cur_process, 0);
    }
}

void clearResources()
{
    if (clearResources_flag)
        exit(0);
    free(Process_control);
    if (process_msgq_id != -1 && msgctl(process_msgq_id, IPC_RMID, NULL) == -1)
    {
        perror("Error: failed to remove message queue signal");
    }
    if (msgq_id != -1 && msgctl(msgq_id, IPC_RMID, NULL) == -1)
    {
        perror("Error: failed to remove message queue process in shcadual");
    }
    printf("Scheduler Terminating!\n");
    clearResources_flag = 1;
    destroyClk(true);
    exit(0);
}

void initialize_memory()
{
    if (mem_flag == 1)
    {
        for (int i = 0; i < memo_size; i++)
            memory[i] = 0;
    }
    else
    {
        root = (TreeNode *)malloc(sizeof(TreeNode));
        root->left = NULL;
        root->right = NULL;
        root->size = 1024;
        root->start = 0;
        root->takenId = -1;
        root->cursize = 1024;
    }
}

int free_memory(int size, int id)
{
    int start = 0;
    if (mem_flag == 1)
    {
        while (start < memo_size)
        {
            if (memory[start] == id)
                break;
            start++;
        }
        for (int i = 0, j = start; i < size; i++, j++)
            memory[j % memo_size] = 0;
    }
    else
    {
        deleteNode(&root, id, &start);
    }
    return start;
}
int checkMemo(int size){
      if (mem_flag == 1)
    {
        for (int i = 0; i < memo_size; i++)
        {
            if (memory[i] == 0)
            {
                int cnt = 0;
               for (int j = i; cnt < size && j < memo_size; j++)
                {
                    if (memory[j] == 0)
                        cnt++;
                    else
                        break;
                }
                // memory[i] = id -> start of memory
                // memory[i] = -1 -> you cannot take this location, it's reserved
                if (cnt == size)
                {
                    return 1;
                }
            }
        }
        return 0;
    }
    else {
            return Check(&root, next_power_of_two(size), 1024, 0);
    }
}
int Reserved_free_memory_FF(int size, int id)
{ // free memory for first fit
    // printf("memory[0] = %d\n", memory[0]);
    if (mem_flag == 1)
    {
        for (int i = 0; i < memo_size; i++)
        {
            if (memory[i] == 0)
            {
                int cnt = 0;
                for (int j = i; cnt < size && j < memo_size; j++)
                {
                    if (memory[j] == 0)
                        cnt++;
                    else
                        break;
                }
                // memory[i] = id -> start of memory
                // memory[i] = -1 -> you cannot take this location, it's reserved
                if (cnt == size)
                {
                    memory[i] = id;
                    if (id == 1)
                        printf("reserved memroy\n");
                    cnt = 1;
                    for (int j = i + 1; cnt < size && j < memo_size; j++, cnt++)
                    {
                        memory[j] = -1;
                    }
                    return i;
                }
            }
        }
    }
    else
    {
        int sz = insert(&root, next_power_of_two(size), id, 1024, 0);
        if (sz == 0)
            return -1;
        int ret = findNode(&root, id)->start;
        return ret;
    }

    return -1;
}

void intToStrArray(int num1, int num2, int num3, int num4, char strArr[4][10])
{
    sprintf(strArr[0], "%d", num1);
    sprintf(strArr[1], "%d", num2);
    sprintf(strArr[2], "%d", num3);
    sprintf(strArr[3], "%d", num4);
}

void Write_to_MemoryLog(int id, int size, int start, char *state, int actualSize)
{
    FILE *fp;
    char line[200];

    sprintf(line, "At time %d %s %d bytes for process %d from %d to %d\n", getClk(), state, actualSize, id, start, start + size - 1);

    fp = fopen("memory.log", "a");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return;
    }

    fprintf(fp, "%s", line);
    fclose(fp);
}

void Write_to_schedulerLog(int id)
{
    FILE *fp;
    char line[200];
    char state[50];
    strcpy(state, Process_control[id - 1].state);
    int wait = Process_control[id - 1].Waiting_Time;
    int remain = Process_control[id - 1].Remaining_time;
    if (strcmp(state, "finished") != 0)
        sprintf(line, "At time %d process %d %s arr %d total %d remain %d wait %d\n", getClk(), id, state, Process_control[id - 1].Arrival_Time, Process_control[id - 1].Execution_time, remain, wait);
    else
    {
        int TA = Process_control[id - 1].Finish_Time - Process_control[id - 1].Arrival_Time;
        float WTA = Process_control[id - 1].Execution_time > 0 ? (float)TA / Process_control[id - 1].Execution_time + EPS : 0;
        sprintf(line, "At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), id, state, Process_control[id - 1].Arrival_Time, Process_control[id - 1].Execution_time, remain, wait, TA, WTA);
    }
    fp = fopen("scheduler.log", "a");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return;
    }

    fprintf(fp, "%s", line);
    fclose(fp);
}

void make_PCB(Process p)
{
    int index = p.Id - 1;
    Process_control[index].PID = p.Id;
    strcpy(Process_control[index].state, "started");
    Process_control[index].Arrival_Time = p.Arrive_Time;
    Process_control[index].Start_Time = getClk();
    Process_control[index].Waiting_Time = getClk() - p.Arrive_Time - (p.Run_Time - p.Remaining_Time);
    Process_control[index].Remaining_time = p.Remaining_Time;
    Process_control[index].Execution_time = p.Run_Time;
    Process_control[index].Finish_Time = 0;
}

Process Recived_Process(int *priority)
{
    key_t key_id = ftok("keyfile", 77);         // create unique key
    msgq_id = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id
    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
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

void New_File()
{
    FILE *fp;
    char str[] = "#At time x process y state arr w total z remain y wait k\n";
    char strm[] = "#At time x allocated y bytes for process z from I to j\n";
    fp = fopen("scheduler.log", "w");
    FILE *fm = fopen("memory.log", "w");
    if (fp == NULL || fm == NULL)
    {
        printf("Error opening file\n");
        return;
    }
    fputs(str, fp);
    fputs(strm, fm);
    fclose(fm);
    fclose(fp);
}

int Recv_Signal()
{
    key_t process_key = ftok("pidfile", 75);                 // create unique key
    process_msgq_id = msgget(process_key, 0644 | IPC_CREAT); // create message queue and return id
    ProcessSignal to_recv;
    int rec_val = msgrcv(process_msgq_id, &to_recv, sizeof(int), 65, !IPC_NOWAIT); // 0
    if (rec_val == -1)
        perror("reciving");
    return to_recv.Remaining_Time;
}
