#ifndef _SENDER_H_
#define _SENDER_H_

#include <stddef.h>
#include "conn.h"

void init_sender();
int eme_send(conn_t *con, void * const buf, const size_t len);

#endif /* _SENDER_H_ */
