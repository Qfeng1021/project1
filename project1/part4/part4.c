#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "part4.h"
#define handle_error_en(en, msg) do {errno = en; perror(msg); exit(EXIT_FAILURE);} while(0)
//Qianyi Feng, 951439892, CIS 415 Project 1
//This is my own work except the partition() and quick_sort() function.
//Citation: The struct Program and the function code referenced from proj0<anagram.c>;
//Citation: the sig_thread function and the creating of sigset make referenced from https://blog.csdn.net/yusiguyuan/article/details/14237277
//waitpid made referenced from https://blog.csdn.net/yiyi__baby/article/details/45539993
struct Program *MallocProgram(char *buff)
{
     char *data = (char *)malloc(sizeof(char)*255);
     buff[strlen(buff) - 1] = 0;
     strcpy(data, buff);
     int len = strlen(buff);
     char *args[len];
     args[0] = strtok(data, " ");
     int i;
     for(i = 1; i <= len - 1; i++)
    {
        args[i] = strtok(NULL, " ");
    }

    struct Program *newNode = (struct Program *)malloc(sizeof(struct Program));
    newNode->Next = NULL;
    newNode->HasExited = 0;
    newNode->progName = args[0];
    i = 0;
    while (args[i] != NULL)
    {
        newNode->Args[i] = args[i];
        i = i + 1;
    }
    newNode->Args[i] = 0;
    return newNode;
}

struct Program *CurProg = NULL;

void AppendProgram(struct Program **head, struct Program *node)
{
    if(*head == NULL)
    {
        *head = node;
    }
    else
    {
        struct Program *tail = *head;
        while(tail->Next != NULL)
        {
            tail = tail->Next;
        }
        tail->Next = node;
    }
}

void FreeProgram(struct Program **node)
{
    struct Program *cur = *node;
    while(cur != NULL)
    {
        struct Program *tmp = cur;
        cur = cur->Next;
        free(tmp->Args[0]);
        free(tmp);
    }
    node = NULL;
}

int CheckWait(struct Program **node)
{
    struct Program *cur = *node;
    while(cur != NULL)
    {
        if(cur->HasExited == 0)
        {
            return 0;
        }
        cur = cur->Next;
    }
    return 1;
}

static int sig_thread(void *arg, struct Program *cur)
{
    //waiting for the signal from parent, if no signal, then sigwait.
    sigset_t *set = arg;
    int s, sig;
    for(;;)
    {
        s = sigwait(set, &sig);
        if(s != 0)
        {
            handle_error_en(s, "sigwait");
        }
        printf("Child: Signal handling thread got signal %i.\n", sig);
        if(sig == 18)//SigCont
        {
            if(execvp(cur->progName, cur->Args))
            //got SigCont, execvp
            {
                perror("Execvp error: ");
                return -1;
            }
        }
    }
}

static void Print_Proc_Stat(char *filename)
{
    char buff[255] = {0};
    FILE *fin = fopen(filename, "r");
    fgets(buff, sizeof(buff), fin);
    buff[strlen(buff)-1] = 0;
    int len = strlen(buff);
    char *args[len];
    args[0] = strtok(buff, " ");
    int i;
    for (i = 1; i <= len - 1; i++)
    {
        args[i] = strtok(NULL, " ");
    }
    printf("\nPID: %s\n", args[0]);
    printf("Comm: %s\n", args[1]);
    printf("State: %s\n", args[2]);
    printf("Utime: %s\n", args[13]);
    printf("Stime: %s\n", args[14]);
    printf("Rss: %s\n", args[23]);
    fclose(fin);
}

static void Handle_alarm()
{
    int s, status;
    char filename[255];
    sprintf(filename, "/proc/%d/stat", CurProg->Pid);
    Print_Proc_stat(filename);
    s = waitpid(CurProg->Pid, &status, WNOHANG);
    if(s == 0)
    {
        kill(CurProg->Pid, SIGSTOP);
        printf("\nHandle_alarm: Send SIGSTOP to PID_%i.\n\n", CurProg->Pid);
    }
    else if (s != -1)
    {
        if(WIFEXITED(status))
        {
            CurProg->HasExited = 1;
            printf("\nChild process Pid_%i has exited.\n\n", CurProg->Pid);
        }
    }
    CurProg = CurProg->Next;
}

void SigAlarm(struct Program **node)
{
    CurProg = *node;
    signal(SIGALRM, Handle_alarm);//got signal, run handle_alarm;wwwww
    while(CurProg != NULL)
    {
        if(CurProg->HasExited == 0)
        {
            alarm(1);
            printf("SigAlarm: Send SIGCONT to PID_%i.\n\n", CurProg->Pid);
            kill(CurProg->Pid, SIGCONT);
            pause();//parent
            printf("SigAlarm: Wake up...\n");
        }
        else
        {
            CurProg = CurProg->Next;
        }
    }
}


void SendSigstop(struct Program **node)
{
    struct Program *cur = *node;
    while(cur !=  NULL)
    {
        if(kill(cur->Pid, SIGSTOP) == -1)
        {
            perror("Fail to send signal.\n");
        }
        else
        {
            printf("Send SIGSTOP to child process %d.\n", cur->Pid);
        }
        cur = cur->Next;
    }
}

void Launch(FILE *file, struct Program **myProg)
{
    int Pid;
    struct Program *cur = *myProg;
    sigset_t set; //create a set for signal
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGCONT); //add SIGUSR1 to signal set
    s = pthread_sigmask(SIG_BLOCK, &set, NULL); //make sure only the thread with sigwait would react with signal in sigset.
    if(s != 0)
        {
            handle_error_en(s, "pthread sigmask");
        }
    while(cur != NULL)
    {
        Pid = fork();
        if (Pid < 0)
        {
            perror("ERROR: ");
        }
        else if(Pid == 0)
        {
            if (sig_thread(&set, cur))
            {
                //success
                FreeProgram(myProg);
                fclose(file);
                exit(-1);
            }
        }
        else
        {
            cur->Pid = Pid;
        }
        cur = cur->Next;
    }
}

