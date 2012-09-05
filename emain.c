#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "erequest.h"
#include "memdef.h"
#include "ctrlagent.h"
#include "netagent.h"
#include "trigger.h"
#include "erequest.h"
#include "emain.h"
#include "conn.h"
#include "elog.h"

int main(int argc, const char *argv[])
{
    /* 初始化代理张程 */
    init_agent(CTRL_AGENT_ID, "ctrlagent", ctrlagent);
    init_agent(NET_AGENT_ID, "netagent", netagent);

    /* 初始化连接和初始化接收线程 */
    init_net_state("127.0.0.1", 8888);

    e_debug("emain\t", "request for login from erequest");
    e_login("eemeeuser", "eemeepwd");

    thread_wait(5);/* 确保e_login()成功回调到用户 */
    
    /* 断开网络连接并关送接收线程 */
    uninit_net_state();
    
    stop_all_agent();

    return 0;
}
