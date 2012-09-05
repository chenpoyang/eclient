#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "erequest.h"
#include "trigger.h"
#include "eparser.h"
#include "recver.h"
#include "netreq.h"
#include "conn.h"
#include "elog.h"

static char buf[MAX_RECV_BUF + 1];
static pthread_t thrd;

/**
 * @brief    把从con中接收到的字节数保存到buf中, buf长度为len
 *
 * @param    con    连接标识
 * @param    buf    接收缓冲
 * @param    len    缓冲区长度
 *
 * @return   返回接收到的字节数, 反之错误
 */
int eme_recv(conn_t *con, void *buf, size_t len)
{
    int err, rec_bytes, start;

    if (con->state != CONNECTED)
    {
        e_error("eme_recv", "not connected!");
        return -1;
    }

    start = 1;
    while (start)
    {
        rec_bytes = recv(con->fd, buf, len, 0);

        e_debug("eme_recv", "fd (%d) recv %d of %d", con->fd, rec_bytes, len);

        if (rec_bytes > 0)
        {
            con->recv += rec_bytes;

            return rec_bytes;
        }

        err = errno;

        if (EAGAIN == err || EINTR == err)
        {
            e_error("eme_recv", "recv() not ready!");
            continue;
        }
        else
        {
            e_error("eme_recv", "recv() error!");
            start = 0;
        }
    }

    return -1;
}

/* 守护接收线程 */
void *daemon_recver(void *arg)
{
    int recv_bytes, start = 0, max_fd = -2, ready;
    conn_t *con = (conn_t*)arg;
    fd_set rset; /* 读集合 */
// struct timeval tv; /* 等待1秒 */

    e_debug("daemon_recver", "revcer thread start!");

    start = 1;
    FD_SET(con->fd, &rset);
    max_fd = con->fd > max_fd ? con->fd + 1 : max_fd + 1;
//    tv.tv_sec = 1;
//    tv.tv_usec = 1000;
    ready = 0;
    while (start)
    {
        e_debug("daemon_recver", "start select() now!");
        /* 默认阴塞式 */
        ready = e_select(max_fd, &rset, NULL, NULL, NULL);
        e_debug("my_debug", "ready = [%d]", ready);
        if (ready)
        {
            recv_bytes = eme_recv(con, buf, MAX_RECV_BUF);
            buf[recv_bytes] = '\0';
            /* 交给协议解析模块 */
            e_decompress(buf, recv_bytes + 1);
            e_debug("daemon_recver", "select ready[%d], received[%d]",
                    ready, recv_bytes);
        }
        else
        {
            /*　TODO 接收线程退出等 */
        }
    }

    e_debug("daemon_recver", "the recver thread exit.");
    pthread_exit(NULL);
}

/**
 * @brief   根据相应的连接, 初始化接收器守护线程
 *
 * @param   con     :   标识网络连接的参数
 */
void init_recver_thrd(const conn_t *con)
{
    if (con->state != CONNECTED)
    {
        e_error("init_recver_thrd", "not connect, con->state[%s]", "DISCONNECTED");
        return;
    }

    e_debug("init_thrd", "starting a new thread!");
    pthread_create(&thrd, NULL, daemon_recver, (void*)con);
}
