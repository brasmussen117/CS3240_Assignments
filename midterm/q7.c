#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *const s_names[] = { "Dib", "Gaz", "Zim" };

int main(int argc, char **argv) {
    char **names = malloc(sizeof(char*));

    for(int i = 0; i < 3; ++i) {
        names[i] = malloc(4);
        strncpy(names[i], s_names[i], 4);
    }

    printf("Hello, %s!\n", names[0]);
    printf("Hello, %s!\n", names[1]);
    printf("Hello, %s!\n", names[2]);

    free(names[0]);
    free(names[1]);
    free(names[2]);
    free(names);

    return 0;
}