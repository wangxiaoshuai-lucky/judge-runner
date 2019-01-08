#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

void setProcessLimit(long timelimit, long memory_limit) {
    struct rlimit rl;
    /* set the time_limit (second)*/
    rl.rlim_cur = timelimit / 1000;
    rl.rlim_max = rl.rlim_cur + 1;
    setrlimit(RLIMIT_CPU, &rl);

    /* set the memory_limit (b)*/
    rl.rlim_cur = memory_limit * 1024;
    rl.rlim_max = rl.rlim_cur + 1024;
    setrlimit(RLIMIT_DATA, &rl);
}

int run(char *args[],long timelimit, long memory_limit, char *in, char *out){
    pid_t pid = vfork();
    if(pid<0)
        printf("error in fork!\n");
    else if(pid == 0) {
        int newstdin = open(in,O_RDWR|O_CREAT,0644);
        int newstdout=open(out,O_RDWR|O_CREAT,0644);
        if (newstdout != -1 && newstdin != -1){
            if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) != -1) {
                printf("====== ok =====\n");
            }
            dup2(newstdout,fileno(stdout));
            dup2(newstdin,fileno(stdin));
            setProcessLimit(timelimit, memory_limit);
            if (execvp(args[0],args) == -1) {
                printf("execvp is error!\n");
            }
            close(newstdin);
            close(newstdout);
        } else {
            printf("====== error =====\n");
        }
    } else {
        struct rusage ru;
        int status, time_used = 0, memory_used = 0;
        printf("the child pid is %d \n", pid);
        while (1) {
            if (wait4(pid, &status, WUNTRACED, &ru) == -1)
                printf("wait4 [WSTOPPED] failure");
            if (WIFEXITED(status)) {
                time_used = ru.ru_utime.tv_sec * 1000
                        + ru.ru_utime.tv_usec / 1000
                        + ru.ru_stime.tv_sec * 1000
                        + ru.ru_stime.tv_usec / 1000;
                memory_used = ru.ru_maxrss;
                printf("child process is right!\n");
                printf("timeused: %d ms | memoryused : %d b\n",time_used,memory_used);
                return 1;
            }
            else if (WSTOPSIG(status) != SIGTRAP) {
                ptrace(PTRACE_KILL, pid, NULL, NULL);
                waitpid(pid, NULL, 0);
                time_used = ru.ru_utime.tv_sec * 1000
                        + ru.ru_utime.tv_usec / 1000
                        + ru.ru_stime.tv_sec * 1000
                        + ru.ru_stime.tv_usec / 1000;
                memory_used = ru.ru_maxrss;
                switch (WSTOPSIG(status)) {
                    case SIGSEGV:
                        if (memory_used > memory_limit)
                            printf("ME\n");
                        else
                            printf("RE\n");
                        break;
                    case SIGALRM:
                    case SIGXCPU:
                        printf("TLE\n");
                        break;
                    default:
                        printf("RE\n");
                        break;
                }
                printf("child process is wrong!\n");
                printf("timeused: %d ms | memoryused : %d b\n",time_used,memory_used);
                return 0;
            }
            ptrace(PTRACE_CONT, pid, NULL, NULL);
        }
        return -1;
    }
}
int main()
{
    char *args[] = {"/home/hadoop/tmp/demo",NULL};
    run(args,1000,1000,"0.in","0.out");
    return 0;
}
