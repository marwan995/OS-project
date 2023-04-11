#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char * argv[])
{
    initClk();
  //  printf("\nHELLO PROCCESS %d %s\n",atoi(argv[1]),argv[2]);
    //TODO it needs to get the remaining time from somewhere
    remainingtime = atoi(argv[1]);
    int prev=getClk();
    while (remainingtime > 0)
    {   
      if(prev!=getClk()){
      remainingtime--;
      prev=getClk();
      }
    }
    printf("Process  FT = %d, ID = %d\n",getClk(), atoi(argv[2]));
    
    exit(5);
  //  kill(getppid(),SIGCHLD);
    destroyClk(false);
    return 0;
}
