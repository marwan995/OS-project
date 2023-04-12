#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

void handler(int Signum)
{
  printf("process killed \n");

  exit(0);
}

int Send_Signal(int rt)
{
   
  Config RT_to_Send;
  key_t key_id;
  key_id = ftok("pidfile", 75);               // create unique key
  int msgq_id = msgget(key_id, 0644 | IPC_CREAT); // create message queue and return id
  if (msgq_id == -1)
  {
    perror("Error in create");
    exit(-1);
  }
  RT_to_Send.mtype = 65;
  RT_to_Send.Schedule[0] = rt;
  int send_val = msgsnd(msgq_id, &RT_to_Send, sizeof(RT_to_Send.Schedule), !IPC_NOWAIT);
}
int main(int agrc, char *argv[])
{
  initClk();
  signal(SIGINT, handler);
  // TODO it needs to get the remaining time from somewhere
  remainingtime = atoi(argv[1]);
  int id = atoi(argv[2]);
  int quantum = atoi(argv[4]);
  int chosen = atoi(argv[3]);
  int prev = getClk();
  printf("process RT: %d id: %d clk: %d quantum:%d  chosen:%d\n", remainingtime, id, prev,quantum,chosen);
  while (remainingtime > 0&&quantum>0)
  {
    if (prev != getClk())
    {
      quantum--;
      remainingtime--;
      prev = getClk();
    }
  }
  if(remainingtime==0)
    printf("Process  FT = %d, ID = %d\n", getClk(),id);
  
  if(chosen == 3)
    Send_Signal(remainingtime);

  exit(5);
  //  kill(getppid(),SIGCHLD);
  destroyClk(false);
  return 0;
}
