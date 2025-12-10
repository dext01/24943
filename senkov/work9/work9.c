#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    // Проверка количества аргументов
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        fprintf(stderr, "Example: %s longfile.txt\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];

    // Проверка доступности файла
    if (access(filename, F_OK | R_OK) == -1) {
        perror("Error: Cannot access file");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    // Дочерний процесс
    if (pid == 0) {
        printf("child [%d]: running cat for file \"%s\"\n",
               getpid(), filename);

        execlp("cat", "cat", filename, (char *)NULL);

        perror("execlp");
        _exit(127);
    } 
    // Родительский процесс
    else {
        int status;
        pid_t ret;

        printf("parent [%d]: created child with pid %ld\n",
               getpid(), (long)pid);
        printf("parent: waiting for child to finish...\n");

        ret = wait(&status);
        printf("parent: wait's return value: %ld\n", (long)ret);

        // Детальный анализ завершения
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("parent: child exited normally with code: %d\n", exit_code);
            
            if (exit_code != 0) {
                printf("parent: warning - child reported an error (cat failed).\n");
            }
        } 
        else if (WIFSIGNALED(status)) {
            printf("parent: child was killed by signal: %d\n", WTERMSIG(status));
        } 
        else {
            printf("parent: child finished specifically (status: %d)\n", status);
        }

        printf("parent: final message AFTER child has finished.\n");
        exit(0);
    }

    return 0;
}