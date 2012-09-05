#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include "common.h"
#include "recver.h"
#include "conn.h"
#include "elog.h"

static conn_t conn = {-1, DISCONNECTED, 0};
static pthread_mutex_t _con_mtx = PTHREAD_MUTEX_INITIALIZER;

conn_t * get_connection()
{
    if (conn.state != CONNECTED)
    {
        e_error("get_connection", "not connected, state = [%s]",
                conn.state == DISCONNECTED ? "DISCONNECTED" : "CONNECTING");
    }
    
    return &conn;
}
/**
 * @brief  查询数据是否准备好
 * @param  nfds      最大套接字 + 1
 * @param  readfds   读集合
 * @param  writefds  写集合
 * @param  exceptfds 异常套接字集合
 * @param  timeout   如所有集合的套接字都没有数据, 则超时返回
 * @return 0, 超时; -1, 出错; > 0, 数据量到达
 */
int e_select(int nfds, fd_set *readfds, fd_set *writefds,
             fd_set *exceptfds, struct timeval *timeout)
{
    int n;

    if ((n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
    {
        e_error("e_select", "select error, nfds[%d]", nfds);
    }

    return n;
}
    
const conn_state_t get_state(const conn_t *con)
{
    conn_state_t state = DISCONNECTED;

    pthread_mutex_lock(&_con_mtx);
    state = con->state;
    pthread_mutex_unlock(&_con_mtx);

    return state;
}

/**
 * @brief   初始化连接
 *
 * @param   host    :   目标主机
 * @param   port    :   目标端口号
 *
 * @return  是否成功连接
 */
int init_net_state(const char *host, int port)
{
    int sofd;
    conn_t con;

    assert(host != NULL && strlen(host) > 0);

    con = conn;
    if (con.state == CONNECTED)
    {
        e_debug("init_net_state", "already connected!");
        
        return EME_OK;
    }

    sofd = tcp_connect(host, port);
    if (sofd > 0)
    {
        con.fd = sofd;
        con.state = CONNECTED;

        e_debug("init_net_state", "ok, already connected to host[%s]", host);
    }
    else
    {con.fd = -1;
        con.state = DISCONNECTED;
        e_error("init_net_state", "failed, unable connect to host[%s]", host);
    }

    pthread_mutex_lock(&_con_mtx);

    conn = con;

    pthread_mutex_unlock(&_con_mtx);

    /* 启动接收线程 */
    if (conn.state == CONNECTED)
    {
        e_debug("init_net_state", "connected, now start deamon recver");
        
        init_recver_thrd(&conn);
    }
    else
    {
        e_error("init_net_state", "start recver thread error, not connect!");
    }

    return get_state(&conn) ? EME_OK : EME_ERR;
}

/**
 * @brief   关闭网络连接
 *
 * @return  成功关闭返回0, 反之失败
 */
int uninit_net_state()
{
    int chk;

    pthread_mutex_lock(&_con_mtx);
    
    chk = shutdown(conn.fd,SHUT_RDWR);
    if (chk < 0)
    {
        printf("uninit_net_state: shutdown error!");
    }
    close(conn.fd);
    conn.fd = -1;
    conn.state = DISCONNECTED;

    pthread_mutex_unlock(&_con_mtx);
    
    return 0;
}

/**
 * @brief   与目标主机建立TCP连接
 *
 * @param   host    :   目标主机地址
 * @param   port    :   目标主机端口号
 *
 * @return  返回标识此socket连接的文件描述符
 */
sock_t tcp_connect(const char *host, int port)
{
    int sock_fd, chk;
    char ip_str[MAX_ADDR_LEN];
    struct sockaddr_in sa;

    assert(host != NULL && strlen(host) > 0);

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        e_error("conn", "socket error!");
        return -1;
    }

    getip(host, ip_str);
    mbzero(&sa, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    chk = inet_pton(AF_INET, ip_str, &sa.sin_addr);
    if (chk < 0)
    {
        printf("illegal ip address!\n");
        return -1;
    }

    chk = connect(sock_fd, (struct sockaddr *)&sa, sizeof(sa));
    if (chk < 0)
    {
        printf("connect error!\n");
        return -1;
    }

    return sock_fd;
}
/**
 * @brief   根据给定的url, 抽取域名并进行解析
 *
 * @param   url     :   包括域名的网址或纯域名
 * @param   ret_ip  :   以点分十进制返回的ip, eg:xxx.xxx.xxx.xxx
 */
void getip(const char *url, char *ret_ip)
{
    struct hostent *addr = NULL;
    char **ptr_alias = NULL;
    char ip_buf[16];

    assert(url != NULL && ret_ip != NULL);

    addr = gethostbyname(url);
    if (addr == NULL)
    {
        printf("illegal domain name!");
        exit(-1);
    }

    ptr_alias = addr->h_aliases;
    while (*ptr_alias != NULL)
    {
        printf("alias: %s\n", *ptr_alias);
        ++ptr_alias;
    }

    ptr_alias = addr->h_addr_list;
    while (*ptr_alias != NULL)
    {
        inet_ntop(addr->h_addrtype, *ptr_alias, ip_buf, sizeof(ip_buf));
        strcpy(ret_ip, ip_buf);
        return;
        ++ptr_alias;
    }

    strcpy(ret_ip, "");
}
