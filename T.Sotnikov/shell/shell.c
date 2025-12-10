#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "shell.h"
#include <stdlib.h>
#include <fcntl.h>

char *infile, *outfile, *appfile;
struct command cmds[MAXCMDS];
char bkgrnd;

int main(int argc, char *argv[])
{
    int i;
    char line[1024];      /* allow large command lines */
    int ncmds;
    char prompt[50];      /* shell prompt */

    /* PLACE SIGNAL CODE HERE */

    sprintf(prompt,"[%s] ", argv[0]);

    while (promptline(prompt, line, sizeof(line)) > 0) /* until eof */
    {
        if ((ncmds = parseline(line)) <= 0)
        {
            continue;   /* read next line */
        }
        
#ifdef DEBUG
        {
            int i, j;
            for (i = 0; i < ncmds; i++)
            {
                for (j = 0; cmds[i].cmdargs[j] != (char *) NULL; j++)
                {
                    fprintf(stderr, "cmd[%d].cmdargs[%d] = %s\n",
                         i, j, cmds[i].cmdargs[j]);
                }
                fprintf(stderr, "cmds[%d].cmdflag = %o\n", i, cmds[i].cmdflag);
            }
        }
#endif

        for (i = 0; i < ncmds; i++)
        {
            /* FORK AND EXECUTE CODE GOES HERE */

            pid_t pid = fork();

            if (pid < 0)
            {
                perror("Fork failed");
                exit(1);
            }

            if (pid == 0)
            {
                if (infile != NULL && i == 0)
                {
                    int fd_in = open(infile, O_RDONLY);
                    if (fd_in < 0) 
                    {
                        perror("open");
                        exit(1);
                    }
                    if (dup2(fd_in, 0) < 0)
                    {
                        perror("dup2");
                        exit(1);
                    }
                    close(fd_in);
                }

                if (outfile != NULL && i == ncmds - 1)
                {
                    int fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    if (fd_out < 0)
                    {
                        perror("open");
                        exit(1);
                    }
                    if (dup2(fd_out, 1) < 0)
                    {
                        perror("dup2");
                        exit(1);
                    }
                    close(fd_out);
                }

                else if (appfile != NULL && i == ncmds - 1)
                {
                    int fd_app = open(appfile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                    if (fd_app < 0)
                    {
                        perror("open");
                        exit(1);
                    }
                    if (dup2(fd_app, 1) < 0)
                    {
                        perror("dup2");
                        exit(1);
                    }
                    close(fd_app);
                }

                execvp(cmds[i].cmdargs[0], cmds[i].cmdargs);
                perror("execvp");
                exit(1);
            }

            else 
            {
                if (bkgrnd == 0)
                    wait(NULL);
                else 
                    printf("Process %d started in background\n", pid);
            }
        }
    } /* close while */
    return 0;
}

/* PLACE SIGNAL CODE HERE */
