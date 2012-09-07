/* 网络通信的相关细节, IO复用处理, 与服务端连接的信息,  传输层细节, 如:TCP */
#ifndef _CONN_H_
#define _CONN_H_

#include <string.h>
#include <stddef.h>
#include <stdint.h> /* c99, for uint64_t */

#include <sys/select.h> /* select() */
#include <sys/time.h> /* struct timeval */

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ADDR_LEN 128
#define mbzero(base,len) memset(base, 0, len)

typedef int sock_t;

typedef enum _conn_state_t {
    DISCONNECTED, 
    CONNECTING, 
    CONNECTED
} conn_state_t;

typedef struct _conn_t {
    sock_t fd;
    conn_state_t state;
    uint64_t sent;
    uint64_t recv;
} conn_t;

int init_net_state(const char *host, int port);
void getip(const char *url, char *ret_ip);
sock_t tcp_connect(const char *host, int port);
conn_t * get_connection();
int e_select(int , fd_set *, fd_set *, fd_set *, struct timeval *);
const conn_state_t get_state(const conn_t *con);
int uninit_net_state();

#ifdef __cplusplus
}
#endif

#endif /* _CONN_H_ */
