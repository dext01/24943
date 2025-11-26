#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

int main() {
    struct termios saved_attributes, new_attributes;

    tcgetattr(STDIN_FILENO, &saved_attributes);

    new_attributes = saved_attributes;

    new_attributes.c_lflag &= ~ICANON; 
    
    new_attributes.c_cc[VMIN] = 1;
    new_attributes.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &new_attributes);

    printf("Вы любите учиться и узнавать что-то новое, интересное для вас? (y/n)? ");
    fflush(stdout);

    read(STDIN_FILENO, &ch, 1);

    printf("\nВы нажали: %c\n", ch);

    tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);

    return 0;
}
