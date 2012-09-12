#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "elog.h"

static const char * const
_e_log_level_name[4] = {"DEBUG", "INFO", "WARN", "ERROR"};

static void	e_log(const e_log_level_t lvl, 
        const char *area, 
        const char *fmt, 
        va_list ap);

void e_debug(const char *area, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    e_log(LEVEL_DEBUG, area, fmt, ap);
    va_end(ap);
}

void e_info(const char *area, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    e_log(LEVEL_INFO, area, fmt, ap);
    va_end(ap);
}

void e_warn(const char *area, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    e_log(LEVEL_WARN, area, fmt, ap);
    va_end(ap);
}

void e_error(const char *area, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    e_log(LEVEL_ERROR, area, fmt, ap);
    va_end(ap);
}

#ifdef _NO_ELOG
static void	
e_log(const e_log_level_t lvl, const char *area,
          const char *fmt, va_list ap) { return; }
#else
static void	
e_log(const e_log_level_t lvl, const char *area, const char *fmt, va_list ap)
{
    int	err, n;
    char buf[E_MAXLINE + 1];

    err = errno;
    snprintf(buf, E_MAXLINE, "%s\t%s ", area, _e_log_level_name[lvl]);
    n = strlen(buf);
#ifdef HAVE_VSNPRINTF
    vsnprintf(buf + n, MAXLINE - n, fmt, ap);
#else
    vsprintf(buf + n, fmt, ap);
#endif
    n = strlen(buf);
    if (lvl >= LEVEL_ERROR && err != 0)
    {
        snprintf(buf + n, E_MAXLINE - n, ": %s", strerror(err));
    }
    strcat(buf, "\n");

    fflush(stdout);
    fputs(buf, stderr);
    fflush(stderr);
}
#endif
