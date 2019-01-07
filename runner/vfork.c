#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
 
int main()
{
    int pid = vfork();
    if(pid < 0)
        printf("error in fork!\n");
    else if (pid == 0) {
        printf("this is child, pid is %d\n", getpid());
        int newstdin = open(in,O_RDWR|O_CREAT,0644);
        int newstdout=open(out,O_RDWR|O_CREAT,0644);
        dup2(newstdout,fileno(stdout));
        dup2(newstdin,fileno(stdin));
        exit(0);
    }else if (pid > 0) {
        printf("this is parent, pid is %d\n", getpid());
    }
    return 0;
}