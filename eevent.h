/* 最终回调到用户的结果, 如接收到请求注册的结果, 收到即时消息等 */
#ifndef _E_EVENT_H_
#define _E_EVENT_H_

typedef enum {
    E_EVT_LOGIN, 
    E_EVT_REGISTER
} e_fire_event_t;

void e_login_result(int result);
void e_register_result(int result);

#endif /* _E_EVENT_H_ */
