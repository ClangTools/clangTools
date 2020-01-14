
#ifdef _WIN32
#include <Windows.h>
#else

#include <zconf.h>

#endif

#include <cstdlib>
#include <cstdio>
#include <cstring>

#if false
int main(int argc, char **argv) {
    pid_t pid = 0;
    int inpipefd[2];
    int outpipefd[2];
    char buf[256];
    char msg[256];
    int status;

    pipe(inpipefd);
    pipe(outpipefd);
    pid = fork();
    if (pid == 0) {
        // Child
        dup2(outpipefd[0], STDIN_FILENO);
        dup2(inpipefd[1], STDOUT_FILENO);
        dup2(inpipefd[1], STDERR_FILENO);

        //ask kernel to deliver SIGTERM in case the parent dies
//        prctl(PR_SET_PDEATHSIG, SIGTERM);

        //replace tee with your process
        execl("/usr/bin/tee", "tee", (char *) nullptr);
        // Nothing below this line should be executed by child process. If so,
        // it means that the execl function wasn't successfull, so lets exit:
        exit(1);
    }
    // The code below will be executed only by parent. You can write and read
    // from the child using pipefd descriptors, and you can send signals to
    // the process using its pid by kill() function. If the child process will
    // exit unexpectedly, the parent process will obtain SIGCHLD signal that
    // can be handled (e.g. you can respawn the child process).

    //close unused pipe ends
    close(outpipefd[0]);
    close(inpipefd[1]);

    // Now, you can write to outpipefd[1] and read from inpipefd[0] :
    while (1) {
        printf("Enter message to send\n");
        memset(msg, 0x00, sizeof(msg));
        scanf("%s", msg);
        if (strcmp(msg, "exit") == 0) break;

        write(outpipefd[1], msg, strlen(msg));
        memset(buf, 0x00, sizeof(buf));
        read(inpipefd[0], buf, 256);

        printf("Received answer: %s\n", buf);

    }

    kill(pid, SIGKILL); //send SIGKILL signal to the child process
    waitpid(pid, &status, 0);
    return 0;
}

#endif

void print_result(FILE *fp) {
    char buf[100];

    if (!fp) {
        return;
    }
    printf("\n>>>\n");
    while (memset(buf, 0, sizeof(buf)), fgets(buf, sizeof(buf) - 1, fp) != nullptr) {
        printf("%s", buf);
    }
    printf("\n<<<\n");
}

int main(int, char **) {
    FILE *fp = nullptr;
#ifdef _WIN32
    fp = _popen("read a && echo $a", "r+");
#else
    fp = popen("read a && echo $a", "r+");
#endif
    if (!fp) {
        exit(EXIT_FAILURE);
    }
    fputs("12\n", fp);
    print_result(fp);
#ifdef _WIN32
    _pclose(fp);
#else
    pclose(fp);
#endif
    return 0;
}