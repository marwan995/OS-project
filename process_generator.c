#include "headers.h"
#include <string.h>
#define Process struct Process
#define msgbuff struct msgbuff
#define Config struct Config
int msgq_id,counter_of_process=0;
void clearResources(int);

Process File_To_Process(char line[])
{
  Process single;
  single.Id = atoi(strtok(line, "\t"));
  single.Arrive_Time = atoi(strtok(NULL, "\t"));
  single.Run_Time = atoi(strtok(NULL, "\t"));
  single.Priority = atoi(strtok(NULL, "\t"));
  single.Remaining_Time=single.Run_Time;
  return single;
}
int Read_File(char File_Path[], Process Processes[])
{
  FILE *File = fopen(File_Path, "r");
  char line[1000];

  // read and display the contents of the file line by line
  counter_of_process = 0;
  while (fgets(line, sizeof(line), File))
  {
    if (line[0] == '#')
      continue;
    Processes[counter_of_process] = File_To_Process(line);
    counter_of_process++;
  }

  // close the file
  fclose(File);
  return counter_of_process;
}
int Chosen_Algorithm(int *quantum)
{
  int Chosen = 2;
  printf("Enter the number of algorthim for scheduling \n1-Non-preemptive Highest Priority First\n2-Shortest Remaining time Next\n3-Round Robin\n");
  scanf("%d", &Chosen);
  if (Chosen == 3)
  {
    printf("Enter the quantum size");
    scanf("%d", quantum);
  }
  return Chosen;
}
void Create_Scheduler_and_Clock()
{
  int *pids = malloc((2) * sizeof(*pids));
  for (int i = 0; i < 2; ++i)
  {
    if ((pids[i] = fork()) < 0)
      perror("erorr");

    else if (pids[i] == 0)
    {
      if (i == 0)
      {
        execl("./clk.out", "./clk.out", NULL);
      }
      else
      {
        execl("./scheduler.out", "./scheduler.out", "", NULL);
      }
    }
  }
  
}
bool Insert_process_and_Send(Process p)
{
  msgbuff m;
  m.mtype = 4;
  m.p = p;
  int send_val = msgsnd(msgq_id, &m, sizeof(m.p), IPC_NOWAIT);
  return !(send_val == -1);
}
bool Configration_With_Scheduler(int chosen, int quantum)
{
  Config C;
  key_t key_id;
  key_id = ftok("keyfile", 65);               // create unique key
  msgq_id = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id
  if (msgq_id == -1)
  {
    perror("Error in create");
    exit(-1);
  }
  C.mtype = 1;
  C.Schedule[0] = chosen;
  C.Schedule[1] = quantum;
  C.Schedule[2]=counter_of_process;
  int send_val = msgsnd(msgq_id, &C, sizeof(C.Schedule), !IPC_NOWAIT);
  return !(send_val == -1);
}
int main(int argc, char *argv[])
{
  signal(SIGINT, clearResources);
  Process Processes[100]; // linked list
  int counter_of_process, chosen,quantum;
  // TODO Initialization
  // 1. Read the input files.
  counter_of_process = Read_File("process.txt", Processes);
  // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
  chosen = Chosen_Algorithm(&quantum);
  // 3. Initiate and create the scheduler and clock processes.
  Create_Scheduler_and_Clock();
  // 4. Use this function after creating the clock process to initialize clock
  initClk();
  // To get time use this
  
  //-----------------------------------------------------
  // TODO Generation Main Loop
  // 5. Create a data structure for processes and provide it with its parameters.
  // 6. Send the information to the scheduler at the appropriate time.
  // 7. Clear clock resources

  Configration_With_Scheduler(chosen,quantum);
  int Currunt_proccess_counter=0;
  while (counter_of_process>0)
  {
    int x = getClk();
    if (x >= Processes[Currunt_proccess_counter].Arrive_Time)
    {
       printf("Sending process with id %d time %d\n", Processes[Currunt_proccess_counter].Id,getClk());
       Insert_process_and_Send(Processes[Currunt_proccess_counter]);
       counter_of_process--;
       Currunt_proccess_counter++;

    }
  }
  printf("process generator off\n");
  return 0;
}

void clearResources(int signum)
{
  destroyClk(true);
  // TODO Clears all resources in case of interruption
}
