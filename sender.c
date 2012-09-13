#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include "jsonpro.h"
#include "sender.h"
#include "conn.h"
#include "elog.h"

/* 从conn.c上初始化 */
static const conn_t *con = NULL;

void init_sender()
{
    con = get_connection();
}

/**
 * @brief    把长度为len的缓冲数据发送到con
 *
 * @param    con    连接标识
 * @param    buf    发送缓冲
 * @param    len    缓冲区长度
 *
 * @return    
cem */
int eme_send(conn_t *con, char * const buf, const size_t len)
{
    int snd_bytes, err, start;
    char str[E_MAXLINE + 1];
#ifdef D_EME_SOCKET
    char *_ptr = NULL;
#endif

    if (con->state != CONNECTED)
    {
        e_error("eme_send", "%s", "not connected!");
        return -1;
    }

    start = 1;
    while (start)
    {
        snprintf(str, E_MAXLINE, "%s%s", buf, EME_JSON_SPLIT);
        /* 'strlen(str)', not 'strlen(str) + 1' */
        snd_bytes = send(con->fd, str, strlen(str), 0);
        str[snd_bytes] = '\0';
#ifdef D_EME_SOCKET
        _ptr = strstr(str, EME_JSON_SPLIT);
        if (_ptr != NULL)
        {
            str[_ptr - str] = 0;
        }
        printf("SEND: [%s]\n", str);
#endif
        e_debug("eme_send", "fd (%d) send (%d) of (%d), data = [%s]",
                con->fd, snd_bytes, len, str);

        if (snd_bytes > 0)
        {
            con->sent += snd_bytes;

            return snd_bytes;
        }

        err = errno;

        if (err == EAGAIN || err == EINTR)
        {
            e_error("eme_send", "send() not ready");
            
            continue;
        }
        else
        {
            e_error("eme_send", "send() error");
                        
            start = 0;
        }
    }

    return -1;
}
