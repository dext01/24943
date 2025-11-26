#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

int main() {
    struct termios saved_attributes, new_attributes;
    char ch;  // <--- ВОТ ЭТОЙ СТРОКИ НЕ ХВАТАЛО

    // Сохраняем старые настройки
    tcgetattr(STDIN_FILENO, &saved_attributes);

    new_attributes = saved_attributes;

    // Отключаем ожидание Enter (канонический режим)
    new_attributes.c_lflag &= ~ICANON; 
    
    // Настраиваем чтение по 1 символу
    new_attributes.c_cc[VMIN] = 1;
    new_attributes.c_cc[VTIME] = 0;

    // Применяем новые настройки
    tcsetattr(STDIN_FILENO, TCSANOW, &new_attributes);

    printf("Вы любите учиться и узнавать что-то новое, интересное для вас? (y/n)? ");
    fflush(stdout);

    // Читаем символ
    read(STDIN_FILENO, &ch, 1);

    printf("\nВы нажали: %c\n", ch);

    // Возвращаем настройки как было (ОБЯЗАТЕЛЬНО!)
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);

    return 0;
}
