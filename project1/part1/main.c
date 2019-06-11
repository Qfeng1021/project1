#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "part1.h"


int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        char buff[255] = {0};
        FILE *fin = fopen(argv[1], "r");
        if(fin != NULL)
        {
            struct Program *program = NULL;
            while (fgets(buff, 255, fin) != NULL)
            {
                if(program == NULL)
                {
                    program = MallocProgram(buff);
                }
                else
                {
                    struct Program *newNode = MallocProgram(buff);
                    AppendProgram(&program, newNode);
                }
            }
            Launch(fin, &program);
            WaitProgram(&program);
            FreeProgram(&program);
            fclose(fin);
        }
        else
        {
            printf("Cannot open the file %s.\n", argv[1]);
        }
        return 0;
    }
    else
    {
        exit(-1);
    }
}

