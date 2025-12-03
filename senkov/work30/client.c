#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/uds_socket_30" 
#define TEXT_TO_SEND "ЭтО КруТой сОКет! пРиВЕт всЕм, КТО любИт яЗык ПРоГРаМмиРОваНИЯ с, И ПредмЕТ опеРАцИОнНые СИСтЕМы!"

int main() {
    int client_fd;
    struct sockaddr_un server_addr;
    char *text = TEXT_TO_SEND;
    ssize_t bytes_sent;

    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Ошибка при создании сокета");
        exit(EXIT_FAILURE);
    }

    // Заполнение структуры адреса Сервера
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    
    // Копируем путь к файлу сокета
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    printf("Клиент: Попытка подключения к UDS: %s\n", SOCKET_PATH);

    // 2. Установление соединения
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Ошибка при подключении к Серверу. Убедитесь, что Сервер запущен!");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Клиент: Соединение установлено.\n");

    // 3. Отправка данных
    // Отправляем текст, включая завершающий нулевой символ
    bytes_sent = send(client_fd, text, strlen(text) + 1, 0); 
    
    if (bytes_sent == -1) {
        perror("Ошибка при отправке данных");
    } else {
        printf("Клиент: Отправлено %ld байт.\n", bytes_sent);
        printf("Клиент: Отправленный текст: \"%s\"\n", text);
    }

    // 4. Разрыв соединения и завершение
    // Закрытие сокета клиентом уведомит сервер о разрыве соединения
    close(client_fd);
    printf("Клиент: Соединение закрыто. Клиент завершает работу.\n");

    return 0;
}
