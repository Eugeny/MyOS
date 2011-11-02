#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
    printf("MyOS shell\n");
    char s[256];
    while (1) {
        printf("$ ");
        fflush(stdout);
        scanf("%s", s);
        printf(">> %s\n", s);
    }
}
