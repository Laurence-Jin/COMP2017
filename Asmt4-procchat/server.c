#include "server.h"

#define CONNECT 0
#define SAY 1
#define SAYCON 2
#define RECV 3
#define RECVCON 4
#define DISCONN 7

#define MAX_PERMISSION_MODE (0777)

const char* GEVENT = "gevent";

#define LENGTH_OF_MESSAGE 2048
#define LENGTH_OF_SAY_MESSAGE_PAYLOAD 1790
#define LENGTH_OF_SAYCON_MESSAGE_PAYLOAD 1789
#define LENGTH_OF_TYPE 2
#define LENGTH_OF_ID 256
#define LENGTH_OF_DOMAIN 256
#define FIFO_PATH_LEN (256 + 256 + 10)


void myperror(const char* formatter, const char* obj) {
    char error_buffer[512];
    sprintf(error_buffer, formatter, obj);
    perror(error_buffer);
}

int open_fifo(const char* path, int mode) {
    int fd = open(path, mode);
    if (fd == -1 && errno != EEXIST)
    {
        myperror("open %s failed", path);
        exit(1);
    }
    return fd;
}

void my_mkfifo(const char* fifo_path) {
    int result = mkfifo(fifo_path, MAX_PERMISSION_MODE);
    if (result != 0 && errno != EEXIST)
    {
        myperror("mkfifo error for %s", fifo_path);
        exit(1);
    }
}

void create_fifo(char* fifo_path, char* domain_str, char* id_str, char* post_fix) {
    memset(fifo_path, 0, FIFO_PATH_LEN);
    strcat(fifo_path, domain_str);
    strcat(fifo_path, "/");
    strcat(fifo_path, id_str);
    strcat(fifo_path, post_fix);

    my_mkfifo(fifo_path);
}

void relay_message(char* handler_write_buffer, char* fifo_name, char* domain_str, char* my_rd_fifo_path) {
    char fifo_path[FIFO_PATH_LEN];
    memset(fifo_path, 0, FIFO_PATH_LEN);
    strcat(fifo_path, domain_str);
    strcat(fifo_path, "/");
    strcat(fifo_path, fifo_name);

    if (strcmp(my_rd_fifo_path, fifo_path) != 0) {
        int fifo_fd = open_fifo(fifo_path, O_WRONLY);
        write(fifo_fd, handler_write_buffer, LENGTH_OF_MESSAGE);
        close(fifo_fd);
    }
}

void client_handler_main(char* wr_fifo_path, char* rd_fifo_path, char* id_str, char* domain_str) {
    for (;;) {
        char handler_read_buffer[LENGTH_OF_MESSAGE];
        memset(handler_read_buffer, 0, LENGTH_OF_MESSAGE);

        int handler_read_fd = open_fifo(wr_fifo_path, O_RDONLY);
        read(handler_read_fd, handler_read_buffer, LENGTH_OF_MESSAGE);
        close(handler_read_fd);

        if (handler_read_buffer[0] == SAY || handler_read_buffer[0] == SAYCON) {
            char handler_write_buffer[LENGTH_OF_MESSAGE];
            memset(handler_write_buffer, 0, LENGTH_OF_MESSAGE);

            uint16_t* handler_write_buffer__type_start = (uint16_t*) handler_write_buffer;
            if (handler_read_buffer[0] == SAY) {
                *handler_write_buffer__type_start = RECV;
            } 
            else {
                *handler_write_buffer__type_start = RECVCON;
            }

            char* handler_write_buffer__id_start = handler_write_buffer + LENGTH_OF_TYPE;
            memcpy(handler_write_buffer__id_start, id_str, LENGTH_OF_ID);

            char* handler_write_buffer__message_start = handler_write_buffer + LENGTH_OF_TYPE + LENGTH_OF_ID;
            if (handler_read_buffer[0] == SAY) {
                memcpy(handler_write_buffer__message_start, handler_read_buffer + LENGTH_OF_TYPE, LENGTH_OF_SAY_MESSAGE_PAYLOAD);
            } 
            else {
                memcpy(handler_write_buffer__message_start, handler_read_buffer + LENGTH_OF_TYPE, LENGTH_OF_SAYCON_MESSAGE_PAYLOAD);
                handler_write_buffer[LENGTH_OF_MESSAGE - 1] = handler_read_buffer[LENGTH_OF_MESSAGE - 1];
            }

            DIR* mydir = opendir(domain_str);
            if (mydir == NULL) {
                myperror("handler opendir %s failed", domain_str);
                exit(1);
            }
            struct dirent* mydirent;
            mydirent = readdir(mydir);
            while (mydirent != NULL) {
                int len = strlen(mydirent->d_name);
                if (mydirent->d_name[0] != '.' && mydirent->d_name[len-2] == 'R') {
                    relay_message(handler_write_buffer, mydirent->d_name, domain_str, rd_fifo_path);
                }
                mydirent = readdir(mydir);
            }
            closedir(mydir);
        }
        else if (handler_read_buffer[0] == DISCONN) {
            kill(getppid(), SIGUSR1);
            unlink(wr_fifo_path);
            unlink(rd_fifo_path);
            exit(0);
        } 
        else {
            fprintf(stderr, "[handler]unrecognized message type: %d\n", handler_read_buffer[0]);
        }
    }
    exit(0);
}

void child_exit_handler(int signo) {
    wait(NULL);
}

int main(int argc, char* argv[]) {
    signal(SIGUSR1, child_exit_handler);
    my_mkfifo(GEVENT);
    for (;;) {
        int gevent_fifo_fd = open_fifo(GEVENT, O_RDONLY);
        char gevent_buffer[LENGTH_OF_MESSAGE];
        memset(gevent_buffer, 0, LENGTH_OF_MESSAGE);
        int nbytes = read(gevent_fifo_fd, gevent_buffer, LENGTH_OF_MESSAGE); 
        if (nbytes > 0) { 
            if (gevent_buffer[0] == CONNECT) {
                int pid = fork();
                if (pid == 0) {
                    char id_str[LENGTH_OF_ID + 1];
                    memset(id_str, 0, LENGTH_OF_ID + 1);
                    char* gevent_buffer__id_start = &gevent_buffer[LENGTH_OF_TYPE];
                    memcpy(id_str, gevent_buffer__id_start, LENGTH_OF_ID);

                    char domain_str[LENGTH_OF_DOMAIN + 1];
                    memset(domain_str, 0, LENGTH_OF_DOMAIN + 1);
                    char* gevent_buffer__domain_start = &gevent_buffer[LENGTH_OF_TYPE + LENGTH_OF_ID];
                    memcpy(domain_str, gevent_buffer__domain_start, LENGTH_OF_DOMAIN);

                    mkdir(domain_str, MAX_PERMISSION_MODE);

                    char rd_fifo_path[FIFO_PATH_LEN];
                    create_fifo(rd_fifo_path, domain_str, id_str, "_RD");
                    char wr_fifo_path[FIFO_PATH_LEN];
                    create_fifo(wr_fifo_path, domain_str, id_str, "_WR");

                    client_handler_main(wr_fifo_path, rd_fifo_path, id_str, domain_str);
                }
                else if (pid < 0) {
                    perror("fork failed");
                }
                else {
                    // do nothing for parent process
                }
            }
            else {
                fprintf(stderr, "[gevent]unrecognized message type: %d\n", gevent_buffer[0]);
            }
        }
        else {
            // do nothing if read return 0 or <0.
        }
        close(gevent_fifo_fd);
    }
    return 0;
}
