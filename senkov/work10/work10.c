#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <команда> [аргументы...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("Parent: Preparing to launch command: %s\n", argv[1]);

    // новый процесс
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    // код для дочернего процесса
    if (pid == 0) {
        printf("Child [%d]: Executing command \"%s\"\n", getpid(), argv[1]);

        // запускаем команду, которую передал пользователь.
        // &argv[1] - это указатель на массив аргументов, начиная со второго элемента.
        // сделано тк это позволяет передать саму команду одной структурой.
        execvp(argv[1], &argv[1]);

        // Если программа дошла до этой строки, значит execvp не сработал (ошибка запуска)
        perror("execvp");
        exit(127);
    } 
    // блок кода для родителя
    else {
        int status;

        printf("Parent [%d]: Created child with pid %d\n", getpid(), pid);
        printf("Parent: Waiting for child to finish...\n");

        // блокируем родителя и ждем завершения дочернего процесса (pid)
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return EXIT_FAILURE;
        }

        printf("\n--- Child Process Report ---\n");
        
        // как именно завершился ребенок
        if (WIFEXITED(status)) {
            // если процесс завершился нормально (через return или exit)
            printf("Parent: Child exited normally with code: %d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            // если процесс был убит сигналом извне
            printf("Parent: Child was killed by signal: %d\n", WTERMSIG(status));
        }
        else {
            // (процесс остановлен, но не убит)
            printf("Parent: Child stopped with status: 0x%x\n", status);
        }

        printf("Parent: Final message AFTER child has finished.\n");
    }

    return 0;
}
