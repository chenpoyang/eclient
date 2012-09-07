/* 协议解析模块 */
#ifndef _E_PARSER_H_
#define _E_PARSER_H_

#include <stddef.h>
#include "netreq.h"

typedef struct _eparser_t {
    void *data;
} eparser_t;

int e_compress(const req_srv_t, const void *, char *, size_t);
void e_decompress(const char *buf, size_t len);

#endif /* _E_PARSER_H_ */
