#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>


void run(char* s) {
	int pid = exec(s, NULL, 0, NULL);
	waitpid(pid);
}

int main() {
    printf("MyOS shell\n");
    char s[256];

	char cwd[256];
	strcpy(cwd, "/");

    while (1) {
        printf("%s $ ", cwd);
        fflush(stdout);
        gets(s);
		   
		char *cmd = strtok(s, " ");

        if (strcmp(cmd, "ls") == 0) {
    		struct dirent* dit;
		    DIR *d = opendir(cwd);
			while ((dit = readdir(d)) != NULL)
                printf("%s\n", dit->d_name);
			closedir(d);
		} else if (strcmp(cmd, "cd") == 0) {
			char *f = strtok(NULL, " ");

			if (strcmp(f, "..") == 0 ) {
				if (strlen(cwd) > 1) {
					while (cwd[strlen(cwd)-1] != '/')
						cwd[strlen(cwd)-1] = 0;
					if (strlen(cwd) > 1)
						cwd[strlen(cwd)-1] = 0;
				}
			} else {
				if (strlen(cwd) > 1)
					strcat(cwd, "/");
				strcat(cwd, f);
			}
		} else if (strcmp(cmd, "exec") == 0) {
			char *f = strtok(NULL, " ");
			run(f);
        } else {
        	printf("Unknown command\n");
        }
    }
}
