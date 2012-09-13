#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"
#include "netreq.h"
#include "eparser.h"
#include "jsonpro.h"
#include "trigger.h"
#include "elistener.h"
#include "sender.h"
#include "conn.h"
#include "json.h"
/**
 * @brief  将用户请求sv_type的数据base压缩成ret字符数组, 并用ret返回
 * @param  sv_type 用户的请求类型, 如n_login_t
 * @param  base    请求所携带的数据, 如n_login_t类型的变量
 * @param  ret     将基址为base, 长度为len的数据解析并用ret返回给调用者
 * @param  len     缓冲区长度
 * @return int, EME_OK or EME_ERR, 压缩成功或失败
 */
int e_compress(const req_srv_t sv_type, const void *base, char *str, size_t len)
{
    n_login_t *login = NULL;
    n_register_t *reg = NULL;
    n_send_msg_t *n_snd = NULL;
    int flg = EME_OK;
    conn_t *con = NULL;
    char eme_user[] = "a@icross.com";
    json_t *root = NULL, *label = NULL, *value = NULL;
    char *ret = NULL;
    
    con = get_connection();
    if (get_state(con) != CONNECTED)
    {
        e_error("e_compress", "please connect first!");
    }

    switch (sv_type)
    {
/* 登陆协议:
[{"action":"login","user":"weimade","passwd":"123456"}] */
        case SV_LOGIN:
            login = (n_login_t *)base;
            root = json_new_object();

            label = json_new_string(EME_LOGIN_ACTION_KEY);
            value = json_new_string(EME_LOGIN_VALUE_KEY);
            json_insert_child(label, value);
            json_insert_child(root, label);

            label = json_new_string(EME_LOGIN_USER_KEY);
            value = json_new_string(login->usr);
            json_insert_child(label, value);
            json_insert_child(root, label);

            label = json_new_string(EME_LOGIN_PWD_KEY);
            value = json_new_string(login->pwd);
            json_insert_child(label, value);
            json_insert_child(root, label);

            json_tree_to_string(root, &ret);
            if (eme_send(con, ret, strlen(ret)) !=
                strlen(ret) + strlen(EME_JSON_SPLIT))
            {
                e_error("e_compress", "can not send data");
                flg = EME_ERR;
            }
            free(ret);
            json_free_value(&root);
            break;
            
/* 注册协议:
{"action":"reg","user":"weimade","passwd":"123456","repasswd":"123456"} */
        case SV_REGISTER:
            reg = (n_register_t *)base;
            root = json_new_object();

            label = json_new_string(EME_REGISTER_ACTION_KEY);
            value = json_new_string(EME_REGISTER_VALUE_KEY);
            json_insert_child(label, value);
            json_insert_child(root, label);

            label = json_new_string(EME_REGISTER_USER_KEY);
            value = json_new_string(reg->usr);
            json_insert_child(label, value);
            json_insert_child(root, label);

            label = json_new_string(EME_REGISTER_PWD_KEY);
            value = json_new_string(reg->pwd);
            json_insert_child(label, value);
            json_insert_child(root, label);

            label = json_new_string(EME_REGISTER_REPWD_KEY);
            value = json_new_string(reg->repwd);
            json_insert_child(label, value);
            json_insert_child(root, label);
            
            json_tree_to_string(root, &ret);
            if (eme_send(con, ret, strlen(ret)) !=
                strlen(ret) + strlen(EME_JSON_SPLIT))
            {
                e_error("e_compress", "can not send data");
                flg = EME_ERR;
            }
            free(ret);
            json_free_value(&root);
            
            break;
            
/* 发消息协议:
{"action":"sendmsg","msg":"xxxeeeee","to":"b@icross.com","from":"a@icross.com"} */
        case SV_SEND_MSG:
            n_snd = (n_send_msg_t *)base;
            root = json_new_object();

            label = json_new_string(EME_SENDMSG_ACTION_KEY);
            value = json_new_string(EME_SENDMSG_VALUE_KEY);
            json_insert_child(label, value);
            json_insert_child(root, label);

            label = json_new_string(EME_SENDMSG_MSG_KEY);
            value = json_new_string(n_snd->msg);
            json_insert_child(label, value);
            json_insert_child(root, label);

            label = json_new_string(EME_SENDMSG_TO_KEY);
            value = json_new_string(n_snd->to);
            json_insert_child(label, value);
            json_insert_child(root, label);
            
            label = json_new_string(EME_SENDMSG_FROM_KEY);
            value = json_new_string(eme_user);
            json_insert_child(label, value);
            json_insert_child(root, label);

            json_tree_to_string(root, &ret);
            if (eme_send(con, ret, strlen(ret)) !=
                    strlen(ret) + strlen(EME_JSON_SPLIT))
            {
                e_error("e_compress", "can not send data");
                flg = EME_ERR;
            }
            free(ret);
            json_free_value(&root);
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
