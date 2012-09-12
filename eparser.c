#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"
#include "netreq.h"
#include "eparser.h"
#include "trigger.h"
#include "elistener.h"
#include "sender.h"
#include "conn.h"

/**
 * @brief  将用户请求sv_type的数据base压缩成ret字符数组, 并用ret返回
 * @param  sv_type 用户的请求类型, 如n_login_t
 * @param  base    请求所携带的数据, 如n_login_t类型的变量
 * @param  ret     将基址为base, 长度为len的数据解析并用ret返回给调用者
 * @param  len     缓冲区长度
 * @return int, EME_OK or EME_ERR, 压缩成功或失败
 */
int e_compress(const req_srv_t sv_type, const void *base, char *ret, size_t len)
{
    n_login_t *login = NULL;
    n_register_t *reg = NULL;
    n_send_msg_t *n_snd = NULL;
    int flg = EME_OK;
    conn_t *con = NULL;
    char eme_user[] = "a@icross.com";
    
    con = get_connection();
    if (get_state(con) != CONNECTED)
    {
        e_error("e_compress", "please connect first!");
    }

    strcpy(ret, "");
    switch (sv_type)
    {
/* 登陆协议:
[{"action":"login","user":"weimade","passwd":"123456"}] */
        case SV_LOGIN:
            login = (n_login_t *)base;
            snprintf(ret, len, "{\"%s\":\"%s\", \"%s\":\"%s\", \"%s\":\"%s\"}",
                     "action", "login",
                     "userid", login->usr,
                     "password", login->pwd);

            if (eme_send(con, ret, strlen(ret) + 1) != strlen(ret) + 1)
            {
                e_error("e_compress", "can not send data");
                flg = EME_ERR;
            }
            break;
            
/* 注册协议:
{"action":"reg","user":"weimade","passwd":"123456","repasswd":"123456"} */
        case SV_REGISTER:
            reg = (n_register_t *)base;
            snprintf(ret, len,
            "{\"%s\":\"%s\", \"%s\":\"%s\", \"%s\":\"%s\", \"%s\":\"%s\"}",
                     "action", "reg",
                     "user", reg->usr,
                     "passwd", reg->pwd,
                     "repasswd", reg->repwd);
            printf("+1\n");
            if (eme_send(con, ret, strlen(ret) + 1) != strlen(ret) + 1)
            {
                e_error("e_compress", "can not send data");
                flg = EME_ERR;
            }
            
            break;
            
/* 发消息协议:
{"action":"sendmsg","msg":"xxxeeeee","to":"b@icross.com","from":"a@icross.com"} */
        case SV_SEND_MSG:
            n_snd = (n_send_msg_t *)base;
            snprintf(ret, len,
            "{\"%s\":\"%s\", \"%s\":\"%s\", \"%s\":\"%s\", \"%s\":\"%s\"}",
                     "action", "sendmsg",
                     "msg", n_snd->msg,
                     "to", n_snd->to,
                     "from", eme_user); /* TODO 考虑可设成全局用户 */
            
            if (eme_send(con, ret, strlen(ret) + 1) != strlen(ret) + 1)
            {
                e_error("e_compress", "can not send data");
                flg = EME_ERR;
            }
            break;
            
        default:
            break;
    }

    return flg;
}

/**
 * @brief  将收到消息解压, 以便netagent能够认识
 * @param  buf 从服务端接到到的消息
 * @param  len 接收到的消息长度
 * @return void
 */
/* 考虑添回接收到的数据交给协议解析模块，解析完后再交给监听器,
   再到netagent层, 即防止接收模块和negagent层的阻赛 */
/* 网络IO与协议解析处于同一线程, 协议解析可能阻碍网络IO */
void e_decompress(const char *buf, size_t len)
{
    n_login_res_t *login = NULL;
    n_register_res_t *reg = NULL;
    n_send_msg_res_t *n_snd = NULL;
    int flg_ok = 0;

    e_debug("e_decompress", "received new data from server[%s]", buf);
    e_debug("e_decompress", "transfering [%d] bytes to 'elistener'", len);

    /* json 数据形式的自定义协议, 或其他协议 */
    /* TODO send_signal到netagent前需将接收到字符串解析成相应的数据结构 */
    if (strstr(buf, "action") != NULL && strstr(buf, "login") != NULL)
    {
        login = malloc(sizeof(n_login_res_t)); /* 由dlg[idx].ack管理内存 */
        login->result = EME_OK;

        flg_ok = 1;
        login_arg->listener(login, sizeof(n_login_res_t));
        
        e_debug("e_decompress", "decompress success, login callback!");
    }
    else if (strstr(buf, "action") != NULL && strstr(buf, "reg") != NULL)
    {
        reg = malloc(sizeof(n_register_res_t)); /* 由dlg[idx].ack管理内存 */
        reg->result = EME_OK;

        flg_ok = 1;
        reg_arg->listener(reg, sizeof(n_register_res_t));
        
        e_debug("e_decompress", "decompress success, register callback!");
    }
    else if (strstr(buf, "action") != NULL && strstr(buf, "sendmsg") != NULL)
    {
        n_snd = malloc(sizeof(n_send_msg_res_t));
        n_snd->result = EME_OK;

        flg_ok = 1;
        assert(snd_arg != NULL);
        snd_arg->listener(n_snd, sizeof(n_send_msg_res_t));

        e_debug("e_decompress", "decompress success, send msg callback!");
    }

    if (flg_ok)
    {
        e_debug("e_decompress", "decompress success[%s]", buf);
    }
    else
    {
        e_error("e_decompress", "decompress failed!");
    }
}
