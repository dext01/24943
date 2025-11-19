#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        fprintf(stderr, "Example: %s testing.txt\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        printf("child [%d]: running cat for file \"%s\"\n",
               getpid(), filename);

        execlp("cat", "cat", filename, (char *)NULL);
      
        perror("execlp");
        _exit(127);
    } else {
        int status;

        printf("parent [%d]: created child with pid %d\n",
               getpid(), pid);
        printf("parent: waiting for child to finish...\n");

        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status)) {
            printf("parent: child exited with code %d\n",
                   WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("parent: child was killed by signal %d\n",
                   WTERMSIG(status));
        } else {
            printf("parent: child finished with unknown status 0x%x\n",
                   status);
        }
        printf("parent: final message AFTER child has finished.\n");
    }

    return 0;
}
