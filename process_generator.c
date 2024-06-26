#include "headers.h"
#include <string.h>
#define Process struct Process

void clearResources(int);
int msgq_id;
void intToStrArray(int num1, int num2, int num3,int num4, char strArr[4][10])
{
  sprintf(strArr[0], "%d", num1);
  sprintf(strArr[1], "%d", num2);
  sprintf(strArr[2], "%d", num3);
  sprintf(strArr[3], "%d", num4);
}
Process Process_handler(char line[])
{
  Process single;
  single.Id = atoi(strtok(line, "\t"));
  single.Arrive_Time = atoi(strtok(NULL, "\t"));
  single.Run_Time = atoi(strtok(NULL, "\t"));
  single.Priority = atoi(strtok(NULL, "\t"));
  single.Mem_Size = atoi(strtok(NULL, "\t"));
  if(single.Mem_Size==0)single.Mem_Size=1;
  single.Remaining_Time = single.Run_Time;
  return single;
}
int Read_File(char File_Path[], Process Processes[])
{
  FILE *File = fopen(File_Path, "r");
  char line[200]; // we assume that max line will be 200

  // read the contents of the file line by line
  int counter_of_process = 0;
  while (fgets(line, sizeof(line), File))
  {
    if (line[0] == '#') // ignore comments
      continue;
    Processes[counter_of_process] = Process_handler(line);
    counter_of_process++;
  }

  // close the file
  fclose(File);
  return counter_of_process;
}
int Chosen_Algorithm(int argc, char *argv[],int *quantum,int *Chosen_memory)
{
  // quantum passed by pointer as it holds the additional parameter
    int Chosen = 1;
   for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-sch") == 0) {
            if (i + 1 < argc) {
                Chosen = atoi(argv[i + 1]);
            }
            else {
                printf("Error: Missing value for -sch parameter.\n");
                exit(1);
            }
        }
        if (strcmp(argv[i], "-mem") == 0) {
            if (i + 1 < argc) {
                *Chosen_memory = atoi(argv[i + 1]);
            }
            else {
                printf("Error: Missing value for -mem parameter.\n");
                exit(1);
            }
        }
        if (strcmp(argv[i], "-q") == 0) {
            if (i + 1 < argc) {
                *quantum = atoi(argv[i + 1]);
            }
            else {
                printf("Error: Missing value for -q parameter.\n");
                exit(1);
            }
        }
  
    }

  return Chosen;
}
void Create_Scheduler_and_Clock(char Scheduler_Args[3][10])
{

  // fork two childern one for clk and the other for scheduler
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
        execl("./scheduler.out", "./scheduler.out", Scheduler_Args[0], Scheduler_Args[1], Scheduler_Args[2],Scheduler_Args[3], NULL);
      }
    }
  }
  free(pids);
}
void Insert_process_and_Send(Process p)
{
  key_t key_id;
  key_id = ftok("keyfile", 77);               // create unique key
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
  printf("Send a process from process generator %d\n",getClk());
  if(send_val == -1){
    perror("error itn Insert_process_and_Send");
  }
}
int main(int argc, char *argv[])
{
  signal(SIGINT, clearResources);
  Process Processes[1000];
  char Scheduler_Args[4][10];
  int counter_of_process, chosen, quantum,Chosen_memory, Currunt_proccess_index = 0;

  // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
  chosen = Chosen_Algorithm(argc,*&argv,&quantum,&Chosen_memory);
   // 1. Read the input files.
  counter_of_process = Read_File(argv[1], Processes);
  // 3. Initiate and create the scheduler and clock processes.
  // convert the integer paramters to string to be passed for the execl sys call 
  // as arguments 
  intToStrArray(chosen, quantum, counter_of_process,Chosen_memory,Scheduler_Args);
  Create_Scheduler_and_Clock(Scheduler_Args);
  // 4. Use this function after creating the clock process to initialize clock
  initClk();
  // 5. Create a data structure for processes and provide it with its parameters.
  // 6. Send the information to the scheduler at the appropriate time.
  // 7. Clear clock resources
  while (counter_of_process > 0)
  {
    int curClk = getClk();
    if (curClk >= Processes[Currunt_proccess_index].Arrive_Time)
    {
     // printf("Sending process with id %d time %d\n", Processes[Currunt_proccess_index].Id, getClk());
      Insert_process_and_Send(Processes[Currunt_proccess_index]);
      counter_of_process--;
      Currunt_proccess_index++;
    }
  }
  

  destroyClk(false);
  while (wait(&chosen) > 0); //wait for clk and scheduler
  return 0;
}

void clearResources(int signum)
{
  printf("process_generator Terminating!\n");
  destroyClk(false);
  while (wait(&signum) > 0) //wait for clk and scheduler
        ;

    exit(0);
         
}
