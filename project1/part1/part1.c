#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "part1.h"
//Qianyi Feng, 951439892, CIS 415 Project 1
//This is my own work except the partition() and quick_sort() function.
//Citation: The struct Program and the function code referenced from proj0<anagram.c>;

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

void Launch(FILE *file, struct Program **myProg)
{
    int Pid;
    struct Program *cur = *myProg;
    while(cur != NULL)
    {
        Pid = fork();
        if (Pid < 0)
        {
            perror("ERROR: ");
        }
        else if(Pid == 0)
        {
            if (execvp(cur->progName, cur->Args))
            {
                perror("Execvp error: ");
            }
            FreeProgram(myProg);
            fclose(file);
            exit(-1);
        }
        else
        {
            cur->Pid = Pid;
        }
        cur = cur->Next;
    }
}

