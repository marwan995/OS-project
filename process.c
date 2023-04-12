#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int msgq_id;
key_t key_id;
void handler(int Signum)
{
  printf("process killed\n");

  exit(0);
}
void conif_singal()
{
  key_id = ftok("pidfile", 75);               // create unique key
  msgq_id = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id
}
void Recv_Signal()
{
  Config pid_to_recv;
  int rec_val = msgrcv(msgq_id, &pid_to_recv, sizeof(pid_to_recv.Schedule), 10, IPC_NOWAIT); // 0
  if (rec_val != -1)
    kill(getpid(), SIGINT);
}
int main(int agrc, char *argv[])
{
  initClk();
  signal(SIGINT, handler);
  conif_singal();
  // TODO it needs to get the remaining time from somewhere
  remainingtime = atoi(argv[1]);
  int prev = getClk();
  printf("process RT: %d id: %s clk: %d pid :%d\n", atoi(argv[1]), argv[2], prev, getpid());
  while (remainingtime > 0)
  {
    Recv_Signal();
    if (prev != getClk())
    {
      remainingtime--;
      prev = getClk();
    }
  }
  printf("Process  FT = %d, ID = %d\n", getClk(), atoi(argv[2]));

  exit(5);
  //  kill(getppid(),SIGCHLD);
  destroyClk(false);
  return 0;
}
