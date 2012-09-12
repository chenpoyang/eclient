#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "global.h"

typedef struct client {
    char nick[32];
    int con_fd;
}Client;

static pthread_t cli_thrd[1024];    /* 服务器并发线程 */
Client cli[MAX_LISTEN_QUEUE];   /* 客户端 */
static int cli_que_len;
static char buf[MAX_BUF_LEN];

void *recv_thrd(void *arg);   /* server for client */
void remove_client(const int client_id);    /* deal with client quit */
void get_dst_nick(const char *msg, char *ret);  /* 客户与客户之间的通信 */
int client_connected(const char *nick); /* 此昵称的客户在线 */


int main(int argc, char *argv[])
{
    struct sockaddr_in srv;
    int sock_fd, chk, con_fd;
    pthread_attr_t attr;
    char *host = "127.0.0.1", *port = "8888";

    if (argc != 3)
	{
        printf("use default host[%s] and port[%s]\r\n", host, port);
	}
    else
    {
        host = argv[1];
        port = argv[2];
    }

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket error!");
        return -1;
    }

    bzero(&srv, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port = htons(atoi(port));

    /* inet_pton: support ipv6 and ipv4, set the ipaddr */
    if (inet_pton(AF_INET, host, &srv.sin_addr) < 0)
    {
        printf("inet_pton error!");
        return -1;
    }

    chk = bind(sock_fd, (struct sockaddr*)&srv, sizeof(srv));
    if (chk < 0)
    {
        printf("bind error!");
        return -1;
    }

    chk = listen(sock_fd, MAX_LISTEN_QUEUE);
    if (chk < 0)
    {
        printf("listen error!");
        return -1;
    }

    /* 初始化线程参数 */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    cli_que_len = 0;
    while (1)
    {
        con_fd = accept(sock_fd, (struct sockaddr*)NULL, NULL);
        printf("STAT\t:a client(%d) connected!\n", con_fd);
        
        /* get the nickname of the client */
        cli[cli_que_len].con_fd = con_fd;
        strcpy(cli[cli_que_len].nick, "new user!");

        /* create server thread for a new client */
        chk = pthread_create(cli_thrd + cli_que_len, &attr, recv_thrd, cli + cli_que_len);
        if (chk)
        {
            printf("create thread error!");
            continue;
        }
        ++cli_que_len;
    }
    close(sock_fd);

    return 0;
}

void *recv_thrd(void *arg)   /* server for client */
{
    int sock_fd;
    int rec_bytes;
    time_t cur_time;
    Client *cli_ptr = NULL;
    char *ptr = NULL;
    char res_str[MAX_BUF_LEN + 1];

    cli_ptr = (Client *)arg;
    sock_fd = cli_ptr->con_fd;
    while ((rec_bytes = recv(sock_fd, buf, sizeof(buf), 0)) > 0)
    {
        buf[rec_bytes] = '\0';
        time(&cur_time);
        printf("RECV(%d)\t:[%s], server time:%s", 
                cli_ptr->con_fd, buf, ctime(&cur_time));

        time(&cur_time);
        strcpy(res_str, buf);
        strcat(res_str, " ");
        strcat(res_str, ctime(&cur_time));
        ptr = res_str;
        while (*ptr != '\0' && *ptr != '\n')
        {
            ++ptr;
        }
        if (*ptr == '\n')
        {
            *ptr = '\0';
        }
        send(sock_fd, res_str, strlen(res_str) + 1, 0);
        printf("SEND(%d)\t:[%s]\n", sock_fd, res_str);
    }

    close(sock_fd);
    remove_client(sock_fd);
    printf("STAT\t:a client(%d) quit!\n", cli_ptr->con_fd);

    pthread_exit(NULL);
}

void remove_client(const int client_id)    /* deal with client quit */
{
    int i, index;

    index = -1;
    for (i = 0; i < cli_que_len; ++i)
    {
        if (cli[i].con_fd == client_id)
        {
            index = i;
            break;
        }
    }

    if (index > 0)
    {
        for (i = index + 1; i < cli_que_len; ++i)
        {
            cli[i - 1].con_fd = cli[i].con_fd;
            strcpy(cli[i - 1].nick, cli[i].nick);
        }
    }
    --cli_que_len;
}

/* 此昵称的客户在线 */
int client_connected(const char *nick)
{
    int i;

    for (i = 0; i < cli_que_len; ++i)
    {
        if (strcpy(cli[i].nick, nick) == 0)
        {
            return 1;
        }
    }

    return 0;
}
