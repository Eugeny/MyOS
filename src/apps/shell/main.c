#include <stdio.h>
#include <unistd.h>

#include <sys/stat.h>
#include <dirent.h>


int main() {
    printf("MyOS shell\n");
    char s[256];

    dirent* dit;
    DIR *d = opendir("/");
	while ((dit = readdir(d)) != NULL)
        {
                printf("\n%s", dit->st_name);
        }

    while (1) {
        printf("$ ");
        fflush(stdout);
        scanf("%s", s);
        printf(">> %s\n", s);
    }
}
