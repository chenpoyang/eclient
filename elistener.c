#include <stdlib.h>
#include "elistener.h"
#include "common.h"
#include "netreq.h"

n_login_t *login_arg = NULL;
n_register_t *reg_arg = NULL;

/**
 * @brief  为args结构的请求(如n_login_t)添加回调监听,当服务端的响应时调用
 * @param  tp       请求的类型, 如netreq.h中的SV_LOGIN
 * @param  args     请求的参数，如n_login_t
 * @param  listener 回调的监听，携带加馈信息
 * @return void
 */
void
add_listener(const req_srv_t tp, void *args, eclient_listener listener)
{
    switch (tp)
    {
        case SV_LOGIN:
            login_arg = (n_login_t*)args;
            login_arg->listener = listener;
            e_debug("reg_listener", "event[%s] has added listener", "SV_LOGIN");
            break;
            
        case SV_REGISTER:
            reg_arg = (n_register_t*)args;
            reg_arg->listener = listener;
            e_debug(__func__, "event[%s] has added listener", "SV_REGISTER");
            break;
            
        default:
            break;
    }
}

/**
 * @brief  当从网络接收到字节流后, 结过协议解析后获取相应的信息
 * @param  base 协议解析后的信息位置
 * @param  len  信息的长度
 * @return void
 *//* TODO 准备发送解析后的数据结构到netagent */
void register_listener(void *base, size_t len)
{
    n_register_res_t *reg = NULL;

    reg = (n_register_res_t *)base;

    e_debug("reg_listener",
            "register received data from server,result[%d]", reg->result);

    send_net_notify(reg_arg->idx, EV_REGISTER, reg, sizeof(n_register_res_t));
}

/**
 * @brief  当从网络接收到字节流后, 结过协议解析后获取相应的信息
 * @param  base 协议解析后的信息位置
 * @param  len  信息的长度
 * @return void
 *//* TODO 准备发送解析后的数据结构到netagent */
void login_listener(void *base, size_t len)
{
    n_login_res_t *login = NULL;

    login = (n_login_res_t *)base;

    e_debug("login_listener", "n_login_res[%d], dlg id[%d]",
            login->result, login_arg->idx);

    send_net_notify(login_arg->idx, EV_LOGIN, login, sizeof(n_login_res_t));
}
