#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
void handler(int Signum){
  printf("process killed\n");
  exit(0);
}
int main(int agrc, char * argv[])
{
    initClk();
    signal(SIGUSR1, handler);

    //TODO it needs to get the remaining time from somewhere
    remainingtime = atoi(argv[1]);
    int prev=getClk();
    printf("process RT: %d id: %s clk: %d\n",atoi(argv[1]),argv[2],prev);
    while (remainingtime > 0)
    {   
      if(prev!=getClk()){
      remainingtime--;
      prev=getClk();
      }
    }
    printf("Process  FT = %d, ID = %d\n",getClk(), atoi(argv[2]));
    
    exit(5);
    kill(getppid(),SIGCHLD);
    destroyClk(false);
    return 0;
}
