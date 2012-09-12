#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "erequest.h"
#include "trigger.h"
#include "memdef.h"
#include "erequest.h"
#include "common.h"
#include "elog.h"

static int send_ctrl_agent(int evt, void *base, size_t len);

static int send_ctrl_agent(int evt, void *base, size_t len)
{
    int recver;

    recver = CTRL_AGENT_ID;

    return send_signal(CTRL_REQ_EVT, base, len, 0, recver);
}

int e_snd_msg(int type, const char *msg, const char *to)
{
    ctrl_req_t req = { CMD_SEND_MSG };
    e_send_msg_t *e_msg = NULL;

    if (NULL == msg || NULL == to || strlen(msg) <= 0 || strlen(to) <= 0)
    {
        e_error("e_snd_msg", "illegal request for sending msg");
        return EME_ERR;
    }

    e_msg = (e_send_msg_t *)malloc(sizeof(e_send_msg_t));
    e_msg->type = type;
    strcpy(e_msg->msg, msg);
    strcpy(e_msg->to, to);
    req.req = e_msg;

    e_debug("e_snd_msg", "send msg request, type[%d], msg[%s], to[%s]",
            e_msg->type, e_msg->msg, e_msg->to);

    return send_ctrl_agent(CTRL_REQ_EVT, &req, sizeof(ctrl_req_t));
}

int e_register(const char *usr, const char *pwd, const char *repwd)
{
    ctrl_req_t req = { CMD_REGISTER };
    e_register_t *reg = NULL;

    if (NULL == usr || NULL == pwd || strlen(usr) <= 0 || strlen(pwd) <= 0)
    {
        e_error("e_register", "e_register() illegal register parameters!");
        return EME_ERR;
    }

    reg = malloc(sizeof(e_register_t));

    strcpy(reg->usr, usr);
    strcpy(reg->pwd, pwd);
    strcpy(reg->repwd, repwd);
    req.req = reg;

    e_debug("e_register", "register request, usr[%s], pwd[%s], send:ctrlagent",
            reg->usr, reg->pwd);

    return send_ctrl_agent(CTRL_REQ_EVT, &req, sizeof(ctrl_req_t));
}
    
int e_login(const char *usr, const char *pwd)
{
    ctrl_req_t req = { CMD_LOGIN };
    e_login_t *login = NULL;

    
    if (NULL == usr || NULL == pwd || strlen(usr) <= 0 || strlen(pwd) <= 0)
    {
        e_error("e_login", "e_login() illegal login parameters!");
        return EME_ERR;
    }

    login = malloc(sizeof(e_login_t));
    strcpy(login->usr, usr);
    strcpy(login->pwd, pwd);
    req.req = login;

    e_debug("e_login\t", "new request, usr[%s], pwd[%s], then send_ctrl_agent",
            login->usr, login->pwd);

    return send_ctrl_agent(CTRL_REQ_EVT, &req, sizeof(ctrl_req_t));
}
