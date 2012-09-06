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

int e_register(const char *usr, const char *pwd)
{
    ctrl_req_t req = { CMD_REGISTER };
    e_register_t *reg = NULL;

    if (NULL == usr || NULL == pwd || strlen(usr) <= 0 || strlen(pwd) <= 0)
    {
        e_error("e_register", "e_register() illegal register parameters!");
        return EME_ERR;
    }

    reg = calloc(1, sizeof(e_register_t));

    strcpy(reg->usr, usr);
    strcpy(reg->pwd, pwd);
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

    login = calloc(1, sizeof(e_login_t));
    strcpy(login->usr, usr);
    strcpy(login->pwd, pwd);
    req.req = login;

    e_debug("e_login\t", "new request, usr[%s], pwd[%s], then send_ctrl_agent",
            login->usr, login->pwd);

    return send_ctrl_agent(CTRL_REQ_EVT, &req, sizeof(ctrl_req_t));
}
