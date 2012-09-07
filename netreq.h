/* 网络通信层的请求及回调的数据结构, 及相应逻辑处理函数的定义 */
#ifndef _NETREQ_H_
#define _NETREQ_H_

#include "common.h"

/* netagent 层请求事件 */
typedef enum {
    SV_LOGIN,
    SV_REGISTER
} req_srv_t;

/* netagent 层收到服务端回复事件类型 */
typedef enum {
    EV_LOGIN,
    EV_REGISTER
} nty_evt_t;

typedef struct _n_login_t 
{
    /* dialog mark */
    int idx;
    char usr[E_MINLINE];
    char pwd[E_MINLINE];
    eclient_listener listener;
} n_login_t;

typedef struct _n_register_t
{
    /* dialog mark */
    int idx;
    char usr[E_MINLINE];
    char pwd[E_MINLINE];
    eclient_listener listener;
} n_register_t;

typedef struct _n_login_res_t
{
    int result;
} n_login_res_t;

typedef struct _n_register_res_t
{
    int result;
} n_register_res_t;

typedef struct _net_req_t {
    int cmd;
    int idx; /* dialog mark */
    void *req;
} net_req_t;

typedef struct _net_notify_t {
    int cmd;
    int idx; /* dialog mark */
    void *nty;
} net_notify_t;

int send_net_agent(const req_srv_t sv_type, void *base, size_t len);
int send_net_notify(const int, const nty_evt_t, void *, size_t);
void net_elogin(const req_srv_t sv_type, const n_login_t *login);
void net_eregister(const req_srv_t sv_type, const n_register_t *reg);

#endif /* _NETREQ_H_ */
