#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

// Путь к файлу Unix domain socket
#define SOCKET_PATH "/tmp/uds_socket_32"
// Размер буфера для приёма данных от клиента
#define BUFFER_SIZE 256
// Максимальное количество одновременных клиентов
#define MAX_CLIENTS 10

/*
 * Преобразует строку в верхний регистр.
 * Использует (unsigned char) для безопасной передачи в toupper(),
 * что важно при работе с локалями, содержащими не-ASCII символы.
 */
void to_upper_case(char *str) {
    if (str == NULL) return;
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        str[i] = (char)toupper((unsigned char)str[i]);
    }
}

int main() {
    // Дескриптор серверного (слушающего) сокета
    int listen_fd;
    // Структура адреса сервера (Unix domain)
    struct sockaddr_un server_addr;
    // Массив дескрипторов для poll()
    // Элемент 0 — слушающий сокет, остальные — клиенты
    struct pollfd fds[MAX_CLIENTS + 1];
    // Буфер для приёма данных
    char buffer[BUFFER_SIZE];
    // Текущее количество активных слотов в fds[] (для poll)
    int nfds = 1; // начинаем с одного (слушающий сокет)
    int i;

    // Удаляем старый сокет-файл, если он существует
    // Без этого bind() завершится ошибкой "Address already in use"
    unlink(SOCKET_PATH);

    // Создаём Unix domain сокет потокового типа (аналог TCP, но локальный)
    if ((listen_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Обнуляем структуру адреса — хорошая практика
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    // Копируем путь, оставляя место для завершающего '\0'
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Привязываем сокет к файловой системе
    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    // Переводим сокет в режим прослушивания
    // Очередь из 5 ожидающих подключений — достаточно для учебной задачи
    if (listen(listen_fd, 5) == -1) {
        perror("listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("Сервер 32: Слушаю на %s\n", SOCKET_PATH);

    // Инициализация массива pollfd:
    // Элемент 0 — серверный сокет, ожидаем события входящих подключений
    fds[0].fd = listen_fd;
    fds[0].events = POLLIN; // интересует только возможность чтения (новое подключение)

    // Инициализируем слоты для клиентов: fd = -1 означает "свободен"
    for (i = 1; i <= MAX_CLIENTS; i++) {
        fds[i].fd = -1;
        fds[i].events = POLLIN; // если клиент подключится, будем ждать данные от него
    }

    // Основной цикл обработки событий
    while (1) {
        // Ждём события на любом из зарегистрированных дескрипторов
        // -1 в таймауте = ждать бесконечно
        int ret = poll(fds, nfds, -1);
        if (ret < 0) {
            perror("poll");
            break; // критическая ошибка — выходим
        }

        // Проверяем, есть ли новое подключение (событие на слушающем сокете)
        if (fds[0].revents & POLLIN) {
            int client_fd = accept(listen_fd, NULL, NULL);
            if (client_fd == -1) {
                perror("accept");
                continue; // игнорируем ошибку, продолжаем работу
            }

            // Ищем свободный слот для нового клиента
            int found = 0;
            for (i = 1; i <= MAX_CLIENTS; i++) {
                if (fds[i].fd == -1) {
                    fds[i].fd = client_fd;
                    // Обновляем nfds: poll проверяет элементы [0, nfds)
                    if (i >= nfds) {
                        nfds = i + 1;
                    }
                    found = 1;
                    break;
                }
            }

            if (!found) {
                // Нет свободных слотов — отклоняем подключение
                close(client_fd);
                printf("Слишком много клиентов\n");
            }
        }

        // Обрабатываем данные от уже подключённых клиентов
        for (i = 1; i < nfds; i++) {
            if (fds[i].fd == -1) {
                continue; // слот свободен
            }
            if (fds[i].revents & POLLIN) {
                // Читаем данные от клиента
                ssize_t bytes_read = recv(fds[i].fd, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_read > 0) {
                    // Успешное чтение: завершаем строку и обрабатываем
                    buffer[bytes_read] = '\0';
                    printf("Получено: \"%s\"\n", buffer);
                    to_upper_case(buffer);
                    printf("Верхний регистр: \"%s\"\n", buffer);
                } else {
                    // Клиент отключился (bytes_read == 0) или ошибка
                    close(fds[i].fd);
                    fds[i].fd = -1; // помечаем слот как свободный
                    // nfds можно не уменьшать — poll игнорирует fd == -1
                }
            }
        }
    }

    // Очистка ресурсов при завершении
    close(listen_fd);
    unlink(SOCKET_PATH); // удаляем файл сокета из файловой системы
    return 0;
}
