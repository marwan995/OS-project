#include "headers.h"
#include <string.h>
#define Process struct Process

void clearResources(int);
int msgq_id;
void intToStrArray(int num1, int num2, int num3, char strArr[3][10])
{
  sprintf(strArr[0], "%d", num1);
  sprintf(strArr[1], "%d", num2);
  sprintf(strArr[2], "%d", num3);
}
Process File_To_Process(char line[])
{
  Process single;
  single.Id = atoi(strtok(line, "\t"));
  single.Arrive_Time = atoi(strtok(NULL, "\t"));
  single.Run_Time = atoi(strtok(NULL, "\t"));
  single.Priority = atoi(strtok(NULL, "\t"));
  single.Remaining_Time = single.Run_Time;
  return single;
}
int Read_File(char File_Path[], Process Processes[])
{
  FILE *File = fopen(File_Path, "r");
  char line[200];

  // read the contents of the file line by line
  int counter_of_process = 0;
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
  int Chosen = 1;
  printf("Enter the number of algorthim for scheduling \n1-Non-preemptive Highest Priority First\n2-Shortest Remaining time Next\n3-Round Robin\n");
  scanf("%d", &Chosen);
  if (Chosen == 3)
  {
    printf("Enter the quantum size: ");
    scanf("%d", quantum);
    *quantum = (*quantum) < 1 ? 1 : (*quantum);
  }
  if (Chosen < 1 || Chosen > 3)
  {
    printf("WRONG INPUT EXIT\n");
    exit(-1);
  }
  return Chosen;
}
void Create_Scheduler_and_Clock(char Scheduler_Args[3][10])
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
        execl("./scheduler.out", "./scheduler.out", Scheduler_Args[0], Scheduler_Args[1], Scheduler_Args[2], NULL);
      }
    }
  }
  free(pids);
}
bool Insert_process_and_Send(Process p)
{
  key_t key_id;
  key_id = ftok("keyfile", 65);               // create unique key
  msgq_id = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id
  if (msgq_id == -1)
  {
    perror("Error in create");
    exit(-1);
  }
  msgbuff m;
  m.mtype = 4;
  m.p = p;
  int send_val = msgsnd(msgq_id, &m, sizeof(m.p), IPC_NOWAIT);
  return !(send_val == -1);
}
int main(int argc, char *argv[])
{
  signal(SIGINT, clearResources);
  Process Processes[100];
  char Scheduler_Args[3][10];
  int counter_of_process, chosen, quantum, Currunt_proccess_counter = 0;
  // 1. Read the input files.
  counter_of_process = Read_File("processes.txt", Processes);
  // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
  chosen = Chosen_Algorithm(&quantum);
  // 3. Initiate and create the scheduler and clock processes.
  intToStrArray(chosen, quantum, counter_of_process, Scheduler_Args);
  Create_Scheduler_and_Clock(Scheduler_Args);
  // 4. Use this function after creating the clock process to initialize clock
  initClk();

  // 5. Create a data structure for processes and provide it with its parameters.
  // 6. Send the information to the scheduler at the appropriate time.
  // 7. Clear clock resources
  while (counter_of_process > 0)
  {
    int x = getClk();
    if (x >= Processes[Currunt_proccess_counter].Arrive_Time)
    {
      printf("Sending process with id %d time %d\n", Processes[Currunt_proccess_counter].Id, getClk());
      Insert_process_and_Send(Processes[Currunt_proccess_counter]);
      counter_of_process--;
      Currunt_proccess_counter++;
    }
  }
  printf("process generator off\n");
  destroyClk(false);
  return 0;
}

void clearResources(int signum)
{
     while (wait(&signum) > 0) //wait for clk and scheduler
        ;
         destroyClk(false);
  exit(-1);
}
