#include "headers.h"
int remainingtime;

void handler(int Signum)
{
  printf("process killed \n");
  exit(-1);
}

int Send_Signal(int rt)
{
   
  ProcessSignal RT_to_Send;
  key_t key_id;
  key_id = ftok("pidfile", 75);               // create unique key
  int msgq_id = msgget(key_id, 0644 | IPC_CREAT); // create message queue and return id
  if (msgq_id == -1)
  {
    perror("Error in create");
    exit(-1);
  }
  RT_to_Send.mtype = 65;
  RT_to_Send.Remaining_Time = rt;
  int send_val = msgsnd(msgq_id, &RT_to_Send, sizeof(RT_to_Send.Remaining_Time), !IPC_NOWAIT);
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
  if(chosen==3)
  printf("process RT: %d id: %d clk: %d quantum:%d\n", remainingtime, id, prev,quantum);
  else
  printf("process RT: %d id: %d clk: %d\n", remainingtime, id, prev);
  while (remainingtime > 0 && quantum>0)
  {
    if (prev != getClk())
    {
      quantum--;
      remainingtime--;
      prev = getClk();
    }
  }
  if(remainingtime == 0)
    printf("Process  FT = %d, ID = %d\n", getClk(),id);
  
  if(chosen == 3 || chosen == 2)
    Send_Signal(remainingtime);
  destroyClk(false);
  return 0;
}
