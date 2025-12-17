#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/uds_socket_31"

int main(void) {
    int client_fd;
    struct sockaddr_un server_addr;
    char text[128];
    int pid = getpid();

    // Формируем уникальное сообщение с PID процесса
    snprintf(text, sizeof(text), "Hello from client %d!", pid);

    // Создаём Unix domain сокет
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Инициализируем структуру адреса
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Подключаемся к серверу
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Отправляем сообщение (включая завершающий '\0')
    send(client_fd, text, strlen(text) + 1, 0);
    printf("Клиент %d: отправил \"%s\"\n", pid, text);

    // Закрываем сокет и завершаемся
    close(client_fd);
    return 0;
}
