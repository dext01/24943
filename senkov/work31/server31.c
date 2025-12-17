#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

// Путь к файлу Unix domain socket
#define SOCKET_PATH "/tmp/uds_socket_31"
// Размер буфера для приёма данных от клиента
#define BUFFER_SIZE 256

/*
 * Преобразует строку в верхний регистр.
 * Безопасно обрабатывает NULL и использует unsigned char
 * для корректной работы с не-ASCII символами в разных локалях.
 */
void to_upper_case(char *str) {
    if (str == NULL) return;
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        str[i] = (char)toupper((unsigned char)str[i]);
    }
}

int main() {
    // Дескриптор серверного сокета
    int listen_fd;
    // Максимальный дескриптор для select() — требуется для корректной работы
    int max_fd;
    // Адрес сервера (Unix domain socket)
    struct sockaddr_un server_addr;
    // Набор файловых дескрипторов для мониторинга (постоянный)
    fd_set read_fds;
    // Временная копия для передачи в select() (он её модифицирует)
    fd_set temp_fds;
    // Буфер для приёма данных
    char buffer[BUFFER_SIZE];
    // Вспомогательная переменная для итерации по дескрипторам
    int fd;

    // Удаляем старый сокет-файл, если он существует
    // Важно: иначе bind() завершится ошибкой
    unlink(SOCKET_PATH);

    // Создаём Unix domain сокет потокового типа (SOCK_STREAM)
    if ((listen_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Инициализируем структуру адреса нулями (обязательно!)
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    // Копируем путь в структуру, оставляя место для '\0'
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Привязываем сокет к файловой системе
    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    // Переводим сокет в режим прослушивания (макс. 5 соединений в очереди)
    if (listen(listen_fd, 5) == -1) {
        perror("listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("Сервер 31: Слушаю на %s\n", SOCKET_PATH);

    // Инициализируем набор дескрипторов: сначала только слушающий сокет
    FD_ZERO(&read_fds);
    FD_SET(listen_fd, &read_fds);
    max_fd = listen_fd; // отслеживаем максимальный fd для select()

    // Основной цикл обработки событий
    while (1) {
        // select() модифицирует переданный fd_set, поэтому делаем копию
        temp_fds = read_fds;

        // Ждём событий на любом из дескрипторов
        // Первый аргумент: max_fd + 1 (требование POSIX)
        int activity = select(max_fd + 1, &temp_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select");
            break; // выходим при критической ошибке
        }

        // Проверяем, есть ли новое подключение (событие на listen_fd)
        if (FD_ISSET(listen_fd, &temp_fds)) {
            int client_fd = accept(listen_fd, NULL, NULL);
            if (client_fd == -1) {
                perror("accept");
                continue; // игнорируем ошибку, продолжаем работу
            }
            printf("Клиент подключился (fd=%d)\n", client_fd);
            // Добавляем новый клиентский дескриптор в набор
            FD_SET(client_fd, &read_fds);
            // Обновляем максимальный дескриптор
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
        }

        // Проверяем все остальные дескрипторы (клиенты)
        for (fd = 0; fd <= max_fd; fd++) {
            // Пропускаем слушающий сокет (он уже обработан выше)
            if (fd == listen_fd) continue;
            // Есть ли активность на этом клиентском сокете?
            if (FD_ISSET(fd, &temp_fds)) {
                // Читаем данные от клиента
                ssize_t bytes_read = recv(fd, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_read > 0) {
                    // Завершаем строку нулём и выводим
                    buffer[bytes_read] = '\0';
                    printf("Получено: \"%s\"\n", buffer);
                    // Преобразуем в верхний регистр
                    to_upper_case(buffer);
                    printf("Верхний регистр: \"%s\"\n", buffer);
                } else {
                    // Клиент закрыл соединение или ошибка
                    printf("Клиент (fd=%d) отключился\n", fd);
                    close(fd);
                    FD_CLR(fd, &read_fds); // убираем из набора
                }
            }
        }
    }

    // Очистка ресурсов перед завершением
    close(listen_fd);
    unlink(SOCKET_PATH); // удаляем файл сокета из ФС
    return 0;
}
