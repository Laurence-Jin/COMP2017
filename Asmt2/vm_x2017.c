#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Exception: Error not caught and handled!");
        return -1;
    }
    if (argv[1] == NULL) {
        printf("Exception: Error not caught and handled!");
        return -1;
    }
    printf("0");
    return 0;
}