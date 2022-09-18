#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdint.h>
#include "server.h"

#define CONNECT 0
#define SAY 1
#define SAYCON 2
#define RECV 3
#define RECVCON 4
#define DISCONN 7

struct connect_message {
    uint16_t type;
    char id[256];
    char domain[256];
    char padding[2048 - 2 - 256 - 256];
};
struct say_message {
    uint16_t type;
    char content[1790];
    char padding[2048 - 2 - 1790];
};
struct say_cont {
    uint16_t type;
    char content[1789];
    char padding[2048 - 2 - 1789 - 1];
    char terminator;
};
struct rcv_message {
    uint16_t type;
    char ident[256];
    char content[1790];
};
struct rcv_cont {
    uint16_t type;
    char ident[256];
    char content[1789];
    char terminator;
};

#define CLIENT_ID_1 ("client_a")
#define CLIENT_ID_2 ("client_b")

int main(int argc, char const *argv[])
{
    // use fork to create two client in one domain, they can communicate with each other.

    // client1
    if(fork() == 0) {
        int fd = open("gevent", O_WRONLY);
        printf("[DEBUG-1]fd=%d\n", fd);
        struct connect_message conn_msg = {0, CLIENT_ID_1, "domain1", "padding......"};
        write(fd, &conn_msg, 2048);
        close(fd);

        usleep(8000);

        int wfd = open("domain1/client_a_WR", O_WRONLY);
        struct say_message say_msg = {1, "test say from client_a", "padding......"};
        write(wfd, &say_msg, 2048);
        close(wfd);

        sleep(2);

        exit(0);
    }

    // client2
    if(fork() == 0) {
        int fd = open("gevent", O_WRONLY);
        printf("[DEBUG-2]fd=%d\n", fd);
        struct connect_message conn_msg = {0, CLIENT_ID_2, "domain1", "padding......."};
        write(fd, &conn_msg, 2048);
        close(fd);

        usleep(8000);

        int rfd = open("domain1/client_b_RD", O_RDONLY);
        struct rcv_message rcv_msg = {0};
        read(rfd, &rcv_msg, 2048);
        close(rfd);

        printf("[client_b]recv msg: %hu\n", rcv_msg.type);
        printf("[client_b]recv msg: %s\n", rcv_msg.ident);
        printf("[client_b]recv msg: %s\n", rcv_msg.content);

        sleep(2);

        exit(0);
    }

    sleep(5);
    // parent work here
    for (size_t i = 0; i < 2; i++)
    {
        wait(NULL);
    }
    
    
    return 0;
}
