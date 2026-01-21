#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>



void splitcommand(const char* inbuf, char argbuf[][512], int lettercount, int maxargs) {
    int arg = 0;

    int pos = 0;
    for (int letter = 0; letter < lettercount && inbuf[letter] != '\0'; letter++) {
        if (inbuf[letter] != ' ') {
            argbuf[arg][pos++] = inbuf[letter];
            argbuf[arg][pos] = '\0';
        } else {
            if (pos > 0) {
                arg++;
                pos = 0;
                if (arg >= maxargs) break;
            }
        }
    }
}


int main() {
    signal(SIGINT, SIG_IGN);


    char buf[512];

    char args[32][512] = {{0}};

    while (1) {
        char cwd[512] = {0};
        getcwd(cwd, sizeof(cwd));

        for (int i = 0; i < 32; i++)
            args[i][0] = '\0';


        printf("<-%s->-$ ", cwd);
        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = 0;

        splitcommand(buf, args, 512, 32);

        if (!strcmp("exit", args[0])) break;

        if (!strcmp("cd", args[0])) {
            if (chdir(args[1]) != 0) {
                perror("cd: ");
            }
        }

        char* argv[32] = {0};
        int i;
        for (i = 0; i < 32 && args[i][0] != '\0'; i++) {
            argv[i] = args[i];
        }
        argv[i] = NULL;

        if (strcmp("cd", args[0])) {
            pid_t pid = fork();
            if (pid == 0) {
                signal(SIGINT, SIG_DFL);
                execvp(args[0], argv);
                char errorbuf[1024];
                sprintf(errorbuf, "%s failed", args[0]);

                perror(errorbuf);
                return 1;
            } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);
            } else {
                perror("Fork failed");
            }
        }

        memcpy(args[0], buf, sizeof(buf));
    }

    return 0;
}
