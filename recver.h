#ifndef _RECVER_H_
#define _RECVER_H_

#include <stddef.h>
#include "conn.h"

#define MAX_RECV_BUF 1500

void *daemon_recver(void *arg);
int eme_recv(conn_t *con, void *buf, size_t len);
void init_recver_thrd(const conn_t *con);

#endif /* _RECVER_H_ */