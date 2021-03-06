/* negagent层各种请求的的回调, 实质上分别添加一监听器, 监听来自服务端的回复 */
#ifndef _ELISTENER_H_
#define _ELISTENER_H_

#include <stddef.h>
#include "netreq.h"

extern n_login_t *login_arg;
extern n_register_t *reg_arg;
extern n_send_msg_t *snd_arg;

void add_listener(const req_srv_t , void *, eclient_listener);
void login_listener(void *base, size_t len);
void register_listener(void *base, size_t len);
void send_msg_listener(void *base, size_t len);

#endif /* _ELISTENER_H_ */
