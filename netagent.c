#include <stdlib.h>
#include <string.h>
#include "netagent.h"
#include "ctrlagent.h"
#include "erequest.h"
#include "common.h"
#include "trigger.h"
#include "sender.h"
#include "netreq.h"
#include "elog.h"
#include "conn.h"

static void deal_net_req(void *base, size_t len);
static int deal_ctrl_net(const req_srv_t type, void *base);
static void deal_net_data(const void *base, size_t len);

/* entrance */
void netagent(int evt, void *base, size_t len, int cmd)
{
    net_req_t * req = NULL;
    
    switch (evt)
    {
        case NET_REQ_EVT:
            req = (net_req_t *)base;
            deal_net_req(req, len);
            break;
            
        /* 新的数据到达, 已进行协议解析,转换成相应数据结构, 详见netreq.h */
        case NET_NOTIFY_EVT: 
            deal_net_data(base, len);
            break;
            
        default:
            break;
    }
}

/**
 * @brief  收到服务端发送过来的数据
 * @param  base 数据的基址
 * @param  len  数据的长度
 * @return void
 */
static void deal_net_data(const void *base, size_t len) /* base 内存需管理 */
{
    net_notify_t *nty = NULL;
    n_login_res_t *login_res = NULL;
    int idx = -1;/* 会话id, 用于标识请求, 控制请求的生命周期, 清理资源等 */

    nty = (net_notify_t *)base;
    if (NULL == nty)
    {
        e_debug("deal_net_data", "data transfered from server is empty!");
    }

    idx = nty->idx;
    validate_dlg(idx);
    switch (nty->cmd)
    {
        case EV_LOGIN:
            login_res = nty->nty;
            
            e_debug("deal_net_data",
                    "transfered from server success, type[%s] result[%d]",
                    "EV_LOGIN", login_res->result);
            
            break;
        default:
            e_error("deal_net_data", "unknown data type!");
            
            break;
    }
        
    e_debug("deal_net_data", "received data from server, dialog id[%d]", idx);

    dlg[idx].ack = nty;
    dlg[idx].result = DLG_RES_ACK;
    deal_dialog(idx); /* 到服务端的请求结束, 在客户端处理与UI交付及资源的清理 */
}

/**
 * @brief    处理所有net请求
 * 
 * @param    base 请求携带的信息
 * @param    len  请求信息长度
 * @return   void
 */
/* TODO 记录dlg_id, 并交给协议打包模块, 由协议处理模块发送及接收解析, 将在接收线程处处理 */
static void deal_net_req(void *base, size_t len)
{
    int idx = -1;
    net_req_t *req = NULL;

    req = (net_req_t*)base;
    if (req == NULL)
    {
        e_error("deal_net_req",
                "data transfer to netagent is NULL, req[%s]!",
                req == NULL ? "NULL" : "NOT NULL");

        return;
    }

    e_debug("deal_net_req", "new data has transferd to netagent!");
    
    /* 获取请求的会话id */
    idx = deal_ctrl_net(req->cmd, req->req);
    /* 传递数据 */    
    dlg[idx].net_req = (net_req_t *)base;
}

/**
 * @brief    处理从ctrlagent到netagent的请求
 * 
 * @param    type netreq.h中的请求类型
 * @param    base type类请求的信息起始基址
 *
 * @return   int  会话标识idx
 */
static int deal_ctrl_net(const req_srv_t sv_type, void *base)
{
    int idx = -1;
    
    n_login_t *login = NULL;
    n_register_t *reg = NULL;
    
    switch (sv_type)
    {
        case SV_LOGIN:
            login = (n_login_t *)base;

            net_elogin(sv_type, login);
            
            /* 将数据包装好发向网络 */
            e_debug("deal_ctrl_net",
                    "new request in negagent, dlg[%d], usr[%s], pwd[%s]",
                    login->idx, login->usr, login->pwd);
            idx = login->idx;
            
            break;
        case SV_REGISTER:
            reg = (n_register_t *)base;

            net_eregister(sv_type, reg);
            
            e_debug(__func__,
                    "transfer to netagent success![%s][%s]",
                    reg->usr, reg->pwd);

            idx = reg->idx;
        default:
            idx = -1;
            break;
    }

    return idx;
}
