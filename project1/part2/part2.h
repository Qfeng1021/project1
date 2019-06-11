#include <stdio.h>
#include <stdlib.h>
#include <malloc/malloc.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <sys/signal.h>
#include <pthread.h>
#include <errno.h>
#ifndef PART1_H_INCLUDED
#define PART1_H_INCLUDED

struct Program
{
    char *progName;
    int Pid;
    struct Program *Next;
    char *Args[255];
};

struct Program *MallocProgram(char *buff);
void AppendProgram(struct Program **head, struct Program *node);
void FreeProgram(struct Program **node);
void WaitProgram(struct Program **node);
void Launch(FILE *file, struct Program **myProg);
void SendSigusr1(struct Program **node);
void SendSigstop(struct Program **node);
void Sendcont(struct Program **node);

#endif // PART1_H_INCLUDED
