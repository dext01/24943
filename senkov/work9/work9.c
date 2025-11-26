#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h> // Для кодов ошибок

int main(int argc, char *argv[])
{
    // 1. Проверка количества аргументов
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        fprintf(stderr, "Example: %s longfile.txt\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];

    // 2. НОВАЯ ПРОВЕРКА: Существует ли файл и доступен ли он для чтения?
    // F_OK - существует, R_OK - можно читать
    if (access(filename, F_OK | R_OK) == -1) {
        perror("Error: Cannot access file");
        // Не имеет смысла создавать процесс, если файла нет
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    // --- Дочерний процесс ---
    if (pid == 0) {
        printf("child [%d]: running cat for file \"%s\"\n",
               getpid(), filename);

        execlp("cat", "cat", filename, (char *)NULL);

        // Сюда попадем только если execlp не сработал (например, нет утилиты cat)
        perror("execlp");
        _exit(127);
    } 
    // --- Родительский процесс ---
    else {
        int status;

        printf("parent [%d]: created child with pid %d\n",
               getpid(), pid);
        printf("parent: waiting for child to finish...\n");

        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return EXIT_FAILURE;
        }

        // 3. НОВАЯ ПРОВЕРКА: Как именно завершился ребенок?
        
        if (WIFEXITED(status)) {
            // Процесс завершился нормально (return или exit)
            int exit_code = WEXITSTATUS(status);
            printf("parent: child exited normally with code: %d\n", exit_code);
            
            if (exit_code != 0) {
                printf("parent: warning - child reported an error (cat failed).\n");
            }
        } 
        else if (WIFSIGNALED(status)) {
            // Процесс был убит сигналом (например, kill -9)
            printf("parent: child was killed by signal: %d\n", WTERMSIG(status));
        } 
        else {
            // Прочие редкие случаи (например, остановлен ptrace)
            printf("parent: child finished specifically (status: %d)\n", status);
        }

        printf("parent: final message AFTER child has finished.\n");
    }

    return 0;
}
