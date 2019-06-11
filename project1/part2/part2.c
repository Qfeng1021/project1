#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "part2.h"
#define handle_error_en(en, msg) do {errno = en; perror(msg); exit(EXIT_FAILURE);} while(0)
//Qianyi Feng, 951439892, CIS 415 Project 1
//This is my own work except the partition() and quick_sort() function.
//Citation: The struct Program and the function code referenced from proj0<anagram.c>;
//Citation: the sig_thread function referenced from https://blog.csdn.net/yusiguyuan/article/details/14237277

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

void WaitProgram(struct Program **node)
{
    struct Program *cur = *node;
    while(cur != NULL)
    {
        struct Program *tmp = cur;
        cur = cur->Next;
        wait(&tmp->Pid);
    }
    node = NULL;
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
        printf("Signal handling thread got signal %i\n", sig);
        if(execvp(cur->progName, cur->Args))
            //got signal, execvp£»
        {
            perror("Execvp error: ");
            return -1;
        }
    }
}

void SendSigusr1(struct Program **node)
{
    struct Program *cur = *node;
    while(cur !=  NULL)
    {
        if(kill(cur->Pid, SIGUSR1) == -1)
        {
            perror("Fail to send signal.\n");
        }
        else
        {
            printf("Send SIGUSR1 to child process %d.\n", cur->Pid);
        }
        cur = cur->Next;
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

void Sendcont(struct Program **node)
{
    struct Program *cur = *node;
    while(cur !=  NULL)
    {
        if(kill(cur->Pid, SIGCONT) == -1)
        {
            perror("Fail to send signal.\n");
        }
        else
        {
            printf("Send SIGCONT to child process %d.\n", cur->Pid);
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
    sigaddset(&set, SIGUSR1); //add SIGUSR1 to signal set
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
            printf("PID_%i: waiting for sigusrl...\n", cur->Pid);
        }
        cur = cur->Next;
    }
}

