/* negagent层的回调 */
#ifndef _ELISTENER_H_
#define _ELISTENER_H_

#include <stddef.h>
#include "netreq.h"

extern n_login_t *login_arg;

void add_listener(const req_srv_t , void *, eclient_listener);
void login_listener(void *base, size_t len);
void register_listener(void *base, size_t len);

#endif /* _ELISTENER_H_ */
