#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

#define PORT "9000"
#define DATA_FILE "/var/tmp/aesdsocketdata"
#define BUFFER_SIZE 1024

volatile sig_atomic_t caught_sig = 0;

void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        caught_sig = 1;
    }
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    if (chdir("/") < 0) {
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    int devnull = open("/dev/null", O_RDWR);
    if (devnull != -1) {
        dup2(devnull, STDIN_FILENO);
        dup2(devnull, STDOUT_FILENO);
        dup2(devnull, STDERR_FILENO);
    }
}

int main(int argc, char *argv[]) {
    bool run_as_daemon = false;
    
    if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        run_as_daemon = true;
    }
    openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int server_fd;
    struct addrinfo hints, *servinfo, *p;
    int yes = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, PORT, &hints, &servinfo) != 0) {
        syslog(LOG_ERR, "getaddrinfo failed");
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server_fd == -1) continue;

        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            syslog(LOG_ERR, "setsockopt failed");
            return -1;
        }

        if (bind(server_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(server_fd);
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        syslog(LOG_ERR, "Failed to bind to port %s", PORT);
        return -1;
    }
    if (run_as_daemon) {
        daemonize();
    }

    if (listen(server_fd, 10) == -1) {
        syslog(LOG_ERR, "Listen failed");
        return -1;
    }

    while (!caught_sig) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);

        if (client_fd == -1) {
            if (caught_sig) break; 
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);

        size_t current_buf_size = BUFFER_SIZE;
        char *rx_buffer = (char *)malloc(current_buf_size);
        if (!rx_buffer) {
            syslog(LOG_ERR, "Malloc failed");
            close(client_fd);
            continue;
        }
        
        size_t total_received = 0;
        ssize_t bytes_received;

        while ((bytes_received = recv(client_fd, rx_buffer + total_received, current_buf_size - total_received - 1, 0)) > 0) {
            total_received += bytes_received;
            rx_buffer[total_received] = '\0';

            if (strchr(rx_buffer, '\n') != NULL) {
                int data_fd = open(DATA_FILE, O_CREAT | O_WRONLY | O_APPEND, 0644);
                if (data_fd == -1) {
                    syslog(LOG_ERR, "Failed to open data file for writing");
                    break;
                }
                write(data_fd, rx_buffer, total_received);
                close(data_fd);

                data_fd = open(DATA_FILE, O_RDONLY);
                if (data_fd != -1) {
                    char tx_buffer[BUFFER_SIZE];
                    ssize_t bytes_read;
                    while ((bytes_read = read(data_fd, tx_buffer, sizeof(tx_buffer))) > 0) {
                        send(client_fd, tx_buffer, bytes_read, 0);
                    }
                    close(data_fd);
                }

                memset(rx_buffer, 0, current_buf_size);
                total_received = 0;
            } else {
                current_buf_size *= 2;
                char *new_buffer = (char *)realloc(rx_buffer, current_buf_size);
                if (!new_buffer) {
                    syslog(LOG_ERR, "Realloc failed (heap exhausted)");
                    break;
                }
                rx_buffer = new_buffer;
            }
        }

        free(rx_buffer);
        close(client_fd);
        syslog(LOG_INFO, "Closed connection from %s", client_ip);
    }
    syslog(LOG_INFO, "Caught signal, exiting");
    close(server_fd);
    remove(DATA_FILE);
    closelog();

    return 0;
}
