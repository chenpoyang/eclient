#include <errno.h>
#include <sys/socket.h>
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
int eme_send(conn_t *con, void * const buf, const size_t len)
{
    int snd_bytes, err, start;

    if (con->state != CONNECTED)
    {
        e_error("eme_send", "%s", "not connected!");
        return -1;
    }

    start = 1;
    while (start)
    {
        snd_bytes = send(con->fd, buf, len, 0);

        e_debug("eme_send", "fd (%d) send %d of %d, data = [%s]",
                con->fd, snd_bytes, len, buf);

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