#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "trigger.h"
#include "ctrlagent.h"
#include "erequest.h"
#include "eevent.h"
#include "erequest.h"
#include "netreq.h"
#include "elog.h"

/* 会话生命周期标识 */
dlg_t dlg[MAX_DLG_BUF];

/* 业务逻辑处理入口 */
static cmd_info_t g_cmd_info[] = 
{
    { CMD_LOGIN, "e_login", ctrl_elogin }, 
    { CMD_REGISTER, "e_register", ctrl_eregister },
    { CMD_SEND_MSG, "e_send_msg", ctrl_esend_msg }
};

static void deal_notify_evt(void *base, size_t len);
static void deal_req_evt(void *base, size_t len);
static int alloc_dialog(int cmd_info_idx);
static void clear_dialog(int idx);
static int find_entrance(int cmd);
static void control_dialog(size_t, const req_srv_t, void *, size_t);

/* entrance */
void ctrlagent(int evt, void *base, size_t len, int cmd)
{
    ctrl_req_t *req = NULL;
    ctrl_notify_t *nty = NULL;

    switch (evt)
    {
        case CTRL_REQ_EVT:
            req = (ctrl_req_t *)base;
            e_debug("ctrlagent", "new request in ctrlagent");
            deal_req_evt(req, len);
            break;
        case CTRL_NOTIFY_EVT:
            nty = (ctrl_notify_t *)base;
            e_debug("ctrl_agent", 
                    "new notify in ctrlagent");
            deal_notify_evt(nty, len);
            break;
    }
}

/* 处理CTRL_REQ_EVT事件 */
static void deal_req_evt(void *base, size_t len)
{
    ctrl_req_t *req = NULL;
    int dlg_idx, cmd_info_idx;

    req = (ctrl_req_t *)base;
    cmd_info_idx = find_entrance(req->cmd);
    
    /* 根据不同的类型分配会话id */
    dlg_idx = alloc_dialog(cmd_info_idx);

    /* 接收数据, 其他地方已申请内存, 只需接收指针即可 */
    dlg[dlg_idx].req = base;

    e_debug("deal_req_evt", "deal [%s] request, dlg[%d]",
            g_cmd_info[cmd_info_idx].name, dlg_idx);

    deal_dialog(dlg_idx); /* dlg_idx 在netagent层将被绑定 */

    if (dlg[dlg_idx].step == DLG_STEP_FINISH)
    {
        e_debug("deal_req_evt", "dlg[%d]'s step = [%s]",
                dlg_idx, DLG_STEP_FINISH);
        clear_dialog(dlg_idx);
    }
}

/**
 * @brief    收到回复的结果, 最终推送到UI
 *
 * @param    base 数据的基址
 * @param    len  数据的长度
 * @return   void
 */
static void deal_notify_evt(void *base, size_t len)
{
    int res = 0;

    srand(time(NULL));
    res = rand() % 2;

    e_login_result(res);
}

/**
 * @brief    查找逻辑处理函数的位置, 即下标
 *
 * @param    cmd 具体的请求类型, 如CMD_LOGIN
 * @return   int 标识处理入口的下标
 */
static int find_entrance(int cmd)
{
    int len, i;

    /* 寻找处理逻辑入口 */
    len = (sizeof(g_cmd_info)) / (sizeof(cmd_info_t));
    for (i = 0; i < len; ++i)
    {
        if (cmd == g_cmd_info[i].cmd)
        {
            break;
        }
    }

    if (i == len)
    {
        e_error("find_entrance", "logical error, check g_cmd_info[]!");
    }
    else
    {
        e_debug("find_entrance", "logical entrance found");
    }

    return i;
}

/**
 * @brief    根据g_cmd_info[]下标标记的逻辑分配一会话id, 标识整个会话周期
 *
 * @param    cmd 相应的请求类型,eg:CMD_LOGIN
 *
 * @return   int 标识此次请求的会话id
 */
static int alloc_dialog(int cmd)
{
    int idx, i;

    idx = -1;
    for (i = 0; i < MAX_DLG_BUF; ++i)
    {
        if (dlg[i].flg == DLG_BUF_NOT_USE)
        {
            idx = i;
            dlg[idx].id = idx;
            dlg[idx].flg = DLG_BUF_USED;
            dlg[idx].opr = cmd;
            dlg[idx].req = NULL;
            dlg[idx].net_req = NULL;
            dlg[idx].step = DLG_STEP_INIT;
            dlg[idx].result = DLG_RES_IDLE;
            dlg[idx].ack = NULL;
            memset(dlg[idx].data, 0, sizeof(dlg[idx].data));

            e_debug("alloc_dialog", "new dlg[%d] allocated", idx);
            break;
        }
    }

    if (idx == -1)
    {
        e_error("alloc_dialog", "unknown dlg [%d]!", idx);
    }

    return idx;
}

/**
 * @brief    分别处理各类请求,各类请求以idx(会话id)区分
 *
 * @param    idx 会话标识
 * @return   void
 */
void deal_dialog(int idx)
{
    int opr_id = -1;

    opr_id = dlg[idx].opr;
    e_debug("deal_dialog",
            "enter:g_cmd_info[%d].handler->[%d]", opr_id, idx);

    /**
     * 从negagent再次进入此函数后, 回调到用户并进入下一步,
     * 如DLG_STEP_FINISH, 则会清理dialog */
    (*g_cmd_info[opr_id].handler)(idx); /* ctrl_xxx */

    if (dlg[idx].step == DLG_STEP_FINISH)
    {
        clear_dialog(idx);
    }
}

/**
 * @brief    请求登陆的逻辑处理入口
 *
 * @param    idx 标识此次请求的会话id
 * @return   void
 */
void ctrl_eregister(size_t idx)
{
    e_register_t *e_register = NULL; /* UI层请求的数据结构 */
    n_register_t n_register; /* 将准备发送到netagent的数据结构 */
    n_register_res_t *n_register_res = NULL; /* 请求最终返回的结果 */
    ctrl_req_t *req = NULL; /* 指向请求的结构, 类型不确定, 方便获取原请求的数据 */
    net_notify_t *nty = NULL; /* 最终返回到netagent的回应数据, 类型未知 */
    req_srv_t evt = SV_REGISTER;

    if (dlg[idx].step == DLG_STEP_INIT)
    {
        req = dlg[idx].req;
        e_register = (e_register_t*)req->req;
        n_register.idx = idx;
        strcpy(n_register.usr, e_register->usr);
        strcpy(n_register.pwd, e_register->pwd);
        strcpy(n_register.repwd, e_register->repwd);
    }

    control_dialog(idx, evt, &n_register, sizeof(n_register_t));
    
    /* call back to UI */
    if (dlg[idx].step == DLG_STEP_FINISH)
    {

        if (dlg[idx].result == DLG_RES_ACK)
        {
            nty = dlg[idx].ack;
            n_register_res = nty->nty;
            e_register_result(n_register_res->result);
            e_debug("ctrl_eregister", "dialog ack!");
        }
        else
        {
            e_register_result(n_register_res->result);
            e_debug("ctrl_eregister", "dialog timeout!");
        }
    }
}

void ctrl_esend_msg(size_t idx)
{
    e_send_msg_t *e_snd = NULL;
    n_send_msg_t n_snd;
    n_send_msg_res_t *n_snd_res = NULL;
    ctrl_req_t *req = NULL;
    net_notify_t *nty = NULL;
    req_srv_t evt = SV_SEND_MSG;

    /* 防止回调再次调用此函数出现 逻辑错误 */
    if (dlg[idx].step == DLG_STEP_INIT)
    {
        req = dlg[idx].req;
        e_snd = (e_send_msg_t*)req->req;
        n_snd.idx = idx;
        n_snd.type = e_snd->type;
        strcpy(n_snd.msg, e_snd->msg);
        strcpy(n_snd.to, e_snd->to);
    }

    control_dialog(idx, evt, &n_snd, sizeof(n_send_msg_t));

    /* call back to UI */
    if (dlg[idx].step == DLG_STEP_FINISH)
    {
        if (dlg[idx].result == DLG_RES_ACK)
        {
            nty = dlg[idx].ack;
            n_snd_res = nty->nty;
            e_send_msg_result(n_snd_res->result);
            e_debug("ctrl_esend_msg", "dialog ack!");
        }
        else
        {
            e_send_msg_result(n_snd_res->result);
            e_debug("ctrl_esend_msg", "dialog timeout!");
        }
    }
    
    return;
}

/* 待验证 */
static void
control_dialog(size_t idx, const req_srv_t evt, void *data, size_t len)
{
    int ret, opr_id;
    
    opr_id = dlg[idx].opr;
    e_debug("control_dialog", "dealing new dlg[%d], name [%s]",
            opr_id, g_cmd_info[opr_id].name);
    
    if (DLG_STEP_INIT == dlg[idx].step)
    {
        dlg[idx].beg = time(NULL);
        dlg[idx].step = DLG_STEP_RUN;
        dlg[idx].result = DLG_RES_IDLE;
        e_debug("control_dialog", "dlg[%d] has been initialized", idx);
    }
    
    if (DLG_RES_IDLE == dlg[idx].result)
    {
        e_debug("control_dialog", "new dialog's data will be sent to netagent");
        ret = send_net_agent(evt, data, len);
        if (ret != EME_OK)
        {
            dlg[idx].result = DLG_RES_TIMEOUT;
            e_error("control_dialog","send to netagent failed, error[%d]", ret);
            dlg[idx].step = DLG_STEP_FINISH;
        }
        else
        {
            e_debug("control_dialog", "send to netagent success!");
        }
    }
    else if (DLG_RES_ACK == dlg[idx].result) /* netagent ACK */
    {
        e_debug("control_dialog", "netagent ack, pending to return to UI");

        dlg[idx].step = DLG_STEP_FINISH;
    }
    else if(DLG_RES_TIMEOUT == dlg[idx].result)
    {
        e_debug("control_dialog", "received netagent's result, timeout!");
        dlg[idx].step = DLG_STEP_FINISH;
    }
}

/**
 * @brief    请求登陆的逻辑处理入口
 *
 * @param    idx 标识此次请求的会话id
 * @return   void
 */
void ctrl_elogin(size_t idx)
{
    int opr_id, ret;
    e_login_t *e_login = NULL;
    n_login_t n_login;
    n_login_res_t *n_login_res = NULL;
    ctrl_req_t *req = NULL;
    net_notify_t *nty = NULL;

    opr_id = dlg[idx].opr;
    e_debug("ctrl_elogin", "dealing new dlg[%d], name [%s]", 
            opr_id, g_cmd_info[opr_id].name);

    if (dlg[idx].step == DLG_STEP_INIT)
    {
        dlg[idx].beg = time(NULL);
        dlg[idx].step = DLG_STEP_RUN;
        dlg[idx].result = DLG_RES_IDLE;

        e_debug("ctrl_elogin", "new dlg[%d] has been initialized", idx);
    }
    
        /* 各层之间数据的交换 */
    if (dlg[idx].step == DLG_STEP_RUN)
    {
        req = dlg[idx].req;
        if (dlg[idx].result == DLG_RES_IDLE) /* send to netagent */
        {
            e_login = (e_login_t *)req->req;
            n_login.idx = idx;
            strcpy(n_login.usr, e_login->usr);
            strcpy(n_login.pwd, e_login->pwd);

            e_debug("ctrl_elogin",
                    "n_login_t:idx[%d], usr[%s], pwd[%s]",
                    n_login.idx, n_login.usr, n_login.pwd);
            ret = send_net_agent(SV_LOGIN, &n_login, sizeof(n_login));
            if (ret != EME_OK)
            {
                dlg[idx].result = DLG_RES_TIMEOUT;
                e_error("ctrl_elogin",
                        "send to netagent failed, error[%d]", ret);
                e_login_result(ret);
                clear_dialog(idx);
            }
            else
            {
                e_debug("ctrl_elogin", "send to netagent success!");
            }
        }
        else /* 处理完毕, 返回结果 */
        {
            e_debug("ctrl_elogin", "received result, dlg[%d], name[%s]",
                    idx, g_cmd_info[dlg[idx].opr].name);

            nty = dlg[idx].ack;
            n_login_res = nty->nty;
            if (dlg[idx].result == DLG_RES_ACK) /* netagent 成功返回结果 */
            {
                e_debug("ctrl_elogin", "received netagent ack, dlg[%d]", idx);

                dlg[idx].step = DLG_STEP_FINISH;
                e_login_result(n_login_res->result);
            }
            else if (dlg[idx].result == DLG_RES_TIMEOUT)
            {
                e_debug("ctrl_elogin",
                        "received netagent's result[%d], timeout!",
                        DLG_RES_TIMEOUT);
                
                e_login_result(n_login_res->result);
                dlg[idx].step = DLG_STEP_FINISH;
            }
        }
    }
}

/**
 * @brief  检查会话id是否合法
 * @param  idx 需检查的会话id
 * @return int, EME_OK, EME_ERR
 */
int validate_dlg(const int idx)
{
    int legal = 0;

    legal = idx >= 0 && idx < MAX_DLG_BUF;

    if (!legal)
    {
        e_error("validate_dlg", "illegal dialog id[%d]", idx);
    }

    return legal ? EME_OK : EME_ERR;
}

/**
 * @brief    本次请求最终结束, 清理资源
 *
 * @param    idx 本次请求会话标识
 */
static void clear_dialog(int idx)
{
    int i;

    for (i = 0; i < MAX_DLG_MEMBER; ++i)
    {
        if (dlg[idx].data[i] != NULL) /* must free it's member may be */
        {
            free(dlg[idx].data[i]);
            dlg[idx].data[i] = NULL;
            e_debug("clear_dialog", "dlg[%d]'s data[%d]", idx, i);
        }
    }



    if (dlg[idx].net_req != NULL)
    {
        /* 释放子资源 */
        if (dlg[idx].net_req->req != NULL)
        {
            free(dlg[idx].net_req->req);
            dlg[idx].net_req = NULL;
            e_debug("clear_dialog", "dlg[%d]'s member [%s]", idx, "net_req->req");
        }
        
        free(dlg[idx].net_req);
        dlg[idx].net_req = NULL;
        e_debug("clear_dialog", "dlg[%d]'s member [%s]", idx, "net_req");
    }

    if (dlg[idx].req != NULL)
    {
        /* 释放子资源 */
        if (dlg[idx].req->req != NULL)
        {
            free(dlg[idx].req->req);
            dlg[idx].req->req = NULL;
            e_debug("clear_dialog", "dlg[%d]'s member [%s]", idx, "req->req");
        }
        free(dlg[idx].req);
        dlg[idx].req = NULL;
        e_debug("clear_dialog", "dlg[%d]'s member [%s]", idx, "req");
    }

    if (dlg[idx].ack != NULL)
    {
        /* 释放子资源 */
        if (dlg[idx].ack->nty != NULL)
        {
            free(dlg[idx].ack->nty);
            dlg[idx].ack->nty = NULL;
        }
        free(dlg[idx].ack);
        dlg[idx].ack = NULL;
        e_debug("clear_dialog", "dlg[%d]'s member [%s]", idx, "ack");
    }

    dlg[idx].result = DLG_RES_IDLE;
}
