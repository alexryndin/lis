#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>


int main(int argc, char** argv) {
    puts("lis version 0.1.0\n");
    puts("press ctrl+c to exit\n");

    while(1) {
        char* input = readline("lis> ");
        add_history(input);
        printf("you entered: %s\n", input);
        free(input);
    }

    return 0;
}
