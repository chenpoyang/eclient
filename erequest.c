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

int e_login(const char *usr, const char *pwd)
{
    ctrl_req_t req = {CMD_LOGIN};
    e_login_t *login = NULL;

    
    if (NULL == usr || NULL == pwd || strlen(usr) <= 0 || strlen(pwd) <= 0)
    {
        e_error("e_login", "e_login() illegal user!");
        return EME_ERR;
    }

    login = calloc(1, sizeof(e_login_t));
    strcpy(login->usr, usr);
    strcpy(login->pwd, pwd);
    req.req = login;

    e_debug("e_login\t", "new request, user[%s], pwd[%s], then send_ctrl_agent",
            login->usr, login->pwd);

    return send_ctrl_agent(CTRL_REQ_EVT, &req, sizeof(ctrl_req_t));
}
