#include "systemcalls.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

bool do_system(const char *cmd)
{
    if (cmd == NULL) {
        return system(cmd) != 0;
    }

    int status = system(cmd);

    if (status == -1) {
        return false;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return true;
    }

    return false;
}

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

    fflush(stdout);

    pid_t pid = fork();

    if (pid == -1) {
        va_end(args);
        return false;
    } else if (pid == 0) {
        execv(command[0], command);
        exit(EXIT_FAILURE);
    } else {
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            va_end(args);
            return false;
        }
        va_end(args);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return true;
        }
        return false;
    }
}

bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

    int fd = open(outputfile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (fd < 0) {
        va_end(args);
        return false;
    }

    fflush(stdout);

    pid_t pid = fork();

    if (pid == -1) {
        close(fd);
        va_end(args);
        return false;
    } else if (pid == 0) {
        if (dup2(fd, STDOUT_FILENO) < 0) {
            exit(EXIT_FAILURE);
        }
        close(fd);
        execv(command[0], command);
        exit(EXIT_FAILURE);
    } else {
        close(fd);
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            va_end(args);
            return false;
        }
        va_end(args);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return true;
        }
        return false;
    }
}
