#ifndef _E_EVENT_H_
#define _E_EVENT_H_

typedef enum {
    E_EVT_LOGIN
} e_fire_event_t;

void e_fire_login_result(int result);

#endif /* _E_EVENT_H_ */
