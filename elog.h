#ifndef _ELOG_H_
#define _ELOG_H_

#include "common.h"

typedef enum {
    LEVEL_DEBUG,
    LEVEL_INFO,
    LEVEL_WARN,
    LEVEL_ERROR
} e_log_level_t;

void e_debug(const char *area, const char *fmt, ...);
void e_info(const char *area, const char *fmt, ...);
void e_warn(const char *area, const char *fmt, ...);
void e_error(const char *area, const char *fmt, ...);

#endif /* _ELOG_H_ */
