#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "trigger.h"
#include "netreq.h"
#include "common.h"
#include "eparser.h"
#include "elistener.h"
#include "elog.h"

/* TODO, req.req的信息在网络层中要保存, 当有soccket回应时, 生新取回信息(dlgid!)
 * 再返回 net_notify_t 结构到ctrlagent中 生新进入g_cmd_info[dlgidx]中, 相当于ack!
 * 再进入相应的请求, 如ctrl_elogin(dlg_idx), 处理本次请的收尾(超时, 成功, 清理dialog..
 */
/**
 * @brief  当ctrlagent层需传递数据到netagent层时, 显式调用, 新的用户请求数据需处理
 * @param  sv_type 可供netagent处理的请求类型
 * @param  base    数据的起始地址
 * @param  len     数据的长度
 * @return int     EME_OK, EME_ERR, 意为向netagent传递数据成功或失败
 */
int send_net_agent(const req_srv_t sv_type, void *base, size_t len)
{
    net_req_t req;
    
    req.req = malloc(len);
    /* net_req_t 结构用于标识具体请求类型, eg:SV_LOGIN */
    req.cmd = sv_type;
    /* 保存ctrlagent包装后成 能让netagent认识的结构, must cp! */
    memcpy(req.req, base, len);

    e_debug("send_net_agent", "from ctrlagent, SV_TYPE[%d]", sv_type);
    
    return send_signal(NET_REQ_EVT, &req, sizeof(req),
                       CTRL_AGENT_ID, NET_AGENT_ID);
};

/**
 * @brief  事件监听到服务端的事件回复
 * @param  idx      会话id
 * @param  ev_type  有回应的监听类型
 * @param  base     数据的起始地址
 * @param  len      数据的长度
 * @return int, 分别为EME_OK, 或EME_ERR;
 */
int
send_net_notify(const int idx, const nty_evt_t ev_type, void *base, size_t len)
{
    net_notify_t nty = {ev_type};

    /* 接收服务端传过来的消息, e_compress(..)已申请内存, 不需cp, 直接修改指针 */
    nty.nty = base;

    e_debug("net_notify", "received data from server, send net notify");
    
    return send_signal(NET_NOTIFY_EVT,
                       &nty, sizeof(net_notify_t), -1, NET_AGENT_ID);
}

/**
 * @brief  向服务端请求的业务逻辑之一, 请求登入服务器
 * @param  sv_type 请求类型
 * @param  login   请求的数据
 * @return void
 */
void net_elogin(const req_srv_t sv_type, const n_login_t *login)
{
    char str[E_MAXLINE] = "";

    /* 注册登请求的监听器, 以便接收服务端响应的信息 */
    add_listener(sv_type, (void*)login, login_listener);
    
    e_debug("net_elogin", "add a new listener!");

    e_compress(sv_type, login, str, sizeof(str));
}

/**
 * @brief  向服务端请求的业务逻辑之一, 请求注册
 * @param  sv_type 请求类型
 * @param  reg     请求的数据
 * @return void
 */
void net_eregister(const req_srv_t sv_type, const n_register_t *reg)
{
    char str[E_MAXLINE] = "";

    /* 注册登陆请求的监听器, 以便接收服务端响应的信息 */
    add_listener(sv_type, (void*)reg, register_listener);

    e_debug("net_eregister", "add a new listener!");

    e_compress(sv_type, reg, str, sizeof(str));
}

void net_esnd_msg(const req_srv_t sv_type, const n_send_msg_t *n_snd)
{
    char str[E_MAXLINE] = "";

    /* 注册发消息请求监听器, 以便接收服务端响应的信息 */
    add_listener(sv_type, (void*)n_snd, send_msg_listener);

    e_debug("net_esnd_msg", "add <net_esnd_msg> listener!");

    e_compress(sv_type, n_snd, str, sizeof(str));
    
    return;
}
